#include "core/rendersys/material.h"
#include "core/rendersys/material_cb.h"
#include "core/rendersys/render_system.h"
#include "core/rendersys/interface_type.h"
#include "core/rendersys/const_buffer_decl.h"
#include "core/renderable/post_process.h"
#include "core/base/utility.h"

namespace mir {

/********** TContantBufferInfo **********/
TContantBufferInfo::TContantBufferInfo(IContantBufferPtr __buffer, const std::string& __name, bool __isUnique)
	:buffer(__buffer)
	,name(__name)
	,isUnique(__isUnique)
{
}

/********** TPass **********/
TPass::TPass(const std::string& lightMode, const std::string& name)
	:mLightMode(lightMode)
	,mName(name)
{
}

IContantBufferPtr TPass::AddConstBuffer(const TContantBufferInfo& cbuffer)
{
	mConstantBuffers.push_back(cbuffer);
	return cbuffer.buffer;
}

ISamplerStatePtr TPass::AddSampler(ISamplerStatePtr sampler)
{
	mSamplers.push_back(sampler);
	return sampler;
}

void TPass::ClearSamplers()
{
	mSamplers.clear();
}

IRenderTexturePtr TPass::AddIterTarget(IRenderTexturePtr target)
{
	mIterTargets.push_back(target);
	return target;
}

IContantBufferPtr TPass::GetConstBufferByIdx(size_t idx)
{
	IContantBufferPtr ret = nullptr;
	if (idx < mConstantBuffers.size())
		ret = mConstantBuffers[idx].buffer;
	return ret;
}

IContantBufferPtr TPass::GetConstBufferByName(const std::string& name)
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

void TPass::UpdateConstBufferByName(IRenderSystem* pRenderSys, const std::string& name, const TData& data)
{
	IContantBufferPtr buffer = GetConstBufferByName(name);
	if (buffer)
		pRenderSys->UpdateConstBuffer(buffer, data.data, data.dataSize);
}

std::shared_ptr<TPass> TPass::Clone(IRenderSystem* pRenderSys)
{
	TPassPtr pass = std::make_shared<TPass>(mLightMode, mName);
	pass->mInputLayout = mInputLayout;
	pass->mTopoLogy = mTopoLogy;
	pass->mProgram = mProgram;
	
	for (auto& sampler : mSamplers)
		pass->AddSampler(sampler);

	for (size_t i = 0; i < mConstantBuffers.size(); ++i) {
		auto buffer = mConstantBuffers[i];
		if (!buffer.isUnique)
			buffer.buffer = pRenderSys->CloneConstBuffer(buffer.buffer);
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
void TTechnique::AddPass(TPassPtr pass)
{
	mPasses.push_back(pass);
}

IContantBufferPtr TTechnique::AddConstBuffer(const TContantBufferInfo& cbuffer)
{
	for (auto& pass : mPasses)
		pass->AddConstBuffer(cbuffer);
	return cbuffer.buffer;
}

ISamplerStatePtr TTechnique::AddSampler(ISamplerStatePtr sampler)
{
	for (auto& pass : mPasses)
		pass->AddSampler(sampler);
	return sampler;
}

void TTechnique::ClearSamplers()
{
	for (auto& pass : mPasses)
		pass->ClearSamplers();
}

TPassPtr TTechnique::GetPassByLightMode(const std::string& lightMode)
{
	TPassPtr pass;
	for (int i = 0; i < mPasses.size(); ++i) {
		if (mPasses[i]->mLightMode == lightMode) {
			pass = mPasses[i];
			break;
		}
	}
	return pass;
}

std::vector<TPassPtr> TTechnique::GetPassesByLightMode(const std::string& lightMode)
{
	std::vector<TPassPtr> passVec;
	for (int i = 0; i < mPasses.size(); ++i) {
		if (mPasses[i]->mLightMode == lightMode) {
			passVec.push_back(mPasses[i]);
		}
	}
	return std::move(passVec);
}

void TTechnique::UpdateConstBufferByName(IRenderSystem* pRenderSys, const std::string& name, const TData& data)
{
	for (int i = 0; i < mPasses.size(); ++i)
		mPasses[i]->UpdateConstBufferByName(pRenderSys, name, data);
}

std::shared_ptr<TTechnique> TTechnique::Clone(IRenderSystem* pRenderSys)
{
	TTechniquePtr technique = std::make_shared<TTechnique>();
	for (int i = 0; i < mPasses.size(); ++i)
		technique->AddPass(mPasses[i]->Clone(pRenderSys));
	technique->mName = mName;
	return technique;
}

/********** TMaterial **********/
void TMaterial::AddTechnique(TTechniquePtr technique)
{
	mTechniques.push_back(technique);
}

TTechniquePtr TMaterial::CurTech()
{
	return mTechniques[mCurTechIdx];
}

TTechniquePtr TMaterial::SetCurTechByIdx(int idx)
{
	mCurTechIdx = idx;
	return mTechniques[mCurTechIdx];
}

void TMaterial::SetCurTechByName(const std::string& name)
{
	for (size_t idx = 0; idx < mTechniques.size(); ++idx)
		if (mTechniques[idx]->mName == name) {
			mCurTechIdx = idx;
			break;
		}
}

IContantBufferPtr TMaterial::AddConstBuffer(const TContantBufferInfo& cbuffer)
{
	for (auto& tech : mTechniques)
		tech->AddConstBuffer(cbuffer);
	return cbuffer.buffer;
}

ISamplerStatePtr TMaterial::AddSampler(ISamplerStatePtr sampler)
{
	for (auto& tech : mTechniques)
		tech->AddSampler(sampler);
	return sampler;
}

std::shared_ptr<TMaterial> TMaterial::Clone(IRenderSystem* pRenderSys)
{
	TMaterialPtr material = std::make_shared<TMaterial>();
	
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