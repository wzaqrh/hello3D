#pragma once
#include "core/mir_export.h"
#include "core/renderable/renderable.h"

namespace mir {

class MIR_CORE_API RenderableFactory 
{
	ResourceManager& mResourceMng;
	IRenderSystem& mRenderSys;
	MaterialFactory& mMaterialFac;
	FontCachePtr mFontCache;
public:
	typedef const std::string& string_cref;
	RenderableFactory(IRenderSystem& renderSys, ResourceManager& resMng, MaterialFactory& matFac);
	SkyBoxPtr CreateSkybox(string_cref imgpath);
	SpritePtr CreateColorLayer(string_cref matName = "");
	SpritePtr CreateSprite(string_cref imgpath = "", string_cref matName = "");
	MeshPtr CreateMesh(int vertCount = 1024, int indexCount = 1024, string_cref matName = "");
	AssimpModelPtr CreateAssimpModel(const TransformPtr& transform, string_cref matName = "");
	LabelPtr CreateLabel(string_cref fontPath, int fontSize);
	PostProcessPtr CreatePostProcessEffect(string_cref effectName, Camera& camera);
};

}