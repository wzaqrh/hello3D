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

#if defined _DEBUG
inline bool ValidateResult(bool res) {
	BOOST_ASSERT(res);
	return res;
}
#define VR(V) ValidateResult(V)
#else
#define VR(V) V
#endif

/********** Pass **********/
void Pass::AddSampler(ISamplerStatePtr sampler)
{
	mSamplers.push_back(sampler);
}

bool Pass::Validate() const
{
	return VR(!mProperty->LightMode.empty()
		&& mProperty->TopoLogy != kPrimTopologyUnkown
		&& mInputLayout
		&& mProgram);
}

/********** Technique **********/
bool Technique::Validate() const
{
	for (auto& pass : mElements)
		if (!VR(pass->Validate()))
			return false;
	return true;
}

std::vector<PassPtr> Technique::GetPassesByLightMode(const std::string& lightMode)
{
	std::vector<PassPtr> result;
	for (auto& pass : mElements) {
		if (pass->GetLightMode() == lightMode) {
			result.push_back(pass);
		}
	}
	return std::move(result);
}

/********** Material **********/
bool Shader::Validate() const
{
	for (auto& tech : mElements)
		if (!VR(tech->Validate()))
			return false;
	return true;
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

CoTask<bool> Material::Build(Launch launchMode, ResourceManager& resMng)
{
	if (mShaderVariant == nullptr) {
		CoAwait resMng.CreateShader(mShaderVariant, launchMode, mShaderVariantParam.Build());
	}
	CoReturn mShaderVariant->IsLoaded();
}

MaterialInstance Material::CreateInstance(Launch launchMode, ResourceManager& resMng) const
{
	GpuParametersPtr newParametrs = mGpuParametersByShareType[kCbShareNone]->Clone(launchMode, resMng);
	newParametrs->Merge(*mGpuParametersByShareType[kCbSharePerMaterial]);
	newParametrs->Merge(*mGpuParametersByShareType[kCbSharePerFrame]);
	return MaterialInstance(const_cast<Material*>(this)->shared_from_this(), newParametrs);
}

}
}