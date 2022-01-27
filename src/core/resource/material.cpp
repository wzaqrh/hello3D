#include "core/base/macros.h"
#include "core/rendersys/program.h"
#include "core/rendersys/input_layout.h"
#include "core/rendersys/hardware_buffer.h"
#include "core/rendersys/texture.h"
#include "core/rendersys/render_system.h"
#include "core/resource/material.h"
#include "core/resource/material_parameter.h"
#include "core/resource/material_factory.h"
#include "core/resource/resource_manager.h"

namespace mir {
namespace res {

/********** Pass **********/
void Pass::AddSampler(ISamplerStatePtr sampler)
{
	mSamplers.push_back(sampler);
}

void Pass::GetLoadDependencies(std::vector<IResourcePtr>& depends)
{
	depends.push_back(mInputLayout);
	depends.push_back(mProgram);
	for (const auto& sampler : mSamplers)
		depends.push_back(sampler);
}

/********** Technique **********/
void Technique::GetLoadDependencies(std::vector<IResourcePtr>& depends)
{
	for (const auto& pass : mElements)
		depends.push_back(pass);
}

std::vector<PassPtr> Technique::GetPassesByLightMode(const std::string& lightMode)
{
	std::vector<PassPtr> result;
	for (auto& pass : mElements) {
		if (pass->mLightMode == lightMode) {
			result.push_back(pass);
		}
	}
	return std::move(result);
}

/********** Shader **********/
void Shader::GetLoadDependencies(std::vector<IResourcePtr>& depends)
{
	for (const auto& tech : mElements)
		depends.push_back(tech);
}

/********** MaterialInstance **********/
MaterialInstance::MaterialInstance()
{}

MaterialInstance::MaterialInstance(const MaterialPtr& material, const GpuParametersPtr& gpuParamters)
	: mMaterial(material)
	, mGpuParameters(gpuParamters)
{}

std::vector<IContantBufferPtr> MaterialInstance::GetConstBuffers() const
{
	return mGpuParameters->GetConstBuffers();
}
void MaterialInstance::WriteToCb(RenderSystem& renderSys, const std::string& cbName, Data data) const
{
	mGpuParameters->WriteToElementCb(renderSys, cbName, data);
}
void MaterialInstance::FlushGpuParameters(RenderSystem& renderSys) const
{
	mGpuParameters->FlushToGpu(renderSys);
}

/********** Material **********/
Material::Material()
{}

void Material::EnableKeyword(const std::string& macroName, int value /*= TRUE*/)
{
	mShaderVariantParam[macroName] = value;
	mShaderVariant = nullptr;
}

cppcoro::shared_task<bool> Material::Build(Launch launchMode, ResourceManager& resMng)
{
	if (mShaderVariant == nullptr) {
		mShaderVariant = co_await resMng.CreateShader(launchMode, mShaderVariantParam.Build());
	}
	return mShaderVariant != nullptr;
}

MaterialInstance Material::CreateInstance(Launch launchMode, ResourceManager& resMng) const
{
	GpuParametersPtr newParametrs = mGpuParametersByShareType[kCbShareNone]->Clone(launchMode, resMng);
	newParametrs->Merge(*mGpuParametersByShareType[kCbSharePerMaterial]);
	newParametrs->Merge(*mGpuParametersByShareType[kCbSharePerFrame]);
	return MaterialInstance(const_cast<Material*>(this)->shared_from_this(), newParametrs);
}

const ShaderPtr& Material::GetShader() const
{
	BOOST_ASSERT(mShaderVariant);
	return mShaderVariant;
}

void Material::GetLoadDependencies(std::vector<IResourcePtr>& depends)
{
	depends.push_back(mShaderVariant);
	for (auto& texture : mTextures)
		depends.push_back(texture);
}

}
}