#pragma once
#include "core/renderable/renderable.h"

namespace mir {

class RenderableFactory 
{
	IRenderSystem& mRenderSys;
	MaterialFactory& mMaterialFac;
	FontCachePtr mFontCache;
	IRenderTexturePtr mPPEInput;
public:
	RenderableFactory(IRenderSystem& renderSys, MaterialFactory& matFac, IRenderTexturePtr PPEInput);
	SpritePtr CreateSprite();
	SpritePtr CreateColorLayer();
	MeshPtr CreateMesh(const std::string& matName, int vertCount = 1024, int indexCount = 1024);
	LabelPtr CreateLabel(const std::string& fontPath, int fontSize);
	SkyBoxPtr CreateSkybox(const std::string& imgName);
	PostProcessPtr CreatePostProcessEffect(const std::string& effectName);
};

}