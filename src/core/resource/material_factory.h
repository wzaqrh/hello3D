#pragma once
#include <boost/noncopyable.hpp>
#include "core/base/launch.h"
#include "core/resource/material.h"

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
#define E_MAT_SKYBOX "Skybox"
#define E_MAT_MODEL "Model"
#define E_MAT_MODEL_PBR "ModelPbr"
#define E_MAT_MODEL_SHADOW "model_shadow"
#define E_MAT_POSTPROC_BLOOM "Bloom"

#define E_PASS_SHADOWCASTER "ShadowCaster"
#define E_PASS_FORWARDBASE "ForwardBase"
#define E_PASS_FORWARDADD "ForwardAdd"
#define E_PASS_POSTPROCESS "PostProcess"

struct MaterialAsset;
struct MaterialFactory : boost::noncopyable
{
	std::shared_ptr<class MaterialAssetManager> mMatAssetMng;
public:
	MaterialFactory();
	MaterialPtr CreateMaterial(Launch launchMode, ResourceManager& resourceMng, const std::string& matName);
	
	MaterialPtr CloneMaterial(Launch launchMode, ResourceManager& resourceMng, const Material& material);
	TechniquePtr CloneTechnique(Launch launchMode, ResourceManager& resourceMng, const Technique& technique);
	PassPtr ClonePass(Launch launchMode, ResourceManager& resourceMng, const Pass& pass);
private:
	MaterialPtr CreateMaterialByMaterialAsset(Launch launchMode, ResourceManager& resourceMng, const MaterialAsset& matAsset);
};

}