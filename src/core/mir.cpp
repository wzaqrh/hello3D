#include "core/mir.h"
#include "core/rendersys/d3d11/render_system11.h"
#include "core/rendersys/d3d9/render_system9.h"

namespace mir {

Mir::Mir()
{}
bool Mir::Initialize(HWND hWnd)
{
#if 0
	mRenderSys = new TRenderSystem9;
#else
	mRenderSys = new RenderSystem11;
#endif
	if (FAILED(mRenderSys->Initialize(hWnd))) {
		mRenderSys->CleanUp();
		return false;
	}

	mSceneMng = mRenderSys->GetSceneManager().get();
	mRenderableFac = new RenderableFactory(mRenderSys);
	return true;
}

Mir::~Mir()
{}
void Mir::Dispose()
{}

}