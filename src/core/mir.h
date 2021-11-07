#pragma once
#include "core/rendersys/render_system.h"
#include "core/rendersys/scene_manager.h"
#include "core/renderable/renderable_factory.h"

namespace mir {

class Mir {
	SceneManager* mSceneMng;
	IRenderSystem* mRenderSys;
	RenderableFactory* mRenderableFac;
public:
	Mir();
	~Mir();
	bool Initialize(HWND hWnd);
	void Dispose();
public:
	SceneManager* GetSceneMng() { return mSceneMng; }
	IRenderSystem* GetRenderSys() { return mRenderSys; }
	RenderableFactory* GetRenderableFac() { return mRenderableFac; }
};

}