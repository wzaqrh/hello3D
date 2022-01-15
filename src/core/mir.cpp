#include "core/mir.h"
#include "core/rendersys/d3d11/render_system11.h"
#include "core/rendersys/d3d9/render_system9.h"
#include "core/resource/material_factory.h"
#include "core/resource/assimp_resource.h"

namespace mir {

Mir::Mir(Launch launchMode)
	:mLaunchMode(launchMode)
{}
Mir::~Mir()
{}

bool Mir::Initialize(HWND hWnd) {
#if 0
	mRenderSys = std::static_pointer_cast<RenderSystem>(CreateInstance<TRenderSystem9>());
#else
	mRenderSys = std::static_pointer_cast<RenderSystem>(CreateInstance<RenderSystem11>());
#endif
	if (FAILED(mRenderSys->Initialize(hWnd))) {
		mRenderSys->Dispose();
		return false;
	}

	mMaterialFac = CreateInstance<res::MaterialFactory>();
	mAiResourceFac = CreateInstance<res::AiResourceFactory>();
	mResourceMng = CreateInstance<ResourceManager>(*mRenderSys, *mMaterialFac, *mAiResourceFac);
	
	mRenderPipe = CreateInstance<RenderPipeline>(*mRenderSys, *mResourceMng);
	mSceneMng = CreateInstance<SceneManager>(*mResourceMng);

	mRenderableFac = CreateInstance<RenderableFactory>(*mResourceMng, mLaunchMode);
	return true;
}

void Mir::Dispose() {
	if (mRenderSys) {
		mRenderableFac = nullptr;
		mMaterialFac = nullptr;
		mSceneMng = nullptr;
		
		mResourceMng->Dispose();
		mResourceMng = nullptr;

		mRenderSys->Dispose();
		mRenderSys = nullptr;
	}
}

void Mir::Update()
{
	mRenderSys->Update(0);
	mResourceMng->UpdateForLoading();
}

Eigen::Vector2i Mir::WinSize() const
{
	return mRenderSys->WinSize();
}

}