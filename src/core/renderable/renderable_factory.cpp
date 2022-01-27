#include "core/renderable/renderable_factory.h"
#include "core/renderable/skybox.h"
#include "core/renderable/sprite.h"
#include "core/renderable/mesh.h"
#include "core/renderable/font.h"
#include "core/renderable/label.h"
#include "core/renderable/cube.h"
#include "core/renderable/assimp_model.h"
#include "core/renderable/post_process.h"
#include "core/base/debug.h"
#include "core/scene/camera.h"
#include "core/resource/material_factory.h"
#include "core/rendersys/render_pipeline.h"
#include "core/resource/resource_manager.h"

using namespace mir::rend;

namespace mir {

#define NotEmptyOr(Str, DefStr) (!Str.ShaderVariantName.empty() ? Str : DefStr)

RenderableFactory::RenderableFactory(ResourceManager& resMng, Launch launchMode)
	: mResourceMng(resMng)
{
	mFontCache = CreateInstance<FontCache>(mResourceMng);
	mLaunchMode = launchMode;
}

CoTask<SpritePtr> RenderableFactory::CreateSprite(string_cref imgpath, const MaterialLoadParam& loadParam)
{
	COROUTINE_VARIABLES_2(imgpath, loadParam);

	SpritePtr sprite = CoAwait Sprite::Create(mLaunchMode, mResourceMng, NotEmptyOr(loadParam, MAT_SPRITE));
	if (! imgpath.empty()) 
		sprite->SetTexture(CoAwait mResourceMng.CreateTextureByFile(mLaunchMode, imgpath));
	return sprite;
}

CoTask<SpritePtr> RenderableFactory::CreateColorLayer(const MaterialLoadParam& loadParam)
{
	COROUTINE_VARIABLES_1(loadParam);

	return CoAwait Sprite::Create(mLaunchMode, mResourceMng, NotEmptyOr(loadParam, MAT_LAYERCOLOR));
}

//#include <cppcoro/fmap.hpp>
CoTask<MeshPtr> RenderableFactory::CreateMesh(int vertCount, int indexCount, const MaterialLoadParam& loadParam)
{
	COROUTINE_VARIABLES_3(vertCount, indexCount, loadParam);

	return CoAwait Mesh::Create(mLaunchMode, mResourceMng, NotEmptyOr(loadParam, MAT_SPRITE), vertCount, indexCount);
}

CoTask<CubePtr> RenderableFactory::CreateCube(const Eigen::Vector3f& center, const Eigen::Vector3f& halfsize, unsigned bgra, const MaterialLoadParam& loadParam)
{
	COROUTINE_VARIABLES_4(center, halfsize, bgra, loadParam);

	auto cube = CoAwait Cube::Create(mLaunchMode, mResourceMng, NotEmptyOr(loadParam, MAT_LAYERCOLOR "-Cube"));
	cube->SetPosition(center);
	cube->SetHalfSize(halfsize);
	cube->SetColor(bgra);
	return cube;
}

CoTask<AssimpModelPtr> RenderableFactory::CreateAssimpModel(const MaterialLoadParam& loadParam)
{
	COROUTINE_VARIABLES_1(loadParam);

	return CoAwait AssimpModel::Create(mLaunchMode, mResourceMng, NotEmptyOr(loadParam, MAT_MODEL));
}

CoTask<LabelPtr> RenderableFactory::CreateLabel(string_cref fontPath, int fontSize)
{
	COROUTINE_VARIABLES_2(fontPath, fontSize);

	FontPtr font = mFontCache->GetFont(fontPath, fontSize);
	return CoAwait Label::Create(mLaunchMode, mResourceMng, MAT_LABEL, font);
}

CoTask<SkyBoxPtr> RenderableFactory::CreateSkybox(string_cref imgpath, const MaterialLoadParam& loadParam)
{
	COROUTINE_VARIABLES_2(imgpath, loadParam);

	return CoAwait SkyBox::Create(mLaunchMode, mResourceMng, NotEmptyOr(loadParam, MAT_SKYBOX), imgpath);
}

CoTask<PostProcessPtr> RenderableFactory::CreatePostProcessEffect(string_cref effectName, scene::Camera& camera)
{
	PostProcessPtr process;
	CoReturn process;
}

}