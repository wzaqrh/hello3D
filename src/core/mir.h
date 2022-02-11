#pragma once
#include "core/mir_config.h"
#include "core/mir_export.h"
#include "core/base/cppcoro.h"
#include "core/base/launch.h"
#include "core/predeclare.h"
#include "core/rendersys/render_system.h"
#include "core/renderable/renderable_factory.h"
#include "core/resource/resource_manager.h"
#include "core/scene/scene_manager.h"
#include "core/rendersys/render_pipeline.h"

namespace mir {

class MIR_CORE_API Mir {
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	Mir(Launch launchMode);
	~Mir();
	bool Initialize(HWND hWnd);
	void Dispose();

	void Update(float dt);
	void Render();
public:
	Eigen::Vector2i WinSize() const;
	inline const RenderSystemPtr& RenderSys() const { return mRenderSys; }
	inline const ResourceManagerPtr& ResourceMng() const { return mResourceMng; }
	inline const RenderPipelinePtr& RenderPipe() const { return mRenderPipe; }
	inline const RenderableFactoryPtr& RenderableFac() const { return mRenderableFac; }
	inline const SceneManagerPtr& SceneMng() const { return mSceneMng; }
	inline const scene::LightFactoryPtr& LightFac() const;
	inline const scene::CameraFactoryPtr& CameraFac() const;
	inline const SceneNodeFactoryPtr& NodeFac() const;
	void ExecuteTaskSync(const CoTask<bool>& task);
private:
	Launch mLaunchMode;
	RenderSystemPtr mRenderSys;
	res::MaterialFactoryPtr mMaterialFac;
	res::AiResourceFactoryPtr mAiResourceFac;
	std::shared_ptr<cppcoro::io_service> mIoService;
	ResourceManagerPtr mResourceMng;
	RenderPipelinePtr mRenderPipe;
	RenderableFactoryPtr mRenderableFac;
	SceneManagerPtr mSceneMng;
private:
	Eigen::Vector4f mBackgndColor;
};

}