#pragma once
#include "core/mir_export.h"
#include "core/renderable/renderable.h"

namespace mir {

class MIR_CORE_API RenderableFactory 
{
	IRenderSystem& mRenderSys;
	MaterialFactory& mMaterialFac;
	FontCachePtr mFontCache;
public:
	RenderableFactory(IRenderSystem& renderSys, MaterialFactory& matFac);
	SpritePtr CreateSprite();
	SpritePtr CreateColorLayer();
	MeshPtr CreateMesh(const std::string& matName, int vertCount = 1024, int indexCount = 1024);
	LabelPtr CreateLabel(const std::string& fontPath, int fontSize);
	SkyBoxPtr CreateSkybox(const std::string& imgName);
	PostProcessPtr CreatePostProcessEffect(const std::string& effectName, Camera& camera);
};

}