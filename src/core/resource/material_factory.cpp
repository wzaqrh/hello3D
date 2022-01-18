#include <unordered_map>
#include "core/base/d3d.h"
#include "core/base/macros.h"
#include "core/base/rendersys_debug.h"
#include "core/resource/resource_manager.h"
#include "core/resource/material_asset.h"
#include "core/resource/material_factory.h"

namespace mir {
namespace res {

MaterialFactory::MaterialFactory()
{
	mMatAssetMng = CreateInstance<mat_asset::MaterialAssetManager>();
}

ShaderPtr MaterialFactory::DoCreateShader(Launch launchMode, ResourceManager& resMng,
	const mat_asset::ShaderNode& shaderNode, ShaderPtr shader)
{
	shader->SetPrepared();

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

				curPass->mProgram = resMng.CreateProgram(launchMode,
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

				for (auto& uniform : passProgram.Uniforms)
					curPass->AddConstBuffer(resMng.CreateConstBuffer(launchMode, uniform.Decl, kHWUsageDynamic, Data::Make(uniform.Data)),
						uniform.ShortName, uniform.IsUnique, uniform.Slot);
			}//for techniqueNode.Passes
		}//for shaderNode.SubShaders
	}//for shaderNode.Categories

	if (launchMode == LaunchSync) shader->SetLoaded();
	else resMng.AddResourceDependencyRecursive(shader);
	return shader;
}

ShaderPtr MaterialFactory::CreateShader(Launch launchMode, ResourceManager& resMng,
	const ShaderLoadParam& loadParam, ShaderPtr shader) {
	shader = IF_OR(shader, CreateInstance<Shader>());
	mat_asset::ShaderNode shaderNode;
	if (mMatAssetMng->GetShaderNode(loadParam, shaderNode)) {
		return DoCreateShader(launchMode, resMng, shaderNode, shader);
	}
	else {
		shader->SetLoaded(false);
		return shader;
	}
}

MaterialPtr MaterialFactory::DoCreateMaterial(Launch launchMode, ResourceManager& resMng,
	const mat_asset::MaterialNode& materialNode, MaterialPtr material)
{
	material->mShadeVariant = DoCreateShader(launchMode, resMng, materialNode.Shader, CreateInstance<Shader>());
	material->mShaderVariantParam.ShaderName() = materialNode.Shader.ShortName;
	return material;
}

MaterialPtr MaterialFactory::CreateMaterial(Launch launchMode, ResourceManager& resMng,
	const std::string& loadPath, MaterialPtr material)
{
	material = IF_OR(material, CreateInstance<Material>());
	mat_asset::MaterialNode materialNode;
	if (mMatAssetMng->GetMaterialNode(loadPath, materialNode)) {
		return DoCreateMaterial(launchMode, resMng, materialNode, material);
	}
	else {
		material->SetLoaded(false);
		return material;
	}
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

	size_t slot = 0;
	for (auto buffer : proto.mConstantBuffers) {
		if (!buffer.IsUnique)
			buffer.Buffer = resMng.CreateConstBuffer(launchMode, *buffer.Buffer->GetDecl(), buffer.Buffer->GetUsage(), Data::MakeNull());
		result->AddConstBuffer(buffer.Buffer, buffer.Name, buffer.IsUnique, slot);
		++slot;
	}

	result->SetCurState(proto.GetCurState());
	return result;
}

TechniquePtr MaterialFactory::CloneTechnique(Launch launchMode, ResourceManager& resMng, const Technique& proto)
{
	TechniquePtr result = CreateInstance<Technique>();
	for (const auto& protoPass : proto)
		result->AddPass(this->ClonePass(launchMode, resMng, *protoPass));

	result->SetCurState(proto.GetCurState());
	return result;
}

ShaderPtr MaterialFactory::CloneShader(Launch launchMode, ResourceManager& resMng, const Shader& proto)
{
	BOOST_ASSERT(!(launchMode == LaunchSync && !proto.IsLoaded()));

	ShaderPtr result = CreateInstance<Shader>();
	for (const auto& protoTech : proto)
		result->AddTechnique(this->CloneTechnique(launchMode, resMng, *protoTech));
	result->mCurTechIdx = proto.mCurTechIdx;

	if (launchMode == LaunchAsync && !proto.IsLoaded()) {
		resMng.AddResourceDependencyRecursive(result);
	}
	result->SetCurState(proto.GetCurState());
	return result;
}
}
}