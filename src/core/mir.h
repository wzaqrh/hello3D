#pragma once
#include "core/mir_config.h"
#include "core/mir_export.h"
#include "core/base/cppcoro.h"
#include "core/base/launch.h"
#include "core/predeclare.h"
#include "core/rendersys/base/platform.h"
#include "core/renderable/renderable_factory.h"
#include "core/scene/scene_manager.h"

namespace mir {

class MIR_CORE_API Mir {
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	Mir(Launch launchMode);
	~Mir();
	CoTask<bool> Initialize(HWND hWnd, std::string workDir, PlatformType platform = kPlatformOpengl);
	void Dispose();

	CoTask<void> Update(float dt);
	CoTask<void> Render();
public:
	Eigen::Vector2i WinSize() const;
	inline const Configure& Config() const { return mConfigure; }
	inline const RenderSystemPtr& RenderSys() const { return mRenderSys; }
	inline const ResourceManagerPtr& ResourceMng() const { return mResMng; }
	inline const RenderPipelinePtr& RenderPipe() const { return mRenderPipe; }
	inline const RenderableFactoryPtr& RenderableFac() const { return mRenderableFac; }
	inline const SceneManagerPtr& SceneMng() const { return mSceneMng; }
	const scene::LightFactoryPtr& LightFac() const;
	const scene::CameraFactoryPtr& CameraFac() const;
	const SceneNodeFactoryPtr& NodeFac() const;
	inline const GuiManagerPtr& GuiMng() const { return mGuiMng; }
	void ExecuteTaskSync(const CoTask<bool>& task);
	void ExecuteTaskSync(const CoTask<void>& task);
	void ProcessPendingEvent();
	CoTask<void> ScheduleTaskAfter(std::chrono::microseconds time);
private:
	Launch mLchMode;
	Configure mConfigure;
	RenderSystemPtr mRenderSys;
	std::shared_ptr<cppcoro::io_service> mIoService;
	std::string mWorkDirectory;
	ResourceManagerPtr mResMng;
	RenderPipelinePtr mRenderPipe;
	RenderableFactoryPtr mRenderableFac;
	SceneManagerPtr mSceneMng;
	GuiManagerPtr mGuiMng;
};

}