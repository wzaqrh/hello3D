#pragma once
#include "core/renderable/renderable.h"

namespace mir {

class RenderableFactory
{
	IRenderSystem* mRenderSys = nullptr;
	TFontCachePtr mFontCache;
public:
	RenderableFactory(IRenderSystem* renderSys);
	TSpritePtr CreateSprite();
	TSpritePtr CreateColorLayer();
	TMeshPtr CreateMesh(const std::string& matName, int vertCount = 1024, int indexCount = 1024);
	TLabelPtr CreateLabel(const std::string& fontPath, int fontSize);
};

}