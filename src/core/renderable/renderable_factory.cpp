#include "core/renderable/renderable_factory.h"
#include "core/renderable/skybox.h"
#include "core/renderable/sprite.h"
#include "core/renderable/mesh.h"
#include "core/renderable/font.h"
#include "core/renderable/label.h"
#include "core/renderable/cube.h"
#include "core/renderable/assimp_model.h"
#include "core/renderable/post_process.h"
#include "core/renderable/paint3d.h"
#include "core/base/debug.h"
#include "core/scene/camera.h"
#include "core/resource/material_name.h"
#include "core/resource/material_factory.h"
#include "core/resource/texture_factory.h"
#include "core/rendersys/render_pipeline.h"
#include "core/resource/resource_manager.h"

using namespace mir::rend;

namespace mir {

#define NotEmptyOr(Str, DefStr) (!Str.ShaderVariantName.empty() ? Str : MaterialLoadParam(DefStr))

RenderableFactory::RenderableFactory(ResourceManager& resMng, Launch launchMode)
: mResMng(resMng)
, mLaunchMode(launchMode)
{
	mFontCache = CreateInstance<FontCache>(mResMng);
}

CoTask<bool> RenderableFactory::CreatePaint3D(rend::Paint3DPtr& paint)
{
	res::MaterialInstance matLine;
	MaterialLoadParam paramLine = MAT_LINE_PAINT;
	CoAwait mResMng.CreateMaterial(matLine, mLaunchMode, paramLine);

	res::MaterialInstance matTri;
	MaterialLoadParam paramTri = MAT_PAINT;
	CoAwait mResMng.CreateMaterial(matTri, mLaunchMode, paramTri);

	paint = CreateInstance<Paint3D>(mLaunchMode, mResMng, matTri, matLine);
	CoReturn true;
}

CoTask<bool> RenderableFactory::CreatePostProcessEffect(rend::PostProcessPtr& effect, MaterialLoadParam loadParam)
{
	COROUTINE_VARIABLES_1(loadParam);

	res::MaterialInstance material;
	if (!CoAwait mResMng.CreateMaterial(material, mLaunchMode, loadParam))
		CoReturn false;

	effect = CreateInstance<PostProcess>(mLaunchMode, mResMng, material);
#if MIR_GRAPHICS_DEBUG
	CoAwait CreatePaint3D(effect->mDebugPaint);
#endif
	CoReturn true;
}

CoTask<bool> RenderableFactory::CreateSprite(rend::SpritePtr& sprite, std::string imgpath, MaterialLoadParam param)
{
	COROUTINE_VARIABLES_2(imgpath, param);
	
	res::MaterialInstance material;
	MaterialLoadParam loadParam = NotEmptyOr(param, MAT_SPRITE);
	auto t0 = mResMng.CreateMaterial(material, mLaunchMode, loadParam);

	ITexturePtr texture;
	if (!imgpath.empty()) {
		auto t1 = mResMng.CreateTextureByFile(texture, mLaunchMode, imgpath);
		CoAwait WhenAll(t0, t1);
	}
	else {
		CoAwait t0;
	}

	sprite = CreateInstance<Sprite>(mLaunchMode, mResMng, material);
	sprite->SetTexture(texture);
#if MIR_GRAPHICS_DEBUG
	CoAwait CreatePaint3D(sprite->mDebugPaint);
#endif
	CoReturn true;
}

CoTask<bool> RenderableFactory::CreateMesh(rend::MeshPtr& mesh, int vertCount, int indexCount, MaterialLoadParam param)
{
	COROUTINE_VARIABLES_3(vertCount, indexCount, param);

	res::MaterialInstance material;
	MaterialLoadParam loadParam = NotEmptyOr(param, MAT_SPRITE);
	if (!CoAwait mResMng.CreateMaterial(material, mLaunchMode, loadParam))
		CoReturn false;

	mesh = CreateInstance<Mesh>(mLaunchMode, mResMng, material, vertCount, indexCount);
#if MIR_GRAPHICS_DEBUG
	CoAwait CreatePaint3D(mesh->mDebugPaint);
#endif
	CoReturn true;
}

CoTask<bool> RenderableFactory::CreateCube(rend::CubePtr& cube, Eigen::Vector3f center, Eigen::Vector3f halfsize, unsigned bgra, MaterialLoadParam param)
{
	COROUTINE_VARIABLES_4(center, halfsize, bgra, param);

	res::MaterialInstance material;
	MaterialLoadParam loadParam = NotEmptyOr(param, MAT_LAYERCOLOR "-Cube");
	if (!CoAwait mResMng.CreateMaterial(material, mLaunchMode, loadParam))
		CoReturn false;

	cube = CreateInstance<Cube>(mLaunchMode, mResMng, material);
	cube->SetPosition(center);
	cube->SetHalfSize(halfsize);
	cube->SetColor(bgra);
#if MIR_GRAPHICS_DEBUG
	CoAwait CreatePaint3D(cube->mDebugPaint);
#endif
	CoReturn true;
}

CoTask<bool> RenderableFactory::CreateAssimpModel(rend::AssimpModelPtr& model, MaterialLoadParam param)
{
	COROUTINE_VARIABLES_1(param);

	model = CreateInstance<AssimpModel>(mLaunchMode, mResMng, res::MaterialInstance()); BOOST_ASSERT(model);
#if MIR_GRAPHICS_DEBUG
	//CoAwait CreatePaint3D(model->mDebugPaint);
#endif
	CoReturn true;
}

CoTask<bool> RenderableFactory::CreateLabel(rend::LabelPtr& label, std::string fontPath, int fontSize)
{
	COROUTINE_VARIABLES_2(fontPath, fontSize);

	res::MaterialInstance material;
	MaterialLoadParam loadParam = MAT_LABEL;
	if (!CoAwait mResMng.CreateMaterial(material, mLaunchMode, loadParam))
		CoReturn false;

	FontPtr font = mFontCache->GetFont(fontPath, fontSize);
	label = CreateInstance<Label>(mLaunchMode, mResMng, material, font);
#if MIR_GRAPHICS_DEBUG
	CoAwait CreatePaint3D(label->mDebugPaint);
#endif
	CoReturn true;
}

CoTask<bool> RenderableFactory::CreateSkybox(rend::SkyBoxPtr& skybox, std::string imgpath, MaterialLoadParam param)
{
	COROUTINE_VARIABLES_2(imgpath, param);
	std::vector<CoTask<bool>> tasks;

	res::MaterialInstance material;
	MaterialLoadParam loadParam = NotEmptyOr(param, MAT_SKYBOX);
	tasks.push_back(mResMng.CreateMaterial(material, mLaunchMode, loadParam));

	ITexturePtr mainTexture, lutMap, diffuseEnvMap;
	if (!imgpath.empty()) {
		if (!boost::filesystem::is_regular_file(imgpath)) {
			boost::filesystem::path dir(imgpath);
			dir.remove_filename();
			boost::filesystem::path specularEnvPath = dir / "specular_env.dds";
			if (boost::filesystem::exists(specularEnvPath)) {
				boost::filesystem::path lutPath = dir / "lut.png";
				tasks.push_back(mResMng.CreateTextureByFile(lutMap, mLaunchMode, lutPath.string()));

				boost::filesystem::path diffuseEnvPath = dir / "diffuse_env.dds";
				tasks.push_back(mResMng.CreateTextureByFile(diffuseEnvMap, mLaunchMode, diffuseEnvPath.string()));

				tasks.push_back(mResMng.CreateTextureByFile(mainTexture, mLaunchMode, specularEnvPath.string()));
			}
		}
		else {
			tasks.push_back(mResMng.CreateTextureByFile(mainTexture, mLaunchMode, imgpath));
		}
	}
	CoAwait WhenAll(std::move(tasks));

	skybox = CreateInstance<SkyBox>(mLaunchMode, mResMng, material);
	skybox->SetLutMap(lutMap);
	skybox->SetDiffuseEnvMap(diffuseEnvMap);
	skybox->SetTexture(mainTexture);
	CoReturn true;
}

}