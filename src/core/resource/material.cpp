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

#if USE_CBUFFER_ENTRY
void Pass::AddConstBuffer(IContantBufferPtr buffer, const std::string& name, bool isUnique, int slot)
{
	mConstantBuffers.AddOrSet(CBufferEntry{ buffer, name, isUnique }, slot);
}
std::vector<IContantBufferPtr> Pass::GetConstBuffers() const
{
	std::vector<IContantBufferPtr> result(mConstantBuffers.Count());
	struct CBufferEntryToCBuffer {
		IContantBufferPtr operator()(const CBufferEntry& cbuffer) const {
			return cbuffer.Buffer;
		}
	};
	std::transform(mConstantBuffers.begin(), mConstantBuffers.end(), result.begin(), CBufferEntryToCBuffer());
	return result;
}

IContantBufferPtr Pass::GetConstBufferByIdx(size_t idx)
{
	return IF_AND_NULL(idx < mConstantBuffers.Count(), mConstantBuffers[idx].Buffer);
}

IContantBufferPtr Pass::GetConstBufferByName(const std::string& name)
{
	IContantBufferPtr result = nullptr;
	for (auto& cbuffer : mConstantBuffers) {
		if (cbuffer.Name == name) {
			result = cbuffer.Buffer;
			break;
		}
	}
	return result;
}

void Pass::UpdateConstBufferByName(RenderSystem& renderSys, const std::string& name, const Data& data)
{
	IContantBufferPtr buffer = GetConstBufferByName(name);
	if (buffer) renderSys.UpdateBuffer(buffer, data);
}
#endif

void Pass::GetLoadDependencies(std::vector<IResourcePtr>& depends)
{
	depends.push_back(mInputLayout);
	depends.push_back(mProgram);
	for (const auto& sampler : mSamplers)
		depends.push_back(sampler);
#if USE_CBUFFER_ENTRY
	for (const auto& buffer : mConstantBuffers)
		depends.push_back(buffer.Buffer);
#endif
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

void Material::Build(Launch launchMode, ResourceManager& resMng)
{
	if (mShaderVariant == nullptr) {
		mShaderVariant = resMng.CreateShader(launchMode, mShaderVariantParam);
	}
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