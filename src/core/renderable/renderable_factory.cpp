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
	mFontCache = std::make_shared<FontCache>(renderSys);
}

SpritePtr RenderableFactory::CreateSprite()
{
	return std::make_shared<Sprite>(mRenderSys, E_MAT_SPRITE);
}

SpritePtr RenderableFactory::CreateColorLayer()
{
	return std::make_shared<Sprite>(mRenderSys, E_MAT_LAYERCOLOR);
}

MeshPtr RenderableFactory::CreateMesh(const std::string& matName, int vertCount /*= 1024*/, int indexCount /*= 1024*/)
{
	return std::make_shared<Mesh>(mRenderSys, E_MAT_SPRITE, vertCount, indexCount);
}

LabelPtr RenderableFactory::CreateLabel(const std::string& fontPath, int fontSize)
{
	TFontPtr font = mFontCache->GetFont(fontPath, fontSize);
	return std::make_shared<Label>(mRenderSys, font);
}

}