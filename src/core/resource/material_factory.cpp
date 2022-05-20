#include <unordered_map>
#include <boost/format.hpp>
#include "core/base/d3d.h"
#include "core/base/macros.h"
#include "core/base/debug.h"
#include "core/base/rendersys_debug.h"
#include "core/resource/resource_manager.h"
#include "core/resource/material_asset.h"
#include "core/resource/material_factory.h"

namespace mir {
namespace res {

MaterialFactory::MaterialFactory()
{
	mMatAssetMng = CreateInstance<mat_asset::MaterialAssetManager>();
	mFrameGpuParameters = CreateInstance<GpuParameters>();
}

const GpuParametersPtr& MaterialFactory::GetFrameGpuParameters() const { 
	tpl::AutoLock lck(mParametersCache._GetLock());
	return mFrameGpuParameters; 
}

GpuParameters::Element MaterialFactory::AddToParametersCache(Launch launchMode, ResourceManager& resMng, const UniformParameters& parameters) ThreadSafe
{
	const std::string& uniformName = parameters.GetName();
	if (!uniformName.empty()) {
		return mParametersCache.GetOrAdd(uniformName, [&]() {
			GpuParameters::Element result;
			result.Parameters = CreateInstance<UniformParameters>(parameters);
			result.CBuffer = result.Parameters->CreateConstBuffer(launchMode, resMng, IF_AND_OR(parameters.IsReadOnly(), kHWUsageImmutable, kHWUsageDynamic));

			if (parameters.GetShareMode() == kCbSharePerFrame)
				mFrameGpuParameters->AddElement(result);
			return result;
		});
	}
	else {
		GpuParameters::Element result;
		result.Parameters = CreateInstance<UniformParameters>(parameters);
		result.CBuffer = result.Parameters->CreateConstBuffer(launchMode, resMng, IF_AND_OR(parameters.IsReadOnly(), kHWUsageImmutable, kHWUsageDynamic));
		return result;
	}
}
CoTask<bool> MaterialFactory::DoCreateShader(Launch launchMode, ShaderPtr shader, ResourceManager& resMng, mat_asset::ShaderNode shaderNode) ThreadSafe
{
	COROUTINE_VARIABLES_4(launchMode, resMng, shaderNode, shader);
	shader->SetLoading();
	//CoAwait resMng.SwitchToLaunchService(__LaunchSync__);
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

				tasks.push_back([&resMng,launchMode](PassPtr pass, const mat_asset::ProgramNode& programNode)->CoTask<bool> {
					if (!CoAwait resMng.CreateProgram(pass->mProgram, launchMode, programNode.VertexSCD.SourcePath, programNode.VertexSCD, programNode.PixelSCD))
						CoReturn false;

					BOOST_ASSERT(programNode.Attrs.Count() >= 1);
					if (programNode.Attrs.Count() == 1) {
						pass->mInputLayout = resMng.CreateLayout(launchMode, pass->mProgram, programNode.Attrs[0].Layout);
					}
					else if (programNode.Attrs.Count() > 1) {
						auto layout_compose = programNode.Attrs[0].Layout;
						int slot = 1;
						for (const auto& attrNode : boost::make_iterator_range(programNode.Attrs.Range(1))) {
							for (const auto& element_slot : attrNode.Layout) {
								layout_compose.push_back(element_slot);
								layout_compose.back().InputSlot = slot;
							}
							pass->mInputLayout = resMng.CreateLayout(launchMode, pass->mProgram, layout_compose);
							++slot;
						}
					}
					CoReturn true;
				}(curPass, passProgram));

				for (const auto& sampler : passProgram.Samplers)
					curPass->AddSampler(IF_AND_NULL(sampler.CmpFunc != kCompareUnkown, resMng.CreateSampler(launchMode, sampler)));
			}//for techniqueNode.Passes
		}//for shaderNode.SubShaders
	}//for shaderNode.Categories
	CoAwait WhenAll(std::move(tasks));

	BOOST_ASSERT(shader->Validate());
	shader->SetLoaded();
	CoReturn shader->IsLoaded();
}
CoTask<bool> MaterialFactory::CreateShader(Launch launchMode, ShaderPtr& shader, ResourceManager& resMng, MaterialLoadParam loadParam) ThreadSafe 
{
	COROUTINE_VARIABLES_4(launchMode, resMng, loadParam, shader);
	//CoAwait resMng.SwitchToLaunchService(__LaunchAsync__);

	shader = IF_OR(shader, CreateInstance<Shader>());
	mat_asset::ShaderNode shaderNode;
	if (mMatAssetMng->GetShaderNode(loadParam, shaderNode)) {
		CoAwait DoCreateShader(launchMode, shader, resMng, std::move(shaderNode));
	}
	else {
		shader->SetLoaded(false);
	}
	CoReturn shader->IsLoaded();
}

CoTask<bool> MaterialFactory::DoCreateMaterial(Launch launchMode, MaterialPtr material, ResourceManager& resMng, mat_asset::MaterialNode materialNode) ThreadSafe
{
	COROUTINE_VARIABLES_4(launchMode, resMng, materialNode, material);
	material->SetLoading();
	//CoAwait resMng.SwitchToLaunchService(__LaunchSync__);

	std::vector<CoTask<bool>> tasks;
	{
		material->mProperty = materialNode.Property;
		material->mShader = CreateInstance<Shader>();
		tasks.push_back(DoCreateShader(launchMode, material->mShader, resMng, materialNode.Shader));
		material->mLoadParam = materialNode.LoadParam;

		boost::filesystem::path assetPath(boost::filesystem::system_complete(materialNode.Property->DependSrc.Material.FilePath)); assetPath.remove_filename();
		material->mTextures.Resize(kTextureUserSlotCount);
		for (const auto& iter : materialNode.Property->Textures) {
			boost::filesystem::path imagePath = assetPath / iter.second.ImagePath;
			BOOST_ASSERT(boost::filesystem::is_regular_file(imagePath));
			if (boost::filesystem::is_regular_file(imagePath)) {
				if (iter.second.Slot >= material->mTextures.Count())
					material->mTextures.Resize(iter.second.Slot + 1);
				tasks.push_back(resMng.CreateTextureByFile(material->mTextures[iter.second.Slot], launchMode, imagePath.string()));
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
				GpuParameters::Element element = AddToParametersCache(launchMode, resMng, uniform);
				switch (element.GetShareMode())
				{
				case kCbSharePerInstance: {
					GpuParameters::Element newelem = element.Clone(launchMode, resMng);
					for (const auto& iter : materialNode.Property->UniformByName)
						newelem.Parameters->SetPropertyByString(iter.first, iter.second);
					material->mGpuParametersByShareType[kCbSharePerInstance]->AddElement(newelem);
				}break;
				case kCbSharePerMaterial: {
					GpuParameters::Element newelem;
					auto find_share = matParamCache.find(uniform.GetName());
					if (find_share != matParamCache.end()) {
						newelem = find_share->second;
					}
					else {
						newelem = element.Clone(launchMode, resMng);

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
CoTask<bool> MaterialFactory::CreateMaterial(Launch launchMode, MaterialPtr& material, ResourceManager& resMng, MaterialLoadParam loadParam) ThreadSafe
{
	COROUTINE_VARIABLES_4(launchMode, resMng, loadParam, material);
	//CoAwait resMng.SwitchToLaunchService(__LaunchAsync__);

	material = IF_OR(material, CreateInstance<Material>());
	mat_asset::MaterialNode materialNode;
	if (mMatAssetMng->GetMaterialNode(loadParam, materialNode)) {
		TIME_PROFILE((boost::format("\tmatFac.CreateMaterial (name:%1% variant:%2%)") %loadParam.GetShaderName() %loadParam.GetVariantDesc()).str());
		CoAwait DoCreateMaterial(launchMode, material, resMng, std::move(materialNode));
	}
	else {
		material->SetLoaded(false);
	}
	CoReturn material->IsLoaded();
}

bool MaterialFactory::PurgeOutOfDates() ThreadSafe
{
	bool result = mMatAssetMng->PurgeOutOfDates();
	if (result) {
		tpl::AutoLock lck(mParametersCache._GetLock());
		mParametersCache._Clear();
		mFrameGpuParameters = CreateInstance<GpuParameters>();
	}
	return result;
}

//Clone Functions
PassPtr MaterialFactory::ClonePass(Launch launchMode, ResourceManager& resMng, const Pass& proto)
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
TechniquePtr MaterialFactory::CloneTechnique(Launch launchMode, ResourceManager& resMng, const Technique& proto)
{
	TechniquePtr result = CreateInstance<Technique>();
	for (const auto& protoPass : proto)
		result->AddPass(this->ClonePass(launchMode, resMng, *protoPass));

	result->SetLoaded();
	return result;
}
ShaderPtr MaterialFactory::CloneShader(Launch launchMode, ResourceManager& resMng, const Shader& proto)
{
	BOOST_ASSERT(proto.IsLoaded());

	ShaderPtr result = CreateInstance<Shader>();
	for (const auto& protoTech : proto)
		result->AddTechnique(this->CloneTechnique(launchMode, resMng, *protoTech));
	result->mCurTechIdx = proto.mCurTechIdx;

	result->SetLoaded();
	return result;
}
}
}