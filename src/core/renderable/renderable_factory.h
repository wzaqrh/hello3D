#pragma once
#include "core/mir_export.h"
#include "core/base/cppcoro.h"
#include "core/base/launch.h"
#include "core/base/material_load_param.h"
#include "core/renderable/renderable.h"

namespace mir {

class MIR_CORE_API RenderableFactory 
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	typedef const std::string& string_cref;
	RenderableFactory(ResourceManager& resMng, Launch launchMode);
	CoTask<rend::SkyBoxPtr> CreateSkybox(std::string imagePath, MaterialLoadParam loadParam = "");
	CoTask<rend::SpritePtr> CreateColorLayer(MaterialLoadParam loadParam = "");
	CoTask<rend::SpritePtr> CreateSprite(std::string imagePath = "", MaterialLoadParam loadParam = "");
	CoTask<rend::MeshPtr> CreateMesh(int vertCount = 1024, int indexCount = 1024, MaterialLoadParam loadParam = "");
	CoTask<rend::CubePtr> CreateCube(Eigen::Vector3f center, Eigen::Vector3f halfsize, unsigned bgra = -1, MaterialLoadParam loadParam = "");
	CoTask<rend::AssimpModelPtr> CreateAssimpModel(MaterialLoadParam loadParam = "");
	CoTask<rend::LabelPtr> CreateLabel(std::string fontPath, int fontSize);
	CoTask<rend::PostProcessPtr> CreatePostProcessEffect(std::string effectName, scene::Camera& camera);
private:
	Launch mLaunchMode;
	ResourceManager& mResourceMng;
	FontCachePtr mFontCache;
};

}