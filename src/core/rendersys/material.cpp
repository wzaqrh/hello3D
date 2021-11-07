#include "core/rendersys/material.h"
#include "core/rendersys/material_cb.h"
#include "core/rendersys/render_system.h"
#include "core/rendersys/interface_type.h"
#include "core/rendersys/const_buffer_decl.h"
#include "core/renderable/post_process.h"
#include "core/base/utility.h"

namespace mir {

/********** TextureBySlot **********/
void TextureBySlot::Merge(const TextureBySlot& other) {
	if (Textures.size() < other.Textures.size())
		Textures.resize(other.Textures.size());

	for (size_t i = 0; i < other.Textures.size(); ++i) {
		if (other.Textures[i] && other.Textures[i]->HasSRV()) {
			Textures[i] = other.Textures[i];
		}
	}
}

/********** TContantBufferInfo **********/
ContantBufferInfo::ContantBufferInfo(IContantBufferPtr __buffer, const std::string& __name, bool __isUnique)
	:buffer(__buffer)
	,name(__name)
	,isUnique(__isUnique)
{
}

/********** TPass **********/
Pass::Pass(const std::string& lightMode, const std::string& name)
	:mLightMode(lightMode)
	,mName(name)
{
}

IContantBufferPtr Pass::AddConstBuffer(const ContantBufferInfo& cbuffer)
{
	mConstantBuffers.push_back(cbuffer);
	return cbuffer.buffer;
}

ISamplerStatePtr Pass::AddSampler(ISamplerStatePtr sampler)
{
	mSamplers.push_back(sampler);
	return sampler;
}

void Pass::ClearSamplers()
{
	mSamplers.clear();
}

IRenderTexturePtr Pass::AddIterTarget(IRenderTexturePtr target)
{
	mIterTargets.push_back(target);
	return target;
}

IContantBufferPtr Pass::GetConstBufferByIdx(size_t idx)
{
	IContantBufferPtr ret = nullptr;
	if (idx < mConstantBuffers.size())
		ret = mConstantBuffers[idx].buffer;
	return ret;
}

IContantBufferPtr Pass::GetConstBufferByName(const std::string& name)
{
	IContantBufferPtr ret = nullptr;
	for (size_t i = 0; i < mConstantBuffers.size(); ++i) {
		if (mConstantBuffers[i].name == name) {
			ret = mConstantBuffers[i].buffer;
			break;
		}
	}
	return ret;
}

void Pass::UpdateConstBufferByName(IRenderSystem& pRenderSys, const std::string& name, const TData& data)
{
	IContantBufferPtr buffer = GetConstBufferByName(name);
	if (buffer)
		pRenderSys.UpdateConstBuffer(buffer, data.Data, data.DataSize);
}

std::shared_ptr<Pass> Pass::Clone(IRenderSystem& pRenderSys)
{
	PassPtr pass = std::make_shared<Pass>(mLightMode, mName);
	pass->mInputLayout = mInputLayout;
	pass->mTopoLogy = mTopoLogy;
	pass->mProgram = mProgram;
	
	for (auto& sampler : mSamplers)
		pass->AddSampler(sampler);

	for (size_t i = 0; i < mConstantBuffers.size(); ++i) {
		auto buffer = mConstantBuffers[i];
		if (!buffer.isUnique)
			buffer.buffer = pRenderSys.CloneConstBuffer(buffer.buffer);
		pass->AddConstBuffer(buffer);
	}

	pass->mRenderTarget = mRenderTarget;
	for (auto& target : mIterTargets)
		pass->AddIterTarget(target);
	pass->mTextures = mTextures;

	pass->OnBind = OnBind;
	pass->OnUnbind = OnUnbind;
	return pass;
}

/********** TTechnique **********/
void Technique::AddPass(PassPtr pass)
{
	mPasses.push_back(pass);
}

IContantBufferPtr Technique::AddConstBuffer(const ContantBufferInfo& cbuffer)
{
	for (auto& pass : mPasses)
		pass->AddConstBuffer(cbuffer);
	return cbuffer.buffer;
}

ISamplerStatePtr Technique::AddSampler(ISamplerStatePtr sampler)
{
	for (auto& pass : mPasses)
		pass->AddSampler(sampler);
	return sampler;
}

void Technique::ClearSamplers()
{
	for (auto& pass : mPasses)
		pass->ClearSamplers();
}

PassPtr Technique::GetPassByLightMode(const std::string& lightMode)
{
	PassPtr pass;
	for (int i = 0; i < mPasses.size(); ++i) {
		if (mPasses[i]->mLightMode == lightMode) {
			pass = mPasses[i];
			break;
		}
	}
	return pass;
}

std::vector<PassPtr> Technique::GetPassesByLightMode(const std::string& lightMode)
{
	std::vector<PassPtr> passVec;
	for (int i = 0; i < mPasses.size(); ++i) {
		if (mPasses[i]->mLightMode == lightMode) {
			passVec.push_back(mPasses[i]);
		}
	}
	return std::move(passVec);
}

void Technique::UpdateConstBufferByName(IRenderSystem& pRenderSys, const std::string& name, const TData& data)
{
	for (int i = 0; i < mPasses.size(); ++i)
		mPasses[i]->UpdateConstBufferByName(pRenderSys, name, data);
}

std::shared_ptr<Technique> Technique::Clone(IRenderSystem& pRenderSys)
{
	TechniquePtr technique = std::make_shared<Technique>();
	for (int i = 0; i < mPasses.size(); ++i)
		technique->AddPass(mPasses[i]->Clone(pRenderSys));
	technique->mName = mName;
	return technique;
}

/********** TMaterial **********/
void Material::AddTechnique(TechniquePtr technique)
{
	mTechniques.push_back(technique);
}

TechniquePtr Material::CurTech()
{
	return mTechniques[mCurTechIdx];
}

TechniquePtr Material::SetCurTechByIdx(int idx)
{
	mCurTechIdx = idx;
	return mTechniques[mCurTechIdx];
}

void Material::SetCurTechByName(const std::string& name)
{
	for (size_t idx = 0; idx < mTechniques.size(); ++idx)
		if (mTechniques[idx]->mName == name) {
			mCurTechIdx = idx;
			break;
		}
}

IContantBufferPtr Material::AddConstBuffer(const ContantBufferInfo& cbuffer)
{
	for (auto& tech : mTechniques)
		tech->AddConstBuffer(cbuffer);
	return cbuffer.buffer;
}

ISamplerStatePtr Material::AddSampler(ISamplerStatePtr sampler)
{
	for (auto& tech : mTechniques)
		tech->AddSampler(sampler);
	return sampler;
}

std::shared_ptr<Material> Material::Clone(IRenderSystem& pRenderSys)
{
	MaterialPtr material = std::make_shared<Material>();
	
	for (auto& depend : mDepends)
		material->AddDependency(depend);
	material->mCurState = mCurState;

	for (int i = 0; i < mTechniques.size(); ++i) {
		material->AddTechnique(mTechniques[i]->Clone(pRenderSys));
	}
	material->mCurTechIdx = mCurTechIdx;
	return material;
}

}