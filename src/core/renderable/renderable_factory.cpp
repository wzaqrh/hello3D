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
, mLchMode(launchMode)
{
	mFontCache = CreateInstance<FontCache>(mResMng);
}

RenderableFactory::~RenderableFactory()
{
	DEBUG_LOG_MEMLEAK("rendFac.destrcutor");
	Dispose();
}
void RenderableFactory::Dispose()
{
	if (mFontCache) {
		DEBUG_LOG_MEMLEAK("rendFac.dispose");
		mFontCache = nullptr;
	}
}

CoTask<bool> RenderableFactory::CreatePaint3D(rend::Paint3DPtr& paint)
{
	//BOOST_ASSERT(! mResMng.IsCurrentInAsyncService());
	COROUTINE_VARIABLES_1(paint);

	res::MaterialInstance matLine;
	MaterialLoadParam paramLine = MAT_LINE_PAINT;
	CoAwait mResMng.CreateMaterial(matLine, mLchMode, paramLine);

	res::MaterialInstance matTri;
	MaterialLoadParam paramTri = MAT_PAINT;
	CoAwait mResMng.CreateMaterial(matTri, mLchMode, paramTri);

	paint = CreateInstance<Paint3D>(mLchMode, mResMng, matTri, matLine);

	CoAwait mResMng.SwitchToLaunchService(__LaunchSync__);
	CoReturn true;
}

CoTask<bool> RenderableFactory::CreatePostProcessEffect(rend::PostProcessPtr& effect, MaterialLoadParam loadParam)
{
	//BOOST_ASSERT(! mResMng.IsCurrentInAsyncService());
	COROUTINE_VARIABLES_1(loadParam);

	res::MaterialInstance material;
	if (!CoAwait mResMng.CreateMaterial(material, mLchMode, loadParam))
		CoReturn false;

	effect = CreateInstance<PostProcess>(mLchMode, mResMng, material);
#if MIR_GRAPHICS_DEBUG
	CoAwait CreatePaint3D(effect->mDebugPaint);
#endif

	CoAwait mResMng.SwitchToLaunchService(__LaunchSync__);
	CoReturn true;
}

CoTask<bool> RenderableFactory::CreateSprite(rend::SpritePtr& sprite, std::string imgpath, MaterialLoadParam param)
{
	//BOOST_ASSERT(! mResMng.IsCurrentInAsyncService());
	COROUTINE_VARIABLES_2(imgpath, param);
	
	res::MaterialInstance material;
	MaterialLoadParam loadParam = NotEmptyOr(param, MAT_SPRITE);
	auto t0 = mResMng.CreateMaterial(material, mLchMode, loadParam);

	ITexturePtr texture;
	if (!imgpath.empty()) {
		auto t1 = mResMng.CreateTextureByFile(texture, mLchMode, imgpath);
		CoAwait WhenAll(t0, t1);
	}
	else {
		CoAwait t0;
	}

	sprite = CreateInstance<Sprite>(mLchMode, mResMng, material);
	sprite->SetTexture(texture);
#if MIR_GRAPHICS_DEBUG
	CoAwait CreatePaint3D(sprite->mDebugPaint);
#endif

	CoAwait mResMng.SwitchToLaunchService(__LaunchSync__);
	CoReturn true;
}

CoTask<bool> RenderableFactory::CreateMesh(rend::MeshPtr& mesh, int vertCount, int indexCount, MaterialLoadParam param)
{
	//BOOST_ASSERT(! mResMng.IsCurrentInAsyncService());
	COROUTINE_VARIABLES_3(vertCount, indexCount, param);

	res::MaterialInstance material;
	MaterialLoadParam loadParam = NotEmptyOr(param, MAT_SPRITE);
	if (!CoAwait mResMng.CreateMaterial(material, mLchMode, loadParam))
		CoReturn false;

	mesh = CreateInstance<Mesh>(mLchMode, mResMng, material, vertCount, indexCount);
#if MIR_GRAPHICS_DEBUG
	CoAwait CreatePaint3D(mesh->mDebugPaint);
#endif

	CoAwait mResMng.SwitchToLaunchService(__LaunchSync__);
	CoReturn true;
}

CoTask<bool> RenderableFactory::CreateCube(rend::CubePtr& cube, Eigen::Vector3f center, Eigen::Vector3f halfsize, unsigned bgra, MaterialLoadParam param)
{
	//BOOST_ASSERT(! mResMng.IsCurrentInAsyncService());
	COROUTINE_VARIABLES_4(center, halfsize, bgra, param);

	res::MaterialInstance material;
	MaterialLoadParam loadParam = NotEmptyOr(param, MAT_LAYERCOLOR "-Cube");
	if (!CoAwait mResMng.CreateMaterial(material, mLchMode, loadParam))
		CoReturn false;

	cube = CreateInstance<Cube>(mLchMode, mResMng, material);
	cube->SetPosition(center);
	cube->SetHalfSize(halfsize);
	cube->SetColor(bgra);
#if MIR_GRAPHICS_DEBUG
	CoAwait CreatePaint3D(cube->mDebugPaint);
#endif

	CoAwait mResMng.SwitchToLaunchService(__LaunchSync__);
	CoReturn true;
}

CoTask<bool> RenderableFactory::CreateAssimpModel(rend::AssimpModelPtr& model, MaterialLoadParam param)
{
	//BOOST_ASSERT(! mResMng.IsCurrentInAsyncService());
	COROUTINE_VARIABLES_1(param);

	model = CreateInstance<AssimpModel>(mLchMode, mResMng, param); BOOST_ASSERT(model);
#if MIR_GRAPHICS_DEBUG
	//CoAwait CreatePaint3D(model->mDebugPaint);
#endif

	CoAwait mResMng.SwitchToLaunchService(__LaunchSync__);
	CoReturn true;
}

CoTask<bool> RenderableFactory::CreateLabel(rend::LabelPtr& label, std::string fontPath, int fontSize)
{
	//BOOST_ASSERT(! mResMng.IsCurrentInAsyncService());
	COROUTINE_VARIABLES_2(fontPath, fontSize);

	res::MaterialInstance material;
	MaterialLoadParam loadParam = MAT_LABEL;
	if (!CoAwait mResMng.CreateMaterial(material, mLchMode, loadParam))
		CoReturn false;
	CoAwait mResMng.SwitchToLaunchService(__LaunchSync__);

	FontPtr font = mFontCache->GetFont(fontPath, fontSize);
	label = CreateInstance<Label>(mLchMode, mResMng, material, font);
#if MIR_GRAPHICS_DEBUG
	CoAwait CreatePaint3D(label->mDebugPaint);
#endif

	CoAwait mResMng.SwitchToLaunchService(__LaunchSync__);
	CoReturn true;
}

CoTask<bool> RenderableFactory::CreateSkybox(rend::SkyBoxPtr& skybox, std::vector<std::string> filenames, MaterialLoadParam param)
{
	//BOOST_ASSERT(! mResMng.IsCurrentInAsyncService());
	COROUTINE_VARIABLES_2(filenames, param);
	std::vector<CoTask<bool>> tasks;

	res::MaterialInstance material;
	MaterialLoadParam loadParam = NotEmptyOr(param, MAT_SKYBOX);
	tasks.push_back(mResMng.CreateMaterial(material, mLchMode, loadParam));

	ITexturePtr specEnvMap, diffuseEnvMap, sheenEnvMap, lutMap;
	if (filenames.size() >= 0) tasks.push_back(mResMng.CreateTextureByFile(specEnvMap, mLchMode, filenames[0]));
	if (filenames.size() >= 1) tasks.push_back(mResMng.CreateTextureByFile(diffuseEnvMap, mLchMode, filenames[1]));
	if (filenames.size() >= 2) tasks.push_back(mResMng.CreateTextureByFile(lutMap, mLchMode, filenames[2]));
	if (filenames.size() >= 3) tasks.push_back(mResMng.CreateTextureByFile(sheenEnvMap, mLchMode, filenames[3]));
	CoAwait WhenAll(std::move(tasks));

	skybox = CreateInstance<SkyBox>(mLchMode, mResMng, material);
	skybox->SetLutMap(lutMap);
	skybox->SetDiffuseEnvMap(diffuseEnvMap);
	skybox->SetTexture(specEnvMap);
	skybox->SetSheenMap(sheenEnvMap);

	CoAwait mResMng.SwitchToLaunchService(__LaunchSync__);
	CoReturn true;
}

}