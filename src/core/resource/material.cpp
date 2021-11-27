#include "core/resource/material.h"
#include "core/resource/material_factory.h"
#include "core/resource/resource_manager.h"

namespace mir {

/********** TextureBySlot **********/
void TextureBySlot::Merge(const TextureBySlot& other) 
{
	if (Textures.size() < other.Textures.size())
		Textures.resize(other.Textures.size());

	for (size_t i = 0; i < other.Textures.size(); ++i) {
		if (other.Textures[i] && other.Textures[i]->IsLoaded()) {
			Textures[i] = other.Textures[i];
		}
	}
}

bool TextureBySlot::IsLoaded() const {
	for (size_t i = 0; i < Textures.size(); ++i) {
		if (Textures[i] && !Textures[i]->IsLoaded())
			return false;
	}
	return true;
}

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
void Pass::ClearSamplers()
{
	mSamplers.clear();
}
void Pass::AddIterTarget(IFrameBufferPtr target)
{
	mRTIterators.push_back(target);
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
	PassPtr pass = std::make_shared<Pass>(proto.mLightMode, proto.mName);
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
			buffer.Buffer = resourceMng.CreateConstBuffer(__launchMode__, *buffer.Buffer->GetDecl(), kHWUsageDynamic, Data::MakeNull());
		pass->AddConstBuffer(buffer, slot);
		resourceMng.AddResourceDependency(pass, buffer.Buffer);
	}

	pass->mRenderTarget = proto.mRenderTarget;
	for (auto& target : proto.mRTIterators) {
		pass->AddIterTarget(target);
		resourceMng.AddResourceDependency(pass, target);
	}

	pass->mTextures = proto.mTextures;
	for (auto& tex : proto.mTextures)
		resourceMng.AddResourceDependency(pass, tex);

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
	TechniquePtr technique = std::make_shared<Technique>();
	for (int i = 0; i < proto.mPasses.size(); ++i) {
		PassPtr pass = this->ClonePass(launchMode, resourceMng, *proto.mPasses[i]);
		technique->AddPass(pass);
		resourceMng.AddResourceDependency(technique, pass);
	}
	technique->mName = proto.mName;
	return technique;
}

/********** Material **********/
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

MaterialPtr MaterialFactory::CloneMaterial(Launch launchMode, ResourceManager& resourceMng, const Material& proto)
{
	MaterialPtr material = std::make_shared<Material>();
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