#pragma once
#include "core/mir_export.h"
#include "core/rendersys/predeclare.h"
#include "core/renderable/predeclare.h"
#include "core/rendersys/render_system.h"
#include "core/renderable/renderable_factory.h"
#include "core/resource/resource_manager.h"
#include "core/rendersys/scene_manager.h"
#include "core/rendersys/render_pipeline.h"

namespace mir {

class MIR_CORE_API Mir {
	RenderSystemPtr mRenderSys;
	MaterialFactoryPtr mMaterialFac;
	AiResourceFactoryPtr mAiResourceFac;
	ResourceManagerPtr mResourceMng;
	RenderPipelinePtr mRenderPipe;
	RenderableFactoryPtr mRenderableFac;
	SceneManagerPtr mSceneMng;
public:
	Mir();
	~Mir();
	bool Initialize(HWND hWnd);
	void Dispose();
	void Update();
public:
	inline const RenderSystemPtr& RenderSys() { return mRenderSys; }
	inline const ResourceManagerPtr& ResourceMng() { return mResourceMng; }
	inline const RenderPipelinePtr& RenderPipe() { return mRenderPipe; }
	inline const RenderableFactoryPtr& RenderableFac() { return mRenderableFac; }
	inline const SceneManagerPtr& SceneMng() { return mSceneMng; }
};

}