#include <unordered_map>
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
CoTask<ShaderPtr> MaterialFactory::DoCreateShader(Launch launchMode, ResourceManager& resMng, const mat_asset::ShaderNode& shaderNode, ShaderPtr shader) ThreadSafe
{
	COROUTINE_VARIABLES_4(launchMode, resMng, shaderNode, shader);
	shader->SetLoading();
	CoAwait resMng.SwitchToLaunchService(__LaunchSync__);

	for (const auto& categNode : shaderNode) {
		for (const auto& techniqueNode : categNode) {
			auto curTech = CreateInstance<Technique>();
			shader->AddTechnique(curTech);

			for (const auto& passNode : techniqueNode) {
				const auto& passProgram = passNode.Program;
				auto curPass = CreateInstance<Pass>();
				curTech->AddPass(curPass);
				curPass->mLightMode = passNode.LightMode;
				curPass->mName = passNode.ShortName;
				curPass->mTopoLogy = passProgram.Topo;

				curPass->mProgram = CoAwait resMng.CreateProgram(launchMode,
					passProgram.VertexSCD.SourcePath, passProgram.VertexSCD, passProgram.PixelSCD);

				BOOST_ASSERT(passProgram.Attrs.Count() >= 1);
				if (passProgram.Attrs.Count() == 1) {
					curPass->mInputLayout = resMng.CreateLayout(launchMode, curPass->mProgram, passProgram.Attrs[0].Layout);
				}
				else if (passProgram.Attrs.Count() > 1) {
					auto layout_compose = passProgram.Attrs[0].Layout;
					int slot = 1;
					for (const auto& attrNode : boost::make_iterator_range(passProgram.Attrs.Range(1))) {
						for (const auto& element_slot : attrNode.Layout) {
							layout_compose.push_back(element_slot);
							layout_compose.back().InputSlot = slot;
						}
						curPass->mInputLayout = resMng.CreateLayout(launchMode, curPass->mProgram, layout_compose);
						++slot;
					}
				}

				for (const auto& sampler : passProgram.Samplers)
					curPass->AddSampler(IF_AND_NULL(sampler.CmpFunc != kCompareUnkown, resMng.CreateSampler(launchMode, sampler)));
			}//for techniqueNode.Passes
		}//for shaderNode.SubShaders
	}//for shaderNode.Categories

	shader->SetLoaded();
	return shader;
}
CoTask<ShaderPtr> MaterialFactory::CreateShader(Launch launchMode, ResourceManager& resMng, const MaterialLoadParam& loadParam, ShaderPtr shader) ThreadSafe 
{
	COROUTINE_VARIABLES_4(launchMode, resMng, loadParam, shader);
	//CoAwait resMng.SwitchToLaunchService(__LaunchAsync__);

	shader = IF_OR(shader, CreateInstance<Shader>());
	mat_asset::ShaderNode shaderNode;
	if (mMatAssetMng->GetShaderNode(loadParam, shaderNode)) {
		CoAwait DoCreateShader(launchMode, resMng, shaderNode, shader);
	}
	else {
		shader->SetLoaded(false);
	}
	return shader;
}

CoTask<MaterialPtr> MaterialFactory::DoCreateMaterial(Launch launchMode, ResourceManager& resMng, const mat_asset::MaterialNode& materialNode, MaterialPtr material) ThreadSafe
{
	COROUTINE_VARIABLES_4(launchMode, resMng, materialNode, material);
	material->SetLoading();
	CoAwait resMng.SwitchToLaunchService(__LaunchSync__);

	material->mShaderVariant = CoAwait DoCreateShader(launchMode, resMng, materialNode.Shader, CreateInstance<Shader>());
	material->mShaderVariantParam = MaterialLoadParamBuilder(materialNode.LoadParam);

	boost::filesystem::path assetPath(boost::filesystem::system_complete(materialNode.MaterialFilePath));
	assetPath.remove_filename();
	material->mTextures.Resize(8);
	for (const auto& iter : materialNode.TextureProperies) {
		assetPath /= iter.second.ImagePath;
		BOOST_ASSERT(boost::filesystem::is_regular_file(assetPath));
		if (boost::filesystem::is_regular_file(assetPath)) {
			auto texture = CoAwait resMng.CreateTextureByFile(launchMode, assetPath.string());
			material->mTextures.AddOrSet(texture, iter.second.Slot);
		}
		assetPath.remove_filename();
	}

	material->mGpuParametersByShareType[kCbShareNone] = CreateInstance<GpuParameters>();
	material->mGpuParametersByShareType[kCbSharePerMaterial] = CreateInstance<GpuParameters>();
	material->mGpuParametersByShareType[kCbSharePerFrame] = mFrameGpuParameters;
	for (const auto& categNode : materialNode.Shader) {
		const auto& categProgram = categNode.Program;
		for (const auto& uniform : categProgram.Uniforms) {
			GpuParameters::Element element = AddToParametersCache(launchMode, resMng, uniform);
			switch (element.GetShareMode())
			{
			case kCbShareNone: {
				GpuParameters::Element newelem = element.Clone(launchMode, resMng);
				for (const auto& iter : materialNode.UniformProperies)
					newelem.Parameters->SetPropertyByString(iter.first, iter.second);
				material->mGpuParametersByShareType[kCbShareNone]->AddElement(newelem);
			}break;
			case kCbSharePerMaterial: {
				GpuParameters::Element newelem = element.Clone(launchMode, resMng);
				for (const auto& iter : materialNode.UniformProperies)
					newelem.Parameters->SetPropertyByString(iter.first, iter.second);
				material->mGpuParametersByShareType[kCbSharePerMaterial]->AddElement(newelem);
			}break;
			case kCbSharePerFrame:
			default:
				break;
			}
		}
	}

	material->SetLoaded(material->mShaderVariant->IsLoaded());
	return material;
}
CoTask<MaterialPtr> MaterialFactory::CreateMaterial(Launch launchMode, ResourceManager& resMng, const MaterialLoadParam& loadParam, MaterialPtr material) ThreadSafe
{
	COROUTINE_VARIABLES_4(launchMode, resMng, loadParam, material);
	//CoAwait resMng.SwitchToLaunchService(__LaunchAsync__);

	material = IF_OR(material, CreateInstance<Material>());
	mat_asset::MaterialNode materialNode;
	if (mMatAssetMng->GetMaterialNode(loadParam, materialNode)) {
		CoAwait DoCreateMaterial(launchMode, resMng, materialNode, material);
	}
	else {
		material->SetLoaded(false);
	}
	return material;
}

//Clone Functions
PassPtr MaterialFactory::ClonePass(Launch launchMode, ResourceManager& resMng, const Pass& proto)
{
	PassPtr result = CreateInstance<Pass>();
	result->mLightMode = proto.mLightMode;
	result->mName = proto.mName;
	result->mTopoLogy = proto.mTopoLogy;
	result->mInputLayout = proto.mInputLayout;
	result->mProgram = proto.mProgram;
	result->mFrameBuffer = proto.mFrameBuffer;

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