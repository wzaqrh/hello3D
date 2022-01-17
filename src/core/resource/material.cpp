#include "core/base/macros.h"
#include "core/rendersys/program.h"
#include "core/rendersys/input_layout.h"
#include "core/rendersys/hardware_buffer.h"
#include "core/rendersys/texture.h"
#include "core/rendersys/render_system.h"
#include "core/resource/material.h"
#include "core/resource/material_factory.h"
#include "core/resource/resource_manager.h"

namespace mir {
namespace res {

/********** Pass **********/
void Pass::AddConstBuffer(IContantBufferPtr buffer, const std::string& name, bool isUnique, int slot)
{
	mConstantBuffers.AddOrSet(CBufferEntry{ buffer, name, isUnique }, slot);
}
void Pass::AddSampler(ISamplerStatePtr sampler)
{
	mSamplers.push_back(sampler);
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

void Pass::GetLoadDependencies(std::vector<IResourcePtr>& depends)
{
	depends.push_back(mInputLayout);
	depends.push_back(mProgram);
	for (const auto& sampler : mSamplers)
		depends.push_back(sampler);
	for (const auto& buffer : mConstantBuffers)
		depends.push_back(buffer.Buffer);
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

/********** Material **********/
void Material::EnableKeyword(const std::string& macroName, int value /*= TRUE*/)
{
	mShaderVariantParam[macroName] = value;
	mShadeVariant = nullptr;
}

void Material::Build(Launch launchMode, ResourceManager& resMng)
{
	if (mShadeVariant == nullptr) {
		mShadeVariant = resMng.CreateShader(launchMode, mShaderVariantParam);
	}
}

const ShaderPtr& Material::GetShader() const
{
	BOOST_ASSERT(mShadeVariant);
	return mShadeVariant;
}

}
}