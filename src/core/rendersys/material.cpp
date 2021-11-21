#include "core/rendersys/material.h"
#include "core/rendersys/material_cb.h"
#include "core/rendersys/material_factory.h"
#include "core/rendersys/resource_manager.h"
#include "core/rendersys/interface_type.h"
#include "core/renderable/post_process.h"

namespace mir {

/********** TextureBySlot **********/
void TextureBySlot::Merge(const TextureBySlot& other) 
{
	if (Textures.size() < other.Textures.size())
		Textures.resize(other.Textures.size());

	for (size_t i = 0; i < other.Textures.size(); ++i) {
		if (other.Textures[i] && other.Textures[i]->HasSRV()) {
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
	:mLightMode(lightMode)
	,mName(name)
{
	SetPrepared();
}

IContantBufferPtr Pass::AddConstBuffer(const CBufferEntry& cbuffer)
{
	mConstantBuffers.push_back(cbuffer);
	return cbuffer.Buffer;
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

void Pass::UpdateConstBufferByName(ResourceManager& resourceMng, const std::string& name, const Data& data)
{
	IContantBufferPtr buffer = GetConstBufferByName(name);
	if (buffer) resourceMng.UpdateBuffer(buffer, data.Bytes, data.Size);
}

#if 0
std::shared_ptr<Pass> Pass::Clone(ResourceManager& resourceMng)
{
	PassPtr pass = std::make_shared<Pass>(mLightMode, mName);
	pass->mTopoLogy = mTopoLogy;
	
	pass->mInputLayout = mInputLayout;
	resourceMng.AddResourceDependency(pass, mInputLayout);

	pass->mProgram = mProgram;
	resourceMng.AddResourceDependency(pass, mProgram);

	for (auto& sampler : mSamplers) {
		pass->AddSampler(sampler);
		resourceMng.AddResourceDependency(pass, sampler);
	}
		
	for (size_t i = 0; i < mConstantBuffers.size(); ++i) {
		auto buffer = mConstantBuffers[i];
		if (!buffer.IsUnique)
			buffer.Buffer = resourceMng.CreateConstBuffer(*buffer.Buffer->GetDecl(), nullptr);
		pass->AddConstBuffer(buffer);
		resourceMng.AddResourceDependency(pass, buffer.Buffer);
	}

	pass->mRenderTarget = mRenderTarget;
	for (auto& target : mIterTargets) {
		pass->AddIterTarget(target);
		resourceMng.AddResourceDependency(pass, target);
	}

	pass->mTextures = mTextures;
	for (auto& tex : mTextures) {
		resourceMng.AddResourceDependency(pass, tex);
	}

	pass->OnBind = OnBind;
	pass->OnUnbind = OnUnbind;
	return pass;
}
#else
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

	for (size_t i = 0; i < proto.mConstantBuffers.size(); ++i) {
		auto buffer = proto.mConstantBuffers[i];
		if (!buffer.IsUnique) buffer.Buffer = resourceMng.CreateConstBuffer(launchMode, *buffer.Buffer->GetDecl(), nullptr);
		pass->AddConstBuffer(buffer);
		resourceMng.AddResourceDependency(pass, buffer.Buffer);
	}

	pass->mRenderTarget = proto.mRenderTarget;
	for (auto& target : proto.mIterTargets) {
		pass->AddIterTarget(target);
		resourceMng.AddResourceDependency(pass, target);
	}

	pass->mTextures = proto.mTextures;
	for (auto& tex : proto.mTextures)
		resourceMng.AddResourceDependency(pass, tex);

	pass->OnBind = proto.OnBind;
	pass->OnUnbind = proto.OnUnbind;
	return pass;
}
#endif

/********** TTechnique **********/
Technique::Technique()
{
	SetPrepared();
}

void Technique::AddPass(PassPtr pass)
{
	mPasses.push_back(pass);
}

IContantBufferPtr Technique::AddConstBuffer(const CBufferEntry& cbuffer)
{
	for (auto& pass : mPasses)
		pass->AddConstBuffer(cbuffer);
	return cbuffer.Buffer;
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

void Technique::UpdateConstBufferByName(ResourceManager& resourceMng, const std::string& name, const Data& data)
{
	for (int i = 0; i < mPasses.size(); ++i)
		mPasses[i]->UpdateConstBufferByName(resourceMng, name, data);
}

#if 0
TechniquePtr Technique::Clone(ResourceManager& resourceMng)
{
	TechniquePtr technique = std::make_shared<Technique>();
	for (int i = 0; i < mPasses.size(); ++i) {
		PassPtr pass = mPasses[i]->Clone(resourceMng);
		technique->AddPass(pass);
		resourceMng.AddResourceDependency(technique, pass);
	}
	technique->mName = mName;
	return technique;
}
#else
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
#endif

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

IContantBufferPtr Material::AddConstBuffer(const CBufferEntry& cbuffer)
{
	for (auto& tech : mTechniques)
		tech->AddConstBuffer(cbuffer);
	return cbuffer.Buffer;
}

ISamplerStatePtr Material::AddSampler(ISamplerStatePtr sampler)
{
	for (auto& tech : mTechniques)
		tech->AddSampler(sampler);
	return sampler;
}

#if 0
MaterialPtr Material::Clone(ResourceManager& resourceMng)
{
	MaterialPtr material = std::make_shared<Material>();
	material->Assign(*this);
	for (int i = 0; i < mTechniques.size(); ++i) {
		TechniquePtr tech = mTechniques[i]->Clone(resourceMng);
		material->AddTechnique(tech);
		resourceMng.AddResourceDependency(material, tech);
	}
	material->mCurTechIdx = mCurTechIdx;
	return material;
}
#else
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
#endif

}