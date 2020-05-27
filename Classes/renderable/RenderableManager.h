#pragma once
//INCLUDE_PREDEFINE_H

class RenderableManager
{
	IRenderSystem* mRenderSys = nullptr;
	TFontCachePtr mFontCache;
public:
	TSpritePtr CreateSprite();
	TSpritePtr CreateColorLayer();
	TMeshPtr CreateMesh(const std::string& matName, int vertCount = 1024, int indexCount = 1024);
	TLabelPtr CreateLabel(const std::string& fontPath, int fontSize);
};