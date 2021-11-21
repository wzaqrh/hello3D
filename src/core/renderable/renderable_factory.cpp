#include "core/renderable/renderable_factory.h"
#include "core/renderable/skybox.h"
#include "core/renderable/sprite.h"
#include "core/renderable/mesh.h"
#include "core/renderable/font.h"
#include "core/renderable/label.h"
#include "core/renderable/assimp_model.h"
#include "core/renderable/post_process.h"
#include "core/rendersys/camera.h"
#include "core/rendersys/material_factory.h"
#include "core/rendersys/render_pipeline.h"
#include "core/rendersys/resource_manager.h"

namespace mir {

#define NotEmptyOr(Str, DefStr) (!Str.empty() ? Str : DefStr)

RenderableFactory::RenderableFactory(ResourceManager& resMng)
	: mResourceMng(resMng)
{
	mFontCache = std::make_shared<FontCache>(mResourceMng);
	mLaunchMode = Launch::Async;
}

SpritePtr RenderableFactory::CreateSprite(string_cref imgpath, string_cref matName)
{
	SpritePtr sprite = Sprite::Create(mLaunchMode, mResourceMng, NotEmptyOr(matName, E_MAT_SPRITE));
	if (! imgpath.empty()) 
		sprite->SetTexture(mResourceMng.CreateTextureByFile(mLaunchMode, imgpath));
	return sprite;
}

SpritePtr RenderableFactory::CreateColorLayer(string_cref matName)
{
	return Sprite::Create(mLaunchMode, mResourceMng, NotEmptyOr(matName, E_MAT_LAYERCOLOR));
}

MeshPtr RenderableFactory::CreateMesh(int vertCount, int indexCount, string_cref matName)
{
	return Mesh::Create(mLaunchMode, mResourceMng, NotEmptyOr(matName, E_MAT_SPRITE), vertCount, indexCount);
}

AssimpModelPtr RenderableFactory::CreateAssimpModel(const TransformPtr& transform, string_cref matName)
{
	return AssimpModel::Create(mLaunchMode, mResourceMng, transform, NotEmptyOr(matName, E_MAT_MODEL));
}

LabelPtr RenderableFactory::CreateLabel(string_cref fontPath, int fontSize)
{
	FontPtr font = mFontCache->GetFont(fontPath, fontSize);
	return Label::Create(mLaunchMode, mResourceMng, font);
}

SkyBoxPtr RenderableFactory::CreateSkybox(string_cref imgpath)
{
	return SkyBox::Create(mLaunchMode, mResourceMng, imgpath);
}

PostProcessPtr RenderableFactory::CreatePostProcessEffect(string_cref effectName, Camera& camera)
{
	PostProcessPtr process;
	if (effectName == E_MAT_POSTPROC_BLOOM) {
		process = Bloom::Create(mLaunchMode, mResourceMng, camera.FetchPostProcessInput());
	}
	return process;
}

}