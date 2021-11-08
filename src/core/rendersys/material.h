#pragma once
#include <boost/noncopyable.hpp>
#include "core/rendersys/predeclare.h"
#include "core/rendersys/base_type.h"
#include "core/rendersys/resource.h"
#include "core/rendersys/material_cb.h"

namespace mir {

struct TextureBySlot {
public:
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

	ITexturePtr& At(size_t pos) {
		if (pos >= Textures.size()) Textures.resize(pos + 1);
		return Textures[pos];
	}
	ITexturePtr& operator[](size_t pos) { return At(pos); }

	const ITexturePtr& At(size_t pos) const { return Textures[pos]; }
	const ITexturePtr& operator[](size_t pos) const { return At(pos); }
public:
	std::vector<ITexturePtr> Textures;
};

struct CBufferEntry {
	IContantBufferPtr Buffer;
	std::string Name;
	bool IsUnique;
public:
	static CBufferEntry Make(IContantBufferPtr buffer, const std::string& name, bool isUnique) {
		return CBufferEntry{buffer, name, isUnique};
	}
};
#define MAKE_CBNAME(V) #V

struct Pass : boost::noncopyable 
{
	std::string mLightMode,mName;
	IInputLayoutPtr mInputLayout;
	D3D11_PRIMITIVE_TOPOLOGY mTopoLogy;
	
	IProgramPtr mProgram;
	std::vector<ISamplerStatePtr> mSamplers;

	std::vector<CBufferEntry> mConstantBuffers;

	IRenderTexturePtr mRenderTarget;
	std::vector<IRenderTexturePtr> mIterTargets;
	TextureBySlot mTextures;

	std::function<void(Pass&, IRenderSystem&, TextureBySlot&)> OnBind;
	std::function<void(Pass&, IRenderSystem&, TextureBySlot&)> OnUnbind;
public:
	Pass(const std::string& lightMode, const std::string& name);
	std::shared_ptr<Pass> Clone(IRenderSystem& pRenderSys);
	IContantBufferPtr AddConstBuffer(const CBufferEntry& cbuffer);
	ISamplerStatePtr AddSampler(ISamplerStatePtr sampler);
	void ClearSamplers();
	IRenderTexturePtr AddIterTarget(IRenderTexturePtr target);
	
	std::vector<IContantBufferPtr> GetConstBuffers() const;
	IContantBufferPtr GetConstBufferByIdx(size_t idx);
	IContantBufferPtr GetConstBufferByName(const std::string& name);
	void UpdateConstBufferByName(IRenderSystem& pRenderSys, const std::string& name, const Data& data);
};

struct Technique : boost::noncopyable
{
	std::string mName;
	std::vector<PassPtr> mPasses;
public:
	void AddPass(PassPtr pass);
	std::shared_ptr<Technique> Clone(IRenderSystem& pRenderSys);
	IContantBufferPtr AddConstBuffer(const CBufferEntry& cbuffer);
	ISamplerStatePtr AddSampler(ISamplerStatePtr sampler);
	void ClearSamplers();

	PassPtr GetPassByLightMode(const std::string& lightMode);
	std::vector<PassPtr> GetPassesByLightMode(const std::string& lightMode);

	void UpdateConstBufferByName(IRenderSystem& pRenderSys, const std::string& name, const Data& data);
};

class Material : public Resource 
{
	std::vector<TechniquePtr> mTechniques;
	int mCurTechIdx = 0;
public:
	std::shared_ptr<Material> Clone(IRenderSystem& pRenderSys);
	void AddTechnique(TechniquePtr technique);

	TechniquePtr CurTech();
	TechniquePtr SetCurTechByIdx(int idx);
	void SetCurTechByName(const std::string& name);

	IContantBufferPtr AddConstBuffer(const CBufferEntry& cbuffer);
	ISamplerStatePtr AddSampler(ISamplerStatePtr sampler);
};

}