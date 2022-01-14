#include "core/resource/material.h"
#include "core/resource/material_factory.h"
#include "core/resource/resource_manager.h"

namespace mir {

/********** TPass **********/
Pass::Pass(const std::string& lightMode, const std::string& name)
	: mLightMode(lightMode)
	, mName(name)
{
	SetPrepared();
}

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

PassPtr MaterialFactory::ClonePass(Launch launchMode, ResourceManager& resourceMng, const Pass& proto)
{
	PassPtr pass = CreateInstance<Pass>(proto.mLightMode, proto.mName);
	pass->mTopoLogy = proto.mTopoLogy;

	pass->mInputLayout = proto.mInputLayout;
	resourceMng.AddResourceDependency(pass, pass->mInputLayout);

	pass->mProgram = proto.mProgram;
	resourceMng.AddResourceDependency(pass, pass->mProgram);

	for (auto& sampler : proto.mSamplers) {
		pass->AddSampler(sampler);
		resourceMng.AddResourceDependency(pass, sampler);
	}

	for (size_t slot = 0; slot < proto.mConstantBuffers.size(); ++slot) {
		auto buffer = proto.mConstantBuffers[slot];
		if (!buffer.IsUnique) 
			buffer.Buffer = resourceMng.CreateConstBuffer(launchMode, *buffer.Buffer->GetDecl(), buffer.Buffer->GetUsage(), Data::MakeNull());
		pass->AddConstBuffer(buffer, slot);
		resourceMng.AddResourceDependency(pass, buffer.Buffer);
	}

	pass->mFrameBuffer = proto.mFrameBuffer;

	return pass;
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

TechniquePtr MaterialFactory::CloneTechnique(Launch launchMode, ResourceManager& resourceMng, const Technique& proto)
{
	TechniquePtr technique = CreateInstance<Technique>();
	for (int i = 0; i < proto.mPasses.size(); ++i) {
		PassPtr pass = this->ClonePass(launchMode, resourceMng, *proto.mPasses[i]);
		technique->AddPass(pass);
		resourceMng.AddResourceDependency(technique, pass);
	}
	return technique;
}

/********** Material **********/
TechniquePtr Material::SetCurTechByIdx(int idx)
{
	mCurTechIdx = idx;
	return mTechniques[mCurTechIdx];
}

MaterialPtr MaterialFactory::CloneMaterial(Launch launchMode, ResourceManager& resourceMng, const Material& proto)
{
	MaterialPtr material = CreateInstance<Material>();
	material->Assign(proto);
	for (int i = 0; i < proto.mTechniques.size(); ++i) {
		TechniquePtr tech = this->CloneTechnique(launchMode, resourceMng, *proto.mTechniques[i]);
		material->AddTechnique(tech);
		resourceMng.AddResourceDependency(material, tech);
	}
	material->mCurTechIdx = proto.mCurTechIdx;
	return material;
}

}