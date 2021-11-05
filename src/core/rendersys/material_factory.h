#pragma once
#include "core/rendersys/material.h"
#include "core/rendersys/render_system.h"

namespace mir {
	struct TMaterialBuilder
	{
		TMaterialPtr mMaterial;
		TTechniquePtr mCurTech;
		TPassPtr mCurPass;
	public:
		TMaterialBuilder(bool addTechPass = true);
		TMaterialBuilder(TMaterialPtr material);
		TMaterialBuilder& AddTechnique(const std::string& name = "d3d11");
		TMaterialBuilder& CloneTechnique(IRenderSystem* pRenderSys, const std::string& name);
		TMaterialBuilder& AddPass(const std::string& lightMode, const std::string& passName);
		TMaterialBuilder& SetPassName(const std::string& lightMode, const std::string& passName);

		TMaterialBuilder& SetInputLayout(IInputLayoutPtr inputLayout);
		TMaterialBuilder& SetTopology(D3D11_PRIMITIVE_TOPOLOGY topology);
		IProgramPtr SetProgram(IProgramPtr program);
		TMaterialBuilder& AddSampler(ISamplerStatePtr sampler, int count = 1);
		TMaterialBuilder& AddSamplerToTech(ISamplerStatePtr sampler, int count = 1);
		TMaterialBuilder& ClearSamplersToTech();
		TMaterialBuilder& AddConstBuffer(IContantBufferPtr buffer, const std::string& name = "", bool isUnique = true);
		TMaterialBuilder& AddConstBufferToTech(IContantBufferPtr buffer, const std::string& name = "", bool isUnique = true);
		TMaterialBuilder& SetRenderTarget(IRenderTexturePtr target);
		TMaterialBuilder& AddIterTarget(IRenderTexturePtr target);
		TMaterialBuilder& SetTexture(size_t slot, ITexturePtr texture);
		TMaterialPtr Build();
	};

#define FILE_EXT_CSO ".cso"
#define FILE_EXT_FX ".fx"
#ifdef PRELOAD_SHADER
#define MAKE_MAT_NAME(NAME) std::string(NAME)
#else
#define MAKE_MAT_NAME(NAME) std::string(NAME) + FILE_EXT_FX
#endif

#define E_MAT_SPRITE "Sprite"
#define E_MAT_LAYERCOLOR "LayerColor"
#define E_MAT_LABEL "Label"
#define E_MAT_SKYBOX "skybox"
#define E_MAT_MODEL "model"
#define E_MAT_MODEL_PBR "model_pbr"
#define E_MAT_MODEL_SHADOW "model_shadow"
#define E_MAT_POSTPROC_BLOOM "bloom"

//#define MATERIAL_FROM_XML
	struct TRenderSystem;
	struct TMaterialFactory
	{
		TRenderSystem* mRenderSys;
		std::map<std::string, TMaterialPtr> mMaterials;
		std::shared_ptr<class ParseXmlAndCreateMaterial> mParseXmlCreateMaterial;
	public:
		TMaterialFactory(TRenderSystem* pRenderSys);
		TMaterialPtr GetMaterial(std::string name, std::function<void(TMaterialPtr material)> callback = nullptr, std::string identify = "", bool readonly = false);
	private:
		TMaterialPtr CreateStdMaterial(std::string name);
	};
}