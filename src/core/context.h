#pragma once
#include "core/rendersys/render_system.h"
#include "core/rendersys/scene_manager.h"
#include "core/renderable/renderable_factory.h"

namespace mir {

class TContext {
	ISceneManager* mSceneMng;
	IRenderSystem* mRenderSys;
	RenderableFactory* mRenderableFac;
public:
	TContext();
	bool Initialize(HWND hWnd);

	~TContext();
	void Dispose();
public:
	ISceneManager* GetSceneMng() { return mSceneMng; }
	IRenderSystem* GetRenderSys() { return mRenderSys; }
	RenderableFactory* GetRenderableFac() { return mRenderableFac; }
};

}