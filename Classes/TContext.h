#pragma once
#include "IRenderSystem.h"
#include "ISceneManager.h"
#include "RenderableFactory.h"

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