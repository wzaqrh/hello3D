#pragma once
#include "TPredefine.h"
#include "IRenderable.h"
#include "IResource.h"

#define E_PASS_SHADOWCASTER "ShadowCaster"
#define E_PASS_FORWARDBASE "ForwardBase"
#define E_PASS_FORWARDADD "ForwardAdd"
#define E_PASS_POSTPROCESS "PostProcess"

struct TContantBufferInfo {
	IContantBufferPtr buffer;
	bool isUnique;
	std::string name;
public:
	TContantBufferInfo() :buffer(nullptr), isUnique(false) {}
	TContantBufferInfo(IContantBufferPtr __buffer, const std::string& __name = "", bool __isUnique = true);
};
#define MAKE_CBNAME(V) #V

struct TPass {
	std::string mLightMode,mName;
	IInputLayoutPtr mInputLayout;
	D3D11_PRIMITIVE_TOPOLOGY mTopoLogy;
	
	TProgramPtr mProgram;
	std::vector<ISamplerStatePtr> mSamplers;

	std::vector<TContantBufferInfo> mConstantBuffers;

	IRenderTexturePtr mRenderTarget;
	std::vector<IRenderTexturePtr> mIterTargets;
	TTextureBySlot mTextures;

	std::function<void(TPass*,IRenderSystem*,TTextureBySlot&)> OnBind;
	std::function<void(TPass*,IRenderSystem*,TTextureBySlot&)> OnUnbind;
public:
	TPass(const std::string& lightMode, const std::string& name);
	std::shared_ptr<TPass> Clone(IRenderSystem* pRenderSys);
	IContantBufferPtr AddConstBuffer(const TContantBufferInfo& cbuffer);
	ISamplerStatePtr AddSampler(ISamplerStatePtr sampler);
	void ClearSamplers();
	IRenderTexturePtr AddIterTarget(IRenderTexturePtr target);
	
	IContantBufferPtr GetConstBufferByIdx(size_t idx);
	IContantBufferPtr GetConstBufferByName(const std::string& name);
	void UpdateConstBufferByName(IRenderSystem* pRenderSys, const std::string& name, const TData& data);
};
typedef std::shared_ptr<TPass> TPassPtr;

struct TTechnique {
	std::string mName;
	std::vector<TPassPtr> mPasses;
public:
	void AddPass(TPassPtr pass);
	std::shared_ptr<TTechnique> Clone(IRenderSystem* pRenderSys);
	IContantBufferPtr AddConstBuffer(const TContantBufferInfo& cbuffer);
	ISamplerStatePtr AddSampler(ISamplerStatePtr sampler);
	void ClearSamplers();

	TPassPtr GetPassByName(const std::string& passName);
	std::vector<TPassPtr> GetPassesByName(const std::string& passName);

	void UpdateConstBufferByName(IRenderSystem* pRenderSys, const std::string& name, const TData& data);
};
typedef std::shared_ptr<TTechnique> TTechniquePtr;

struct TMaterial : public IResource {
	std::vector<TTechniquePtr> mTechniques;
	int mCurTechIdx = 0;
public:
	std::shared_ptr<TMaterial> Clone(IRenderSystem* pRenderSys);
	void AddTechnique(TTechniquePtr technique);

	TTechniquePtr CurTech();
	TTechniquePtr SetCurTechByIdx(int idx);
	void SetCurTechByName(const std::string& name);

	IContantBufferPtr AddConstBuffer(const TContantBufferInfo& cbuffer);
	ISamplerStatePtr AddSampler(ISamplerStatePtr sampler);
};
typedef std::shared_ptr<TMaterial> TMaterialPtr;

struct TMaterialBuilder {
	TMaterialPtr mMaterial;
	TTechniquePtr mCurTech;
	TPassPtr mCurPass;
public:
	TMaterialBuilder(TMaterialPtr material);
	TMaterialBuilder();
	TMaterialBuilder& AddTechnique(const std::string& name = "d3d11");
	TMaterialBuilder& CloneTechnique(IRenderSystem* pRenderSys, const std::string& name);
	TMaterialBuilder& AddPass(const std::string& lightMode, const std::string& passName);
	TMaterialBuilder& SetPassName(const std::string& lightMode, const std::string& passName);

	TMaterialBuilder& SetInputLayout(IInputLayoutPtr inputLayout);
	TMaterialBuilder& SetTopology(D3D11_PRIMITIVE_TOPOLOGY topology);
	TProgramPtr SetProgram(TProgramPtr program);
	TMaterialBuilder& AddSampler(ISamplerStatePtr sampler, int count = 1);
	TMaterialBuilder& AddSamplerToTech(ISamplerStatePtr sampler, int count = 1);
	TMaterialBuilder& ClearSamplersToTech();
	TMaterialBuilder& AddConstBuffer(IContantBufferPtr buffer, const std::string& name = "", bool isUnique=true);
	TMaterialBuilder& AddConstBufferToTech(IContantBufferPtr buffer, const std::string& name = "", bool isUnique = true);
	TMaterialBuilder& SetRenderTarget(IRenderTexturePtr target);
	TMaterialBuilder& AddIterTarget(IRenderTexturePtr target);
	TMaterialBuilder& SetTexture(size_t slot, ITexturePtr texture);
	TMaterialPtr Build();
};

#define E_MAT_SPRITE "sprite"
#define E_MAT_LAYERCOLOR "LayerColor"
#define E_MAT_SKYBOX "skybox"
#define E_MAT_MODEL "model"
#define E_MAT_MODEL_PBR "model_pbr"
#define E_MAT_MODEL_SHADOW "model_shadow"
#define E_MAT_POSTPROC_BLOOM "bloom"

struct TMaterialFactory {
	IRenderSystem* mRenderSys;
	std::map<std::string, TMaterialPtr> mMaterials;
public:
	TMaterialFactory(IRenderSystem* pRenderSys);
	TMaterialPtr GetMaterial(std::string name, std::function<void(TMaterialPtr material)> callback = nullptr, std::string identify = "", bool readonly=false);
private:
	TMaterialPtr CreateStdMaterial(std::string name);
};
typedef std::shared_ptr<TMaterialFactory> TMaterialFactoryPtr;


#define FILE_EXT_CSO ".cso"
#define FILE_EXT_FX ".fx"

#ifdef PRELOAD_SHADER
#define MAKE_MAT_NAME(NAME) (NAME)
#else
#define MAKE_MAT_NAME(NAME) (NAME FILE_EXT_FX)
#endif