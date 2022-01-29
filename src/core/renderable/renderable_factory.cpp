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

#define NotEmptyOr(Str, DefStr) (!Str.ShaderVariantName.empty() ? Str : MaterialLoadParam(DefStr))

RenderableFactory::RenderableFactory(ResourceManager& resMng, Launch launchMode)
	: mResourceMng(resMng)
{
	mFontCache = CreateInstance<FontCache>(mResourceMng);
	mLaunchMode = launchMode;
}

CoTask<SpritePtr> RenderableFactory::CreateSprite(std::string imgpath, MaterialLoadParam loadParam)
{
	COROUTINE_VARIABLES_2(imgpath, loadParam);
	SpritePtr sprite;
	ITexturePtr texture;

	res::MaterialInstance material;
	auto t0 = mResourceMng.CreateMaterial(mLaunchMode, material, loadParam);

	if (!imgpath.empty()) {
		auto t1 = mResourceMng.CreateTextureByFile(mLaunchMode, texture, imgpath);
		CoAwait WhenAll(t0, t1);
	}
	else {
		CoAwait t0;
	}

	sprite = Sprite::Create(mLaunchMode, mResourceMng, material);
	sprite->SetTexture(texture);
	return sprite;
}

CoTask<SpritePtr> RenderableFactory::CreateColorLayer(MaterialLoadParam loadParam)
{
	COROUTINE_VARIABLES_1(loadParam);

	return CoAwait CreateSprite("", NotEmptyOr(loadParam, MAT_LAYERCOLOR));
}

CoTask<MeshPtr> RenderableFactory::CreateMesh(int vertCount, int indexCount, MaterialLoadParam param)
{
	COROUTINE_VARIABLES_3(vertCount, indexCount, param);

	res::MaterialInstance material;
	MaterialLoadParam loadParam = NotEmptyOr(param, MAT_SPRITE);
	if (!CoAwait mResourceMng.CreateMaterial(mLaunchMode, material, loadParam))
		return nullptr;

	return Mesh::Create(mLaunchMode, mResourceMng, material, vertCount, indexCount);
}

CoTask<CubePtr> RenderableFactory::CreateCube(Eigen::Vector3f center, Eigen::Vector3f halfsize, unsigned bgra, MaterialLoadParam param)
{
	COROUTINE_VARIABLES_4(center, halfsize, bgra, param);

	res::MaterialInstance material;
	MaterialLoadParam loadParam = NotEmptyOr(param, MAT_LAYERCOLOR "-Cube");
	if (!CoAwait mResourceMng.CreateMaterial(mLaunchMode, material, loadParam))
		return nullptr;

	auto cube = Cube::Create(mLaunchMode, mResourceMng, material);
	cube->SetPosition(center);
	cube->SetHalfSize(halfsize);
	cube->SetColor(bgra);
	return cube;
}

CoTask<AssimpModelPtr> RenderableFactory::CreateAssimpModel(MaterialLoadParam param)
{
	COROUTINE_VARIABLES_1(param);

	CoReturn AssimpModel::Create(mLaunchMode, mResourceMng, res::MaterialInstance());
}

CoTask<LabelPtr> RenderableFactory::CreateLabel(std::string fontPath, int fontSize)
{
	COROUTINE_VARIABLES_2(fontPath, fontSize);

	res::MaterialInstance material;
	MaterialLoadParam loadParam = MAT_LABEL;
	if (!CoAwait mResourceMng.CreateMaterial(mLaunchMode, material, loadParam))
		return nullptr;

	FontPtr font = mFontCache->GetFont(fontPath, fontSize);
	return Label::Create(mLaunchMode, mResourceMng, material, font);
}

CoTask<SkyBoxPtr> RenderableFactory::CreateSkybox(std::string imgpath, MaterialLoadParam param)
{
	COROUTINE_VARIABLES_2(imgpath, param);
	std::vector<CoTask<bool>> tasks;

	res::MaterialInstance material;
	MaterialLoadParam loadParam = NotEmptyOr(param, MAT_SKYBOX);
	tasks.push_back(mResourceMng.CreateMaterial(mLaunchMode, material, loadParam));

	ITexturePtr mainTexture, lutMap, diffuseEnvMap;
	if (!imgpath.empty()) {
		if (!boost::filesystem::is_regular_file(imgpath)) {
			boost::filesystem::path dir(imgpath);
			dir.remove_filename();
			boost::filesystem::path specularEnvPath = dir / "specular_env.dds";
			if (boost::filesystem::exists(specularEnvPath)) {
				boost::filesystem::path lutPath = dir / "lut.png";
				tasks.push_back(mResourceMng.CreateTextureByFile(mLaunchMode, lutMap, lutPath.string()));

				boost::filesystem::path diffuseEnvPath = dir / "diffuse_env.dds";
				tasks.push_back(mResourceMng.CreateTextureByFile(mLaunchMode, diffuseEnvMap, diffuseEnvPath.string()));

				tasks.push_back(mResourceMng.CreateTextureByFile(mLaunchMode, mainTexture, specularEnvPath.string()));
			}
		}
		else {
			tasks.push_back(mResourceMng.CreateTextureByFile(mLaunchMode, mainTexture, imgpath));
		}
	}
	CoAwait WhenAll(std::move(tasks));

	auto skybox = SkyBox::Create(mLaunchMode, mResourceMng, material);
	skybox->SetLutMap(lutMap);
	skybox->SetDiffuseEnvMap(diffuseEnvMap);
	skybox->SetTexture(mainTexture);
	return skybox;
}

CoTask<PostProcessPtr> RenderableFactory::CreatePostProcessEffect(std::string effectName, scene::Camera& camera)
{
	PostProcessPtr process;
	CoReturn process;
}

}