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
, mLaunchMode(launchMode)
{
	mFontCache = CreateInstance<FontCache>(mResourceMng);
}

CoTask<bool> RenderableFactory::CreateSprite(rend::SpritePtr& sprite, std::string imgpath, MaterialLoadParam loadParam)
{
	COROUTINE_VARIABLES_2(imgpath, loadParam);
	ITexturePtr texture;

	res::MaterialInstance material;
	auto t0 = mResourceMng.CreateMaterial(material, mLaunchMode, loadParam);

	if (!imgpath.empty()) {
		auto t1 = mResourceMng.CreateTextureByFile(texture, mLaunchMode, imgpath);
		CoAwait WhenAll(t0, t1);
	}
	else {
		CoAwait t0;
	}

	sprite = Sprite::Create(mLaunchMode, mResourceMng, material);
	sprite->SetTexture(texture);
	CoReturn true;
}

CoTask<bool> RenderableFactory::CreateMesh(rend::MeshPtr& mesh, int vertCount, int indexCount, MaterialLoadParam param)
{
	COROUTINE_VARIABLES_3(vertCount, indexCount, param);

	res::MaterialInstance material;
	MaterialLoadParam loadParam = NotEmptyOr(param, MAT_SPRITE);
	if (!CoAwait mResourceMng.CreateMaterial(material, mLaunchMode, loadParam))
		CoReturn false;

	mesh = Mesh::Create(mLaunchMode, mResourceMng, material, vertCount, indexCount);
	CoReturn true;
}

CoTask<bool> RenderableFactory::CreateCube(rend::CubePtr& cube, Eigen::Vector3f center, Eigen::Vector3f halfsize, unsigned bgra, MaterialLoadParam param)
{
	COROUTINE_VARIABLES_4(center, halfsize, bgra, param);

	res::MaterialInstance material;
	MaterialLoadParam loadParam = NotEmptyOr(param, MAT_LAYERCOLOR "-Cube");
	if (!CoAwait mResourceMng.CreateMaterial(material, mLaunchMode, loadParam))
		CoReturn false;

	cube = Cube::Create(mLaunchMode, mResourceMng, material);
	cube->SetPosition(center);
	cube->SetHalfSize(halfsize);
	cube->SetColor(bgra);
	CoReturn true;
}

CoTask<bool> RenderableFactory::CreateAssimpModel(rend::AssimpModelPtr& model, MaterialLoadParam param)
{
	COROUTINE_VARIABLES_1(param);

	model = AssimpModel::Create(mLaunchMode, mResourceMng, res::MaterialInstance());
	BOOST_ASSERT(model);
	CoReturn true;
}

CoTask<bool> RenderableFactory::CreateLabel(rend::LabelPtr& label, std::string fontPath, int fontSize)
{
	COROUTINE_VARIABLES_2(fontPath, fontSize);

	res::MaterialInstance material;
	MaterialLoadParam loadParam = MAT_LABEL;
	if (!CoAwait mResourceMng.CreateMaterial(material, mLaunchMode, loadParam))
		CoReturn false;

	FontPtr font = mFontCache->GetFont(fontPath, fontSize);
	label = Label::Create(mLaunchMode, mResourceMng, material, font);
	CoReturn true;
}

CoTask<bool> RenderableFactory::CreateSkybox(rend::SkyBoxPtr& skybox, std::string imgpath, MaterialLoadParam param)
{
	COROUTINE_VARIABLES_2(imgpath, param);
	std::vector<CoTask<bool>> tasks;

	res::MaterialInstance material;
	MaterialLoadParam loadParam = NotEmptyOr(param, MAT_SKYBOX);
	tasks.push_back(mResourceMng.CreateMaterial(material, mLaunchMode, loadParam));

	ITexturePtr mainTexture, lutMap, diffuseEnvMap;
	if (!imgpath.empty()) {
		if (!boost::filesystem::is_regular_file(imgpath)) {
			boost::filesystem::path dir(imgpath);
			dir.remove_filename();
			boost::filesystem::path specularEnvPath = dir / "specular_env.dds";
			if (boost::filesystem::exists(specularEnvPath)) {
				boost::filesystem::path lutPath = dir / "lut.png";
				tasks.push_back(mResourceMng.CreateTextureByFile(lutMap, mLaunchMode, lutPath.string()));

				boost::filesystem::path diffuseEnvPath = dir / "diffuse_env.dds";
				tasks.push_back(mResourceMng.CreateTextureByFile(diffuseEnvMap, mLaunchMode, diffuseEnvPath.string()));

				tasks.push_back(mResourceMng.CreateTextureByFile(mainTexture, mLaunchMode, specularEnvPath.string()));
			}
		}
		else {
			tasks.push_back(mResourceMng.CreateTextureByFile(mainTexture, mLaunchMode, imgpath));
		}
	}
	CoAwait WhenAll(std::move(tasks));

	skybox = SkyBox::Create(mLaunchMode, mResourceMng, material);
	skybox->SetLutMap(lutMap);
	skybox->SetDiffuseEnvMap(diffuseEnvMap);
	skybox->SetTexture(mainTexture);
	CoReturn true;
}

}