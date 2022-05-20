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

/********** Material **********/
MaterialInstance Material::CreateInstance(Launch launchMode, ResourceManager& resMng) const
{
	GpuParametersPtr newParametrs = mGpuParametersByShareType[kCbSharePerInstance]->Clone(launchMode, resMng);
	newParametrs->Merge(*mGpuParametersByShareType[kCbSharePerMaterial]);
	newParametrs->Merge(*mGpuParametersByShareType[kCbSharePerFrame]);
	return MaterialInstance(const_cast<Material*>(this)->shared_from_this(), mTextures, newParametrs, mLoadParam);
}

/********** MaterialInstance **********/
MaterialInstance::MaterialInstance(const MaterialPtr& material, const TextureVector& textures, const GpuParametersPtr& gpuParamters, const MaterialLoadParam& loadParam)
{
	mSelf = CreateInstance<SharedBlock>(material, textures, gpuParamters, loadParam);
}
CoTask<bool> MaterialInstance::Reload(Launch launchMode, ResourceManager& resMng)
{
	auto mtl = CoAwait resMng.CreateMaterialT(launchMode, mSelf->LoadParam);
	*mSelf = *mtl.mSelf;
	CoReturn true;
}

void MaterialInstance::UpdateKeyword(const std::string& macroName, int value /*= TRUE*/)
{
	mSelf->LoadParam[macroName] = value;
}
CoTask<bool> MaterialInstance::CommitKeywords(Launch launchMode, ResourceManager& resMng)
{
	*this = CoAwait resMng.CreateMaterialT(launchMode, mSelf->LoadParam);
	CoReturn true;
}

std::vector<IContantBufferPtr> MaterialInstance::GetConstBuffers() const
{
	return mSelf->GpuParameters->GetConstBuffers();
}
void MaterialInstance::WriteToCb(RenderSystem& renderSys, const std::string& cbName, Data data)
{
	mSelf->GpuParameters->WriteToElementCb(renderSys, cbName, data);
}
void MaterialInstance::FlushGpuParameters(RenderSystem& renderSys)
{
	mSelf->GpuParameters->FlushToGpu(renderSys);
}

}
}