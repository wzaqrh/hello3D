#pragma once
#include "TInterfaceType.h"

struct TPass {
	std::string mName;
	ID3D11InputLayout* mInputLayout = nullptr;
	D3D11_PRIMITIVE_TOPOLOGY mTopoLogy;
	
	TProgramPtr mProgram;
	std::vector<ID3D11SamplerState*> mSamplers;

	std::vector<ID3D11Buffer*> mConstBuffers;
	std::vector<TContantBufferPtr> mConstantBuffers;
public:
	TContantBufferPtr AddConstBuffer(TContantBufferPtr buffer);
	ID3D11SamplerState* AddSampler(ID3D11SamplerState* sampler);
	std::shared_ptr<TPass> Clone();
};
typedef std::shared_ptr<TPass> TPassPtr;

struct TTechnique {
	std::string mName;
	std::vector<TPassPtr> mPasses;
public:
	void AddPass(TPassPtr pass);
	TContantBufferPtr AddConstBuffer(TContantBufferPtr buffer);
	ID3D11SamplerState* AddSampler(ID3D11SamplerState* sampler);
	std::shared_ptr<TTechnique> Clone();
};
typedef std::shared_ptr<TTechnique> TTechniquePtr;

struct TMaterial {
	std::vector<TTechniquePtr> mTechniques;
	int mCurTechIdx = 0;
public:
	void AddTechnique(TTechniquePtr technique);
	TTechniquePtr CurTech();
	TTechniquePtr SetCurTechByIdx(int idx);
	TContantBufferPtr AddConstBuffer(TContantBufferPtr buffer);
	ID3D11SamplerState* AddSampler(ID3D11SamplerState* sampler);
	std::shared_ptr<TMaterial> Clone();
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
	TMaterialBuilder& AddPass();

	TMaterialBuilder& SetInputLayout(ID3D11InputLayout* inputLayout);
	TMaterialBuilder& SetTopology(D3D11_PRIMITIVE_TOPOLOGY topology);
	TProgramPtr SetProgram(TProgramPtr program);
	TMaterialBuilder& AddSampler(ID3D11SamplerState* sampler);
	TMaterialBuilder& AddConstBuffer(TContantBufferPtr buffer);
	TMaterialPtr Build();
};

class TRenderSystem;
struct TMaterialFactory {
	TRenderSystem* mRenderSys;
	std::map<std::string, TMaterialPtr> mMaterials;
public:
	TMaterialFactory(TRenderSystem* pRenderSys);
	TMaterialPtr GetMaterial(std::string name, std::function<void(TMaterialPtr material)> callback = nullptr, std::string identify = "");
private:
	TMaterialPtr  CreateStdMaterial(std::string name);
};
typedef std::shared_ptr<TMaterialFactory> TMaterialFactoryPtr;