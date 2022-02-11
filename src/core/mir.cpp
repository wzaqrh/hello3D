#include "core/mir.h"
#include "core/rendersys/d3d11/render_system11.h"
#include "core/rendersys/d3d9/render_system9.h"
#include "core/resource/material_factory.h"
#include "core/resource/assimp_resource.h"

namespace mir {

Mir::Mir(Launch launchMode)
	:mLaunchMode(launchMode)
{
	mBackgndColor = Eigen::Vector4f(0.1f, 0.1f, 0.1f, 0.0f);
	mIoService = CreateInstance<cppcoro::io_service>(8);
}
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
	mResourceMng = CreateInstance<ResourceManager>(*mRenderSys, *mMaterialFac, *mAiResourceFac, mIoService);
	
	mRenderPipe = CreateInstance<RenderPipeline>(*mRenderSys, *mResourceMng);
	mRenderableFac = CreateInstance<RenderableFactory>(*mResourceMng, mLaunchMode);

	mSceneMng = CreateInstance<SceneManager>(*mResourceMng, mRenderableFac);
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

void Mir::Update(float dt)
{
	mRenderSys->Update(dt);
	mResourceMng->UpdateForLoading();
	mSceneMng->UpdateFrame(dt);
}

void Mir::Render()
{
	if (mRenderPipe->BeginFrame()) {
		mRenderSys->ClearFrameBuffer(nullptr, mBackgndColor, 1.0f, 0);

		RenderOperationQueue opQue;
		mSceneMng->GenRenderOperation(opQue);
		mRenderPipe->Render(opQue, mSceneMng->GetCameras(), mSceneMng->GetLights());
		mRenderPipe->EndFrame();
	}
}

Eigen::Vector2i Mir::WinSize() const
{
	return mRenderSys->WinSize();
}

const mir::scene::LightFactoryPtr& Mir::LightFac() const
{
	return mSceneMng->GetLightFac();
}
const mir::scene::CameraFactoryPtr& Mir::CameraFac() const
{
	return mSceneMng->GetCameraFac();
}
const mir::SceneNodeFactoryPtr& Mir::NodeFac() const
{
	return mSceneMng->GetNodeFac();
}

void Mir::ExecuteTaskSync(const CoTask<bool>& task)
{
	coroutine::ExecuteTaskSync(*mIoService, task);
}

void Mir::ProcessPendingEvent()
{
	mIoService->process_pending_events();
}

}