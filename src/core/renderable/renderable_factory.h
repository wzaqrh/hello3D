#pragma once
#include "core/mir_export.h"
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
	SkyBoxPtr CreateSkybox(string_cref imgpath, const MaterialLoadParam& matName = "");
	SpritePtr CreateColorLayer(const MaterialLoadParam& matName = "");
	SpritePtr CreateSprite(string_cref imgpath = "", const MaterialLoadParam& matName = "");
	MeshPtr CreateMesh(int vertCount = 1024, int indexCount = 1024, const MaterialLoadParam& matName = "");
	CubePtr CreateCube(const Eigen::Vector3f& center, const Eigen::Vector3f& halfsize, unsigned bgra = -1, const MaterialLoadParam& matName = "");
	AssimpModelPtr CreateAssimpModel(const MaterialLoadParam& matName = "");
	LabelPtr CreateLabel(string_cref fontPath, int fontSize);
	PostProcessPtr CreatePostProcessEffect(string_cref effectName, Camera& camera);
private:
	Launch mLaunchMode;
	ResourceManager& mResourceMng;
	FontCachePtr mFontCache;
};

}