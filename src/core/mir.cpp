#include "core/mir.h"
#include "core/rendersys/d3d11/render_system11.h"
#include "core/rendersys/d3d9/render_system9.h"

namespace mir {

Mir::Mir()
{}
Mir::~Mir()
{}
bool Mir::Initialize(HWND hWnd) {
#if 0
	mRenderSys = std::static_pointer_cast<IRenderSystem>(std::make_shared<TRenderSystem9>());
#else
	mRenderSys = std::static_pointer_cast<IRenderSystem>(std::make_shared<RenderSystem11>());
#endif
	if (FAILED(mRenderSys->Initialize(hWnd))) {
		mRenderSys->CleanUp();
		return false;
	}

	mSceneMng = mRenderSys->GetSceneManager();
	mRenderableFac = std::make_shared<RenderableFactory>(*mRenderSys);
	return true;
}
void Mir::Dispose() {
	mRenderableFac = nullptr;
	mMaterialFac = nullptr;
	mSceneMng = nullptr;
	mRenderSys = nullptr;
}

}