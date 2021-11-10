#include "core/renderable/renderable_factory.h"
#include "core/renderable/sprite.h"
#include "core/renderable/skybox.h"
#include "core/renderable/mesh.h"
#include "core/renderable/font.h"
#include "core/renderable/label.h"
#include "core/rendersys/material_factory.h"

namespace mir {

RenderableFactory::RenderableFactory(IRenderSystem& renderSys, MaterialFactory& matFac)
	: mRenderSys(renderSys) 
	, mMaterialFac(matFac)
{
	mFontCache = std::make_shared<FontCache>(renderSys);
}

SpritePtr RenderableFactory::CreateSprite()
{
	return std::make_shared<Sprite>(mRenderSys, mMaterialFac, E_MAT_SPRITE);
}

SpritePtr RenderableFactory::CreateColorLayer()
{
	return std::make_shared<Sprite>(mRenderSys, mMaterialFac, E_MAT_LAYERCOLOR);
}

MeshPtr RenderableFactory::CreateMesh(const std::string& matName, int vertCount /*= 1024*/, int indexCount /*= 1024*/)
{
	return std::make_shared<Mesh>(mRenderSys, mMaterialFac, E_MAT_SPRITE, vertCount, indexCount);
}

LabelPtr RenderableFactory::CreateLabel(const std::string& fontPath, int fontSize)
{
	TFontPtr font = mFontCache->GetFont(fontPath, fontSize);
	return std::make_shared<Label>(mRenderSys, mMaterialFac, font);
}

SkyBoxPtr RenderableFactory::CreateSkybox(const std::string& imgName)
{
	return std::make_shared<SkyBox>(mRenderSys, mMaterialFac, imgName);
}

}