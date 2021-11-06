#pragma once
#include "core/rendersys/material.h"
#include "core/rendersys/render_system.h"

namespace mir {

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

#define MATERIAL_FROM_XML
	struct TRenderSystem;
	struct TMaterialFactory
	{
		TRenderSystem* mRenderSys;
		std::map<std::string, TMaterialPtr> mMaterials;
		std::shared_ptr<class MaterialAssetManager> mMatAssetMng;
	public:
		TMaterialFactory(TRenderSystem* pRenderSys);
		TMaterialPtr GetMaterial(const std::string& matName, bool sharedUse);
	private:
		TMaterialPtr CreateStdMaterial(const std::string& matName);
	};
}