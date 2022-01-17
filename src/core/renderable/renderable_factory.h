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
	renderable::SkyBoxPtr CreateSkybox(string_cref imgpath, const ShaderLoadParam& matName = "");
	renderable::SpritePtr CreateColorLayer(const ShaderLoadParam& matName = "");
	renderable::SpritePtr CreateSprite(string_cref imgpath = "", const ShaderLoadParam& matName = "");
	renderable::MeshPtr CreateMesh(int vertCount = 1024, int indexCount = 1024, const ShaderLoadParam& matName = "");
	renderable::CubePtr CreateCube(const Eigen::Vector3f& center, const Eigen::Vector3f& halfsize, unsigned bgra = -1, const ShaderLoadParam& matName = "");
	renderable::AssimpModelPtr CreateAssimpModel(const ShaderLoadParam& matName = "");
	renderable::LabelPtr CreateLabel(string_cref fontPath, int fontSize);
	renderable::PostProcessPtr CreatePostProcessEffect(string_cref effectName, scene::Camera& camera);
private:
	Launch mLaunchMode;
	ResourceManager& mResourceMng;
	FontCachePtr mFontCache;
};

}