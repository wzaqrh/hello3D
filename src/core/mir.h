#pragma once
#include "core/rendersys/predeclare.h"
#include "core/renderable/predeclare.h"
#include "core/rendersys/render_system.h"
#include "core/rendersys/scene_manager.h"
#include "core/renderable/renderable_factory.h"

namespace mir {

class Mir {
	RenderSystemPtr mRenderSys;
	MaterialFactoryPtr mMaterialFac;
	RenderableFactoryPtr mRenderableFac;
	SceneManagerPtr mSceneMng;
public:
	Mir();
	~Mir();
	bool Initialize(HWND hWnd);
	void Dispose();
public:
	const RenderSystemPtr& RenderSys() { return mRenderSys; }
	const MaterialFactoryPtr& MaterialFac() { return mMaterialFac; }
	const RenderableFactoryPtr& RenderableFac() { return mRenderableFac; }
	const SceneManagerPtr& SceneMng() { return mSceneMng; }
};

}