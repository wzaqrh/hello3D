#include "core/resource/material.h"
#include "core/resource/material_factory.h"
#include "core/resource/resource_manager.h"

namespace mir {

/********** TPass **********/
void Pass::AddConstBuffer(const CBufferEntry& cbuffer, int slot)
{
	if (slot >= 0) {
		if (mConstantBuffers.size() <= slot + 1)
			mConstantBuffers.resize(slot + 1);
		mConstantBuffers[slot] = cbuffer;
	}
	else {
		mConstantBuffers.push_back(cbuffer);
	}
}
void Pass::AddSampler(ISamplerStatePtr sampler)
{
	mSamplers.push_back(sampler);
}

std::vector<IContantBufferPtr> Pass::GetConstBuffers() const
{
	std::vector<IContantBufferPtr> buffers(mConstantBuffers.size());
	for (size_t i = 0; i < mConstantBuffers.size(); ++i)
		buffers[i] = mConstantBuffers[i].Buffer;
	return buffers;
}

IContantBufferPtr Pass::GetConstBufferByIdx(size_t idx)
{
	IContantBufferPtr ret = nullptr;
	if (idx < mConstantBuffers.size())
		ret = mConstantBuffers[idx].Buffer;
	return ret;
}

IContantBufferPtr Pass::GetConstBufferByName(const std::string& name)
{
	IContantBufferPtr ret = nullptr;
	for (size_t i = 0; i < mConstantBuffers.size(); ++i) {
		if (mConstantBuffers[i].Name == name) {
			ret = mConstantBuffers[i].Buffer;
			break;
		}
	}
	return ret;
}

void Pass::UpdateConstBufferByName(RenderSystem& renderSys, const std::string& name, const Data& data)
{
	IContantBufferPtr buffer = GetConstBufferByName(name);
	if (buffer) renderSys.UpdateBuffer(buffer, data);
}

/********** TTechnique **********/
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

/********** Material **********/
TechniquePtr Material::SetCurTechByIdx(int idx)
{
	mCurTechIdx = idx;
	return mTechniques[mCurTechIdx];
}

}