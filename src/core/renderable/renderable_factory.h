#pragma once
#include "core/mir_export.h"
#include "core/base/launch.h"
#include "core/renderable/renderable.h"

namespace mir {

class MIR_CORE_API RenderableFactory 
{
public:
	typedef const std::string& string_cref;
	RenderableFactory(ResourceManager& resMng, Launch launchMode);
	SkyBoxPtr CreateSkybox(string_cref imgpath);
	SpritePtr CreateColorLayer(string_cref matName = "");
	SpritePtr CreateSprite(string_cref imgpath = "", string_cref matName = "");
	MeshPtr CreateMesh(int vertCount = 1024, int indexCount = 1024, string_cref matName = "");
	CubePtr CreateCube(const Eigen::Vector3f& center, const Eigen::Vector3f& halfsize, unsigned bgra = -1);
	AssimpModelPtr CreateAssimpModel(string_cref matName = "");
	LabelPtr CreateLabel(string_cref fontPath, int fontSize);
	PostProcessPtr CreatePostProcessEffect(string_cref effectName, Camera& camera);
private:
	Launch mLaunchMode;
	ResourceManager& mResourceMng;
	FontCachePtr mFontCache;
};

}