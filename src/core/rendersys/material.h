#pragma once
#include "core/rendersys/material_pred.h"
#include "core/rendersys/interface_type_pred.h"
#include "core/rendersys/resource.h"
#include "core/rendersys/material_cb.h"

namespace mir {

struct TextureBySlot {
	std::vector<ITexturePtr> textures;
public:
	bool empty() const;
	size_t size() const;

	void clear();
	void push_back(ITexturePtr texture);
	void swap(TextureBySlot& other);
	void resize(size_t size);

	const ITexturePtr At(size_t pos) const;
	ITexturePtr& At(size_t pos);

	const ITexturePtr operator[](size_t pos) const;
	ITexturePtr& operator[](size_t pos);
public:
	void Merge(const TextureBySlot& other);
};

struct ContantBufferInfo 
{
	IContantBufferPtr buffer;
	bool isUnique;
	std::string name;
public:
	ContantBufferInfo() :buffer(nullptr), isUnique(false) {}
	ContantBufferInfo(IContantBufferPtr __buffer, const std::string& __name = "", bool __isUnique = true);
};
#define MAKE_CBNAME(V) #V

struct TData;
struct IRenderSystem;
struct Pass 
{
	std::string mLightMode,mName;
	IInputLayoutPtr mInputLayout;
	D3D11_PRIMITIVE_TOPOLOGY mTopoLogy;
	
	IProgramPtr mProgram;
	std::vector<ISamplerStatePtr> mSamplers;

	std::vector<ContantBufferInfo> mConstantBuffers;

	IRenderTexturePtr mRenderTarget;
	std::vector<IRenderTexturePtr> mIterTargets;
	TextureBySlot mTextures;

	std::function<void(Pass*, IRenderSystem*, TextureBySlot&)> OnBind;
	std::function<void(Pass*, IRenderSystem*, TextureBySlot&)> OnUnbind;
public:
	Pass(const std::string& lightMode, const std::string& name);
	std::shared_ptr<Pass> Clone(IRenderSystem* pRenderSys);
	IContantBufferPtr AddConstBuffer(const ContantBufferInfo& cbuffer);
	ISamplerStatePtr AddSampler(ISamplerStatePtr sampler);
	void ClearSamplers();
	IRenderTexturePtr AddIterTarget(IRenderTexturePtr target);
	
	IContantBufferPtr GetConstBufferByIdx(size_t idx);
	IContantBufferPtr GetConstBufferByName(const std::string& name);
	void UpdateConstBufferByName(IRenderSystem* pRenderSys, const std::string& name, const TData& data);
};

struct Technique
{
	std::string mName;
	std::vector<TPassPtr> mPasses;
public:
	void AddPass(TPassPtr pass);
	std::shared_ptr<Technique> Clone(IRenderSystem* pRenderSys);
	IContantBufferPtr AddConstBuffer(const ContantBufferInfo& cbuffer);
	ISamplerStatePtr AddSampler(ISamplerStatePtr sampler);
	void ClearSamplers();

	TPassPtr GetPassByLightMode(const std::string& lightMode);
	std::vector<TPassPtr> GetPassesByLightMode(const std::string& lightMode);

	void UpdateConstBufferByName(IRenderSystem* pRenderSys, const std::string& name, const TData& data);
};

class Material : public Resource 
{
	std::vector<TTechniquePtr> mTechniques;
	int mCurTechIdx = 0;
public:
	std::shared_ptr<Material> Clone(IRenderSystem* pRenderSys);
	void AddTechnique(TTechniquePtr technique);

	TTechniquePtr CurTech();
	TTechniquePtr SetCurTechByIdx(int idx);
	void SetCurTechByName(const std::string& name);

	IContantBufferPtr AddConstBuffer(const ContantBufferInfo& cbuffer);
	ISamplerStatePtr AddSampler(ISamplerStatePtr sampler);
};

}