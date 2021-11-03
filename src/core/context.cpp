#include "TContext.h"
#include "TRenderSystem11.h"
#include "TRenderSystem9.h"

TContext::TContext()
{}
bool TContext::Initialize(HWND hWnd)
{
#if 0
	mRenderSys = new TRenderSystem9;
#else
	mRenderSys = new TRenderSystem11;
#endif
	if (FAILED(mRenderSys->Initialize(hWnd))) {
		mRenderSys->CleanUp();
		return false;
	}

	mSceneMng = mRenderSys->GetSceneManager().get();
	mRenderableFac = new RenderableFactory(mRenderSys);
	return true;
}

TContext::~TContext()
{}
void TContext::Dispose()
{

}