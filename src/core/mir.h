#pragma once
#include "core/rendersys/render_system.h"
#include "core/rendersys/scene_manager.h"
#include "core/renderable/renderable_factory.h"

namespace mir {

class Mir {
	IRenderSystemPtr mRenderSys;
	MaterialFactoryPtr mMaterialFac;
	RenderableFactoryPtr mRenderableFac;
	SceneManagerPtr mSceneMng;
public:
	Mir();
	~Mir();
	bool Initialize(HWND hWnd);
	void Dispose();
public:
	const IRenderSystemPtr& GetRenderSys() { return mRenderSys; }
	const MaterialFactoryPtr& GetMatFac() { return mMaterialFac; }
	const RenderableFactoryPtr& GetRenderableFac() { return mRenderableFac; }
	const SceneManagerPtr& GetSceneMng() { return mSceneMng; }
};

}