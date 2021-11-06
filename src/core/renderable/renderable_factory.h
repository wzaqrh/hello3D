#pragma once
#include "core/renderable/renderable.h"

namespace mir {

class RenderableFactory {
	IRenderSystem* mRenderSys = nullptr;
	FontCachePtr mFontCache;
public:
	RenderableFactory(IRenderSystem* renderSys);
	SpritePtr CreateSprite();
	SpritePtr CreateColorLayer();
	MeshPtr CreateMesh(const std::string& matName, int vertCount = 1024, int indexCount = 1024);
	LabelPtr CreateLabel(const std::string& fontPath, int fontSize);
};

}