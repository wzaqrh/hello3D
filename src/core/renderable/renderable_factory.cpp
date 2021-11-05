#include "core/renderable/renderable_factory.h"
#include "core/renderable/sprite.h"
#include "core/renderable/mesh.h"
#include "core/renderable/font.h"
#include "core/renderable/label.h"
#include "core/rendersys/material_factory.h"

namespace mir {

RenderableFactory::RenderableFactory(IRenderSystem* renderSys)
{
	mRenderSys = renderSys;
	mFontCache = std::make_shared<TFontCache>(renderSys);
}

TSpritePtr RenderableFactory::CreateSprite()
{
	return std::make_shared<TSprite>(mRenderSys, E_MAT_SPRITE);
}

TSpritePtr RenderableFactory::CreateColorLayer()
{
	return std::make_shared<TSprite>(mRenderSys, E_MAT_LAYERCOLOR);
}

TMeshPtr RenderableFactory::CreateMesh(const std::string& matName, int vertCount /*= 1024*/, int indexCount /*= 1024*/)
{
	return std::make_shared<TMesh>(mRenderSys, E_MAT_SPRITE, vertCount, indexCount);
}

TLabelPtr RenderableFactory::CreateLabel(const std::string& fontPath, int fontSize)
{
	TFontPtr font = mFontCache->GetFont(fontPath, fontSize);
	return std::make_shared<TLabel>(mRenderSys, font);
}

}