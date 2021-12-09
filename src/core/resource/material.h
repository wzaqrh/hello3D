#pragma once
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#include "core/predeclare.h"
#include "core/base/declare_macros.h"
#include "core/base/base_type.h"
#include "core/resource/resource.h"

namespace mir {

struct TextureBySlot 
{
	void Clear() {
		Textures.clear();
	}
	void Add(ITexturePtr texture) {
		Textures.push_back(texture);
	}
	void Swap(TextureBySlot& other) {
		Textures.swap(other.Textures);
	}
	void Resize(size_t size) {
		Textures.resize(size);
	}
	void Merge(const TextureBySlot& other);
public:
	bool Empty() const { return Textures.empty(); }
	size_t Count() const { return Textures.size(); }

	std::vector<ITexturePtr>::const_iterator begin() const { return Textures.begin(); }
	std::vector<ITexturePtr>::const_iterator end() const { return Textures.end(); }

	ITexturePtr& At(size_t pos) {
		if (pos >= Textures.size()) Textures.resize(pos + 1);
		return Textures[pos];
	}
	ITexturePtr& operator[](size_t pos) { return At(pos); }

	const ITexturePtr& At(size_t pos) const { return Textures[pos]; }
	const ITexturePtr& operator[](size_t pos) const { return At(pos); }

	bool IsLoaded() const;
public:
	std::vector<ITexturePtr> Textures;
};

struct CBufferEntry 
{
	static CBufferEntry MakeEmpty() {
		return CBufferEntry{nullptr, "", false};
	}
	static CBufferEntry Make(IContantBufferPtr buffer, const std::string& name, bool isUnique) {
		return CBufferEntry{buffer, name, isUnique};
	}
	bool IsValid() const { return Buffer != nullptr; }
public:
	IContantBufferPtr Buffer;
	std::string Name;
	bool IsUnique;
};
#define MAKE_CBNAME(V) #V

class Pass : public ImplementResource<IResource>, std::enable_shared_from_this<Pass> 
{
	friend class MaterialFactory;
public:
	Pass(const std::string& lightMode, const std::string& name);
	void AddConstBuffer(const CBufferEntry& cbuffer, int slot);
	void AddSampler(ISamplerStatePtr sampler);
	void ClearSamplers();
	void AddIterTarget(IFrameBufferPtr target);
	
	void UpdateConstBufferByName(RenderSystem& renderSys, const std::string& name, const Data& data);

	std::vector<IContantBufferPtr> GetConstBuffers() const;
	IContantBufferPtr GetConstBufferByIdx(size_t idx);
	IContantBufferPtr GetConstBufferByName(const std::string& name);
public:
	std::string mLightMode, mName;

	PrimitiveTopology mTopoLogy;
	IInputLayoutPtr mInputLayout;
	IProgramPtr mProgram;
	TextureBySlot mTextures;
	std::vector<ISamplerStatePtr> mSamplers;
	std::vector<CBufferEntry> mConstantBuffers;

	IFrameBufferPtr mRenderTarget;
	std::vector<IFrameBufferPtr> mRTIterators;
};

class Technique : public ImplementResource<IResource>
{
	friend class MaterialFactory;
public:
	Technique() {
		SetPrepared();
	}
	void AddPass(PassPtr pass) {
		mPasses.push_back(pass);
	}
	TemplateArgs void AddConstBuffer(T &&...args) {
		for (auto& pass : mPasses)
			pass->AddConstBuffer(std::forward<T>(args)...);
	}
	TemplateArgs void AddSampler(T &&...args) {
		for (auto& pass : mPasses)
			pass->AddSampler(std::forward<T>(args)...);
	}
	void ClearSamplers() {
		for (auto& pass : mPasses)
			pass->ClearSamplers();
	}

	TemplateArgs void UpdateConstBufferByName(T &&...args) {
		for (auto& pass : mPasses)
			pass->UpdateConstBufferByName(std::forward<T>(args)...);
	}

	PassPtr GetPassByLightMode(const std::string& lightMode);
	std::vector<PassPtr> GetPassesByLightMode(const std::string& lightMode);
public:
	std::string mName;
	std::vector<PassPtr> mPasses;
};

class MIR_CORE_API Material : public ImplementResource<IResource> 
{
	friend class MaterialFactory;
public:
	Material() {
		SetPrepared();
	}
	void AddTechnique(TechniquePtr technique) {
		mTechniques.push_back(technique);
	}

	TechniquePtr SetCurTechByIdx(int idx);
	void SetCurTechByName(const std::string& name);

	TechniquePtr CurTech() const { return mTechniques[mCurTechIdx]; }
private:
	std::vector<TechniquePtr> mTechniques;
	int mCurTechIdx = 0;
};

}