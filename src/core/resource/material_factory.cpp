#include <unordered_map>
#include <boost/format.hpp>
#include "core/base/macros.h"
#include "core/base/debug.h"
#include "core/resource/resource_manager.h"
#include "core/resource/material_asset.h"
#include "core/resource/material_factory.h"
#include "core/resource/texture_factory.h"
#include "core/resource/program_factory.h"

namespace mir {
namespace res {

MaterialFactory::MaterialFactory(ResourceManager& mResMng, const std::string& shaderDir)
: mResMng(mResMng)
{
	mMatAssetMng = CreateInstance<mat_asset::MaterialAssetManager>(shaderDir);
	mFrameGpuParameters = CreateInstance<GpuParameters>();
}

MaterialFactory::~MaterialFactory()
{
	DEBUG_LOG_MEMLEAK("mtlFac.destrcutor");
}

const GpuParametersPtr& MaterialFactory::GetFrameGpuParameters() const 
{ 
	tpl::AutoLock lck(mMaterialCache._GetLock());
	return mFrameGpuParameters; 
}

CoTask<bool> MaterialFactory::DoCreateShaderByShaderNode(ShaderPtr shader, Launch lchMode, mat_asset::ShaderNode shaderNode) ThreadSafe ThreadMaySwitch
{
	DEBUG_LOG_CALLSTK("mtlFac.DoCreateShaderByShaderNode");
	COROUTINE_VARIABLES_3(lchMode, shaderNode, shader);
	if (! mResMng.SupportMTResCreation()) CoAwait mResMng.SwitchToLaunchService(LaunchSync);
	std::vector<CoTask<bool>> tasks;

	for (const auto& categNode : shaderNode) {
		for (const auto& techniqueNode : categNode) {
			auto curTech = CreateInstance<Technique>();
			shader->AddTechnique(curTech);

			for (const auto& passNode : techniqueNode) {
				const mat_asset::ProgramNode& passProgram = passNode.Program;
				PassPtr curPass = CreateInstance<Pass>();
				curTech->AddPass(curPass);
				curPass->mProperty = passNode.Property;

				tasks.push_back([this, lchMode](PassPtr pass, const mat_asset::ProgramNode& programNode)->CoTask<bool> {
					if (!CoAwait mResMng.CreateProgram(pass->mProgram, lchMode, programNode.VertexSCD.SourcePath, programNode.VertexSCD, programNode.PixelSCD))
						CoReturn false;

					BOOST_ASSERT(programNode.Attrs.Count() >= 1);
					if (programNode.Attrs.Count() == 1) {
						pass->mInputLayout = mResMng.CreateLayout(lchMode, pass->mProgram, programNode.Attrs[0].Layout);
					}
					else if (programNode.Attrs.Count() > 1) {
						auto layout_compose = programNode.Attrs[0].Layout;
						int slot = 1;
						for (const auto& attrNode : boost::make_iterator_range(programNode.Attrs.Range(1))) {
							for (const auto& element_slot : attrNode.Layout) {
								layout_compose.push_back(element_slot);
								layout_compose.back().InputSlot = slot;
							}
							pass->mInputLayout = mResMng.CreateLayout(lchMode, pass->mProgram, layout_compose);
							++slot;
						}
					}
					CoReturn true;
				}(curPass, passProgram));

				for (const auto& sampler : passProgram.Samplers)
					curPass->AddSampler(IF_AND_NULL(sampler.CmpFunc != kCompareUnkown, mResMng.CreateSampler(lchMode, sampler)));
			}//for techniqueNode.Passes
		}//for shaderNode.SubShaders
	}//for shaderNode.Categories
	CoAwait WhenAll(std::move(tasks));

	BOOST_ASSERT(shader->Validate());
	shader->SetLoaded();
	CoReturn shader->IsLoaded();
}
CoTask<bool> MaterialFactory::CreateShader(ShaderPtr& shader, Launch lchMode, MaterialLoadParam loadParam) ThreadSafe ThreadMaySwitch
{
	DEBUG_LOG_CALLSTK("mtlFac.CreateShader");
	COROUTINE_VARIABLES_3(lchMode, loadParam, shader);

	shader = IF_OR(shader, CreateInstance<Shader>());
	mat_asset::ShaderNode shaderNode;
	if (mMatAssetMng->GetShaderNode(loadParam, shaderNode)) {
		CoAwait this->DoCreateShaderByShaderNode(shader, lchMode, std::move(shaderNode));
	}
	else {
		shader->SetLoaded(false);
	}
	CoReturn shader->IsLoaded();
}

GpuParameters::Element MaterialFactory::DoCreateGpuParameterElement(Launch lchMode, const UniformParameters& parameters)  ThreadSafe
{
	GpuParameters::Element result;
	result.Parameters = CreateInstance<UniformParameters>(parameters);
	result.CBuffer = result.Parameters->CreateConstBuffer(lchMode, mResMng, IF_AND_OR(parameters.IsReadOnly(), kHWUsageImmutable, kHWUsageDynamic));

	if (parameters.GetShareMode() == kCbSharePerFrame) {
		tpl::AutoLock lck(mMaterialCache._GetLock());
		const auto& uniformName = parameters.GetName();
		BOOST_ASSERT(! uniformName.empty());
		auto iter = mParametersByUniformName.find(uniformName);
		if (iter == mParametersByUniformName.end()) {
			mParametersByUniformName.insert(std::make_pair(uniformName, result));
			mFrameGpuParameters->AddElement(result);
		}
	}
	
	return result;
}
CoTask<bool> MaterialFactory::DoCreateMaterialByMtlNode(MaterialPtr material, Launch lchMode, mat_asset::MaterialNode materialNode) ThreadSafe ThreadMaySwitch
{
	DEBUG_LOG_CALLSTK("mtlFac.DoCreateMaterialByMtlNode");
	COROUTINE_VARIABLES_3(lchMode, materialNode, material);

	std::vector<CoTask<bool>> tasks;
	{
		material->mProperty = materialNode.Property;
		material->mShader = CreateInstance<Shader>();
		tasks.push_back(DoCreateShaderByShaderNode(material->mShader, lchMode, materialNode.Shader));
		material->mLoadParam = materialNode.LoadParam;

		boost::filesystem::path assetPath(boost::filesystem::system_complete(materialNode.Property->DependSrc.Material.FilePath)); assetPath.remove_filename();

		int textureCount = 0;
		for (const auto& iter : materialNode.Property->Textures) {
			textureCount = std::max(textureCount, iter.second.Slot + 1);
		}
		material->mTextures.Resize(textureCount);

		for (const auto& iter : materialNode.Property->Textures) {
			boost::filesystem::path imagePath = assetPath / iter.second.ImagePath;
			BOOST_ASSERT(boost::filesystem::is_regular_file(imagePath));
			if (boost::filesystem::is_regular_file(imagePath)) {
				BOOST_ASSERT(iter.second.Slot < material->mTextures.Count());
				tasks.push_back(mResMng.CreateTextureByFile(material->mTextures[iter.second.Slot], lchMode, imagePath.string(), kFormatUnknown, iter.second.GenMipmap));
			}
		}
	}
	CoAwait WhenAll(std::move(tasks));

	std::map<std::string, GpuParameters::Element> matParamCache;
	material->mGpuParametersByShareType[kCbSharePerInstance] = CreateInstance<GpuParameters>();
	material->mGpuParametersByShareType[kCbSharePerMaterial] = CreateInstance<GpuParameters>();
	material->mGpuParametersByShareType[kCbSharePerFrame] = mFrameGpuParameters;
	for (const auto& categNode : materialNode.Shader) {
		const auto& categProgram = categNode.Program;
		for (const auto& uniform : categProgram.Uniforms) {
			if (uniform.IsValid()) {
				GpuParameters::Element element = DoCreateGpuParameterElement(lchMode, uniform);
				switch (element.GetShareMode())
				{
				case kCbSharePerInstance: {
					auto& newelem = element;
					for (const auto& iter : materialNode.Property->UniformByName)
						newelem.Parameters->SetPropertyByString(iter.first, iter.second);
					material->mGpuParametersByShareType[kCbSharePerInstance]->AddElement(newelem);
				}break;
				case kCbSharePerMaterial: {
					auto find_share = matParamCache.find(uniform.GetName());
					if (find_share != matParamCache.end()) {
						auto& newelem = find_share->second;
					}
					else {
						auto& newelem = element;
						for (const auto& iter : materialNode.Property->UniformByName)
							newelem.Parameters->SetPropertyByString(iter.first, iter.second);
						material->mGpuParametersByShareType[kCbSharePerMaterial]->AddElement(newelem);
					}
				}break;
				case kCbSharePerFrame:
				default:
					break;
				}
			}
		}
	}

	material->SetLoaded(material->mShader->IsLoaded());
	CoReturn material->IsLoaded();
}
CoTask<bool> MaterialFactory::DoCreateMaterial(MaterialPtr& material, Launch lchMode, MaterialLoadParam loadParam) ThreadSafe ThreadMaySwitch
{
	material = IF_OR(material, CreateInstance<Material>());
	mat_asset::MaterialNode materialNode;
	if (mMatAssetMng->GetMaterialNode(loadParam, materialNode)) {
		TIME_PROFILE((boost::format("\tmatFac.DoCreateMaterialByMtlNode (name:%1% variant:%2%)") %loadParam.GetShaderName() %loadParam.GetVariantDesc()).str());
		CoAwait DoCreateMaterialByMtlNode(material, lchMode, std::move(materialNode));
	}
	else {
		material->SetLoaded(false);
	}
	CoReturn material->IsLoaded();
}
CoTask<bool> MaterialFactory::CreateMaterial(MaterialPtr& material, Launch lchMode, MaterialLoadParam loadParam) ThreadSafe ThreadMaySwitch
{
	DEBUG_LOG_CALLSTK("mtlFac.CreateMaterial1");
	COROUTINE_VARIABLES_3(material, lchMode, loadParam);

	bool resNeedLoad = false;
	material = mMaterialCache.GetOrAdd(loadParam, [&]() {
		auto material = CreateInstance<res::Material>();
		DEBUG_SET_RES_PATH(material, (boost::format("name:%1% variant:%2%") %loadParam.GetShaderName() %loadParam.GetVariantDesc()).str());
		DEBUG_SET_CALL(material, lchMode);
		resNeedLoad = true;
		return material;
	});
	if (resNeedLoad) {
		CoAwait this->DoCreateMaterial(material, lchMode, std::move(loadParam));
	}
	else {
		CoAwait mResMng.WaitResComplete(material);
	}
	CoReturn material->IsLoaded();
}
CoTask<bool> MaterialFactory::CreateMaterial(res::MaterialInstance& mtlInst, Launch lchMode, MaterialLoadParam loadParam) ThreadSafe ThreadMaySwitch
{
	DEBUG_LOG_CALLSTK("mtlFac.CreateMaterial");
	MaterialPtr material;

	if (!mResMng.SupportMTResCreation()) CoAwait mResMng.SwitchToLaunchService(LaunchSync);
	CoAwait CreateMaterial(material, lchMode, loadParam);
	BOOST_ASSERT(material->IsLoaded());
	
	mtlInst = material->CreateInstance(lchMode, mResMng);
	CoReturn mtlInst->IsLoaded();
}

bool MaterialFactory::PurgeOutOfDates() ThreadSafe
{
	if (mMatAssetMng->PurgeOutOfDates()) {
		PurgeAll();
		return true;
	}
	else return false;
}
void MaterialFactory::PurgeAll() ThreadSafe
{
	tpl::AutoLock lck(mMaterialCache._GetLock());
	mMaterialCache._Clear();
	mParametersByUniformName.clear();
	mFrameGpuParameters = CreateInstance<GpuParameters>();
}

//Clone Functions
PassPtr MaterialFactory::ClonePass(Launch lchMode, const Pass& proto)
{
	PassPtr result = CreateInstance<Pass>();
	result->mProperty = proto.mProperty;
	result->mInputLayout = proto.mInputLayout;
	result->mProgram = proto.mProgram;

	for (const auto& sampler : proto.mSamplers)
		result->AddSampler(sampler);

	result->SetLoaded();
	return result;
}
TechniquePtr MaterialFactory::CloneTechnique(Launch lchMode, const Technique& proto)
{
	TechniquePtr result = CreateInstance<Technique>();
	for (const auto& protoPass : proto)
		result->AddPass(this->ClonePass(lchMode, *protoPass));

	result->SetLoaded();
	return result;
}
ShaderPtr MaterialFactory::CloneShader(Launch lchMode, const Shader& proto)
{
	BOOST_ASSERT(proto.IsLoaded());

	ShaderPtr result = CreateInstance<Shader>();
	for (const auto& protoTech : proto)
		result->AddTechnique(this->CloneTechnique(lchMode, *protoTech));
	result->mCurTechIdx = proto.mCurTechIdx;

	result->SetLoaded();
	return result;
}

}
}