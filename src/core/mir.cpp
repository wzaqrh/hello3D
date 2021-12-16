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
	mRenderSys = std::static_pointer_cast<RenderSystem>(std::make_shared<TRenderSystem9>());
#else
	mRenderSys = std::static_pointer_cast<RenderSystem>(std::make_shared<RenderSystem11>());
#endif
	if (FAILED(mRenderSys->Initialize(hWnd))) {
		mRenderSys->Dispose();
		return false;
	}

	mMaterialFac = std::make_shared<MaterialFactory>();
	mAiResourceFac = std::make_shared<AiResourceFactory>();
	mResourceMng = std::make_shared<ResourceManager>(*mRenderSys, *mMaterialFac, *mAiResourceFac);
	
	mRenderPipe = std::make_shared<RenderPipeline>(*mRenderSys, *mResourceMng);
	mSceneMng = std::make_shared<SceneManager>(*mResourceMng);

	mRenderableFac = std::make_shared<RenderableFactory>(*mResourceMng, mLaunchMode);
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