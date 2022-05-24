#include "core/mir.h"
#include "core/base/debug.h"
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

CoTask<bool> Mir::Initialize(HWND hWnd) {
	TIME_PROFILE("Mir.Initialize");
#if 0
	mRenderSys = std::static_pointer_cast<RenderSystem>(CreateInstance<TRenderSystem9>());
#else
	mRenderSys = std::static_pointer_cast<RenderSystem>(CreateInstance<RenderSystem11>());
#endif
	if (FAILED(mRenderSys->Initialize(hWnd))) {
		mRenderSys->Dispose();
		CoReturn false;
	}

	mResourceMng = CreateInstance<ResourceManager>(*mRenderSys, mIoService);
	
	mRenderPipe = CreateInstance<RenderPipeline>(*mRenderSys, *mResourceMng, mConfigure);
	CoAwait mRenderPipe->Initialize(*mResourceMng);
	mRenderableFac = CreateInstance<RenderableFactory>(*mResourceMng, mLaunchMode);

	mSceneMng = CreateInstance<SceneManager>(*mResourceMng, mRenderableFac);
	CoReturn true;
}

void Mir::Dispose() {
	if (mRenderSys) {
		mRenderableFac = nullptr;
		mSceneMng = nullptr;
		
		mResourceMng->Dispose();
		mResourceMng = nullptr;

		mRenderSys->Dispose();
		mRenderSys = nullptr;
	}
}

CoTask<void> Mir::Update(float dt)
{
	mRenderSys->UpdateFrame(dt);
	CoAwait mResourceMng->UpdateFrame(dt);
	CoAwait mSceneMng->UpdateFrame(dt);
}

CoTask<void> Mir::Render()
{
	if (mRenderPipe->BeginFrame()) {
		mRenderSys->ClearFrameBuffer(nullptr, mBackgndColor, 1.0f, 0);

		RenderOperationQueue opQue;
		mSceneMng->GenRenderOperation(opQue);
		mRenderPipe->Render(opQue, mSceneMng->GetCameras(), mSceneMng->GetLights());
		mRenderPipe->EndFrame();
	}
	CoReturn;
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