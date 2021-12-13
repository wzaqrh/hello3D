#include "core/renderable/renderable_factory.h"
#include "core/renderable/skybox.h"
#include "core/renderable/sprite.h"
#include "core/renderable/mesh.h"
#include "core/renderable/font.h"
#include "core/renderable/label.h"
#include "core/renderable/cube.h"
#include "core/renderable/assimp_model.h"
#include "core/renderable/post_process.h"
#include "core/scene/camera.h"
#include "core/resource/material_factory.h"
#include "core/rendersys/render_pipeline.h"
#include "core/resource/resource_manager.h"

namespace mir {

#define NotEmptyOr(Str, DefStr) (!Str.ShaderName.empty() ? Str : DefStr)

RenderableFactory::RenderableFactory(ResourceManager& resMng, Launch launchMode)
	: mResourceMng(resMng)
{
	mFontCache = std::make_shared<FontCache>(mResourceMng);
	mLaunchMode = launchMode;
}

SpritePtr RenderableFactory::CreateSprite(string_cref imgpath, const MaterialLoadParam& matName)
{
	SpritePtr sprite = Sprite::Create(mLaunchMode, mResourceMng, NotEmptyOr(matName, MAT_SPRITE));
	if (! imgpath.empty()) 
		sprite->SetTexture(mResourceMng.CreateTextureByFile(mLaunchMode, imgpath));
	return sprite;
}

SpritePtr RenderableFactory::CreateColorLayer(const MaterialLoadParam& matName)
{
	return Sprite::Create(mLaunchMode, mResourceMng, NotEmptyOr(matName, MAT_LAYERCOLOR));
}

MeshPtr RenderableFactory::CreateMesh(int vertCount, int indexCount, const MaterialLoadParam& matName)
{
	return Mesh::Create(mLaunchMode, mResourceMng, NotEmptyOr(matName, MAT_SPRITE), 
		vertCount, indexCount);
}

CubePtr RenderableFactory::CreateCube(const Eigen::Vector3f& center, const Eigen::Vector3f& halfsize, unsigned bgra, const MaterialLoadParam& matName)
{
	auto cube = Cube::Create(mLaunchMode, mResourceMng, NotEmptyOr(matName, MaterialLoadParam(MAT_LAYERCOLOR, "Cube")));
	cube->SetPosition(center);
	cube->SetHalfSize(halfsize);
	cube->SetColor(bgra);
	return cube;
}

AssimpModelPtr RenderableFactory::CreateAssimpModel(const MaterialLoadParam& matName)
{
	return AssimpModel::Create(mLaunchMode, mResourceMng, NotEmptyOr(matName, MAT_MODEL));
}

LabelPtr RenderableFactory::CreateLabel(string_cref fontPath, int fontSize)
{
	FontPtr font = mFontCache->GetFont(fontPath, fontSize);
	return Label::Create(mLaunchMode, mResourceMng, MAT_LABEL, font);
}

SkyBoxPtr RenderableFactory::CreateSkybox(string_cref imgpath)
{
	return SkyBox::Create(mLaunchMode, mResourceMng, MAT_SKYBOX, imgpath);
}

PostProcessPtr RenderableFactory::CreatePostProcessEffect(string_cref effectName, Camera& camera)
{
	PostProcessPtr process;
	if (effectName == MAT_BLOOM) {
		process = Bloom::Create(mLaunchMode, mResourceMng, effectName, camera.FetchOutput2PostProcess());
		camera.AddPostProcessEffect(process);
	}
	return process;
}

}