#pragma once
#include "TPredefine.h"
#include "IRenderable.h"
#include "IResource.h"

#define E_PASS_SHADOWCASTER "ShadowCaster"
#define E_PASS_FORWARDBASE "ForwardBase"
#define E_PASS_FORWARDADD "ForwardAdd"
#define E_PASS_POSTPROCESS "PostProcess"

struct TContantBufferInfo {
	TContantBufferPtr buffer;
	bool isUnique;
	std::string name;
public:
	TContantBufferInfo() :buffer(nullptr), isUnique(false) {}
	TContantBufferInfo(TContantBufferPtr __buffer, const std::string& __name = "", bool __isUnique = true);
};
#define MAKE_CBNAME(V) #V

struct TPass {
	std::string mLightMode,mName;
	TInputLayoutPtr mInputLayout;
	D3D11_PRIMITIVE_TOPOLOGY mTopoLogy;
	
	TProgramPtr mProgram;
	std::vector<ID3D11SamplerState*> mSamplers;

	std::vector<ID3D11Buffer*> mConstBuffers;
	std::vector<TContantBufferInfo> mConstantBuffers;

	TRenderTexturePtr mRenderTarget;
	std::vector<TRenderTexturePtr> mIterTargets;
	TTextureBySlot mTextures;

	std::function<void(TPass*,TRenderSystem*,TTextureBySlot&)> OnBind;
	std::function<void(TPass*,TRenderSystem*,TTextureBySlot&)> OnUnbind;
public:
	TPass(const std::string& lightMode, const std::string& name);
	std::shared_ptr<TPass> Clone(TRenderSystem* pRenderSys);
	TContantBufferPtr AddConstBuffer(const TContantBufferInfo& cbuffer);
	ID3D11SamplerState* AddSampler(ID3D11SamplerState* sampler);
	TRenderTexturePtr AddIterTarget(TRenderTexturePtr target);
	
	TContantBufferPtr GetConstBufferByIdx(size_t idx);
	TContantBufferPtr GetConstBufferByName(const std::string& name);
	void UpdateConstBufferByName(TRenderSystem* pRenderSys, const std::string& name, void* data);
};
typedef std::shared_ptr<TPass> TPassPtr;

struct TTechnique {
	std::string mName;
	std::vector<TPassPtr> mPasses;
public:
	void AddPass(TPassPtr pass);
	std::shared_ptr<TTechnique> Clone(TRenderSystem* pRenderSys);
	TContantBufferPtr AddConstBuffer(const TContantBufferInfo& cbuffer);
	ID3D11SamplerState* AddSampler(ID3D11SamplerState* sampler);
	
	TPassPtr GetPassByName(const std::string& passName);
	std::vector<TPassPtr> GetPassesByName(const std::string& passName);

	void UpdateConstBufferByName(TRenderSystem* pRenderSys, const std::string& name, void* data);
};
typedef std::shared_ptr<TTechnique> TTechniquePtr;

struct TMaterial : public IResource {
	std::vector<TTechniquePtr> mTechniques;
	int mCurTechIdx = 0;
public:
	void AddTechnique(TTechniquePtr technique);
	std::shared_ptr<TMaterial> Clone(TRenderSystem* pRenderSys);

	TTechniquePtr CurTech();
	TTechniquePtr SetCurTechByIdx(int idx);
	TContantBufferPtr AddConstBuffer(const TContantBufferInfo& cbuffer);
	ID3D11SamplerState* AddSampler(ID3D11SamplerState* sampler);
};
typedef std::shared_ptr<TMaterial> TMaterialPtr;

struct TMaterialBuilder {
	TMaterialPtr mMaterial;
	TTechniquePtr mCurTech;
	TPassPtr mCurPass;
public:
	TMaterialBuilder(TMaterialPtr material);
	TMaterialBuilder();
	TMaterialBuilder& AddTechnique();
	TMaterialBuilder& AddPass(const std::string& lightMode, const std::string& passName);
	TMaterialBuilder& SetPassName(const std::string& lightMode, const std::string& passName);

	TMaterialBuilder& SetInputLayout(TInputLayoutPtr inputLayout);
	TMaterialBuilder& SetTopology(D3D11_PRIMITIVE_TOPOLOGY topology);
	TProgramPtr SetProgram(TProgramPtr program);
	TMaterialBuilder& AddSampler(ID3D11SamplerState* sampler);
	TMaterialBuilder& AddConstBuffer(TContantBufferPtr buffer, const std::string& name = "", bool isUnique=true);
	TMaterialBuilder& AddConstBufferToTech(TContantBufferPtr buffer, const std::string& name = "", bool isUnique = true);
	TMaterialBuilder& SetRenderTarget(TRenderTexturePtr target);
	TMaterialBuilder& AddIterTarget(TRenderTexturePtr target);
	TMaterialBuilder& SetTexture(size_t slot, TTexturePtr texture);
	TMaterialPtr Build();
};

#define E_MAT_SPRITE "standard"
#define E_MAT_SKYBOX "skybox"
#define E_MAT_MODEL "model"
#define E_MAT_MODEL_PBR "model_pbr"
#define E_MAT_MODEL_SHADOW "model_shadow"
#define E_MAT_POSTPROC_BLOOM "bloom"

struct TMaterialFactory {
	TRenderSystem* mRenderSys;
	std::map<std::string, TMaterialPtr> mMaterials;
public:
	TMaterialFactory(TRenderSystem* pRenderSys);
	TMaterialPtr GetMaterial(std::string name, std::function<void(TMaterialPtr material)> callback = nullptr, std::string identify = "", bool readonly=false);
private:
	TMaterialPtr CreateStdMaterial(std::string name);
};
typedef std::shared_ptr<TMaterialFactory> TMaterialFactoryPtr;