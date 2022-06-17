#include "core/mir.h"
#include "core/base/debug.h"
#include "core/base/macros.h"
#include "core/rendersys/d3d11/render_system11.h"
#include "core/rendersys/ogl/render_system_ogl.h"
#include "core/rendersys/render_system.h"
#include "core/rendersys/render_pipeline.h"
#include "core/resource/material_factory.h"
#include "core/resource/assimp_resource.h"
#include "core/resource/texture_factory.h"
#include "core/resource/resource_manager.h"
#include "core/scene/camera.h"
#include "core/scene/light.h"

namespace mir {

Mir::Mir(Launch launchMode)
	:mLchMode(launchMode)
{
	mIoService = CreateInstance<cppcoro::io_service>(8);
}
CoTask<bool> Mir::Initialize(HWND hWnd, std::string workDir) 
{
	TIME_PROFILE("Mir.Initialize");
	
	mWorkDirectory = workDir;
	if (mWorkDirectory.back() != '/') 
		mWorkDirectory.push_back('/');

#if 1
	::CoInitialize(0);
	mRenderSys = std::static_pointer_cast<RenderSystem>(CreateInstance<RenderSystemOGL>());
#else
	mRenderSys = std::static_pointer_cast<RenderSystem>(CreateInstance<RenderSystem11>());
#endif
	if (FAILED(mRenderSys->Initialize(hWnd))) {
		mRenderSys->Dispose();
		CoReturn false;
	}
	
	mResMng = CreateInstance<ResourceManager>(*mRenderSys, mIoService, mWorkDirectory + "shader/");

	mRenderPipe = CreateInstance<RenderPipeline>(*mRenderSys, *mResMng, mConfigure);
	CoAwait mRenderPipe->Initialize(mLchMode, *mResMng);
	
	mRenderableFac = CreateInstance<RenderableFactory>(*mResMng, mLchMode);
	
	mGuiMng = CreateInstance<GuiManager>(hWnd);
	CoAwait mGuiMng->Initialize(mLchMode, *mResMng); 

	mSceneMng = CreateInstance<SceneManager>(*mResMng, mRenderableFac, mGuiMng, mConfigure);

	CoAwait mResMng->SwitchToLaunchService(__LaunchSync__);
	CoReturn true;
}

Mir::~Mir()
{
	Dispose();
}
void Mir::Dispose() 
{
	if (mRenderSys) {
		SAFE_DISPOSE_NULL(mRenderableFac);
		SAFE_DISPOSE_NULL(mSceneMng);
		SAFE_DISPOSE_NULL(mRenderPipe);
		SAFE_DISPOSE_NULL(mGuiMng);
		SAFE_DISPOSE_NULL(mResMng);
		SAFE_DISPOSE_NULL(mRenderSys);
		::CoUninitialize();
	}
}

CoTask<void> Mir::Update(float dt)
{
	DEBUG_LOG_CALLSTK("mir.UpdateFrame");

	mRenderSys->UpdateFrame(dt);
	CoAwait mResMng->UpdateFrame(dt);
	CoAwait mSceneMng->UpdateFrame(dt);
}

CoTask<void> Mir::Render()
{
	DEBUG_LOG_CALLSTK("mir.Render");

	if (mRenderPipe->BeginFrame()) {
		RenderableCollection rends;
		mSceneMng->GetRenderables(rends);
		mRenderPipe->Render(rends, mSceneMng->GetCameras(), mSceneMng->GetLights());
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
void Mir::ExecuteTaskSync(const CoTask<void>& task)
{
	coroutine::ExecuteTaskSync(*mIoService, task);
}

void Mir::ProcessPendingEvent()
{
	mIoService->process_pending_events();
}

CoTask<void> Mir::ScheduleTaskAfter(std::chrono::microseconds time)
{
	CoAwait mIoService->schedule_after(time);
}

}