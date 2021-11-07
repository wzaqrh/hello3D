#include "core/mir.h"
#include "core/rendersys/d3d11/render_system11.h"
#include "core/rendersys/d3d9/render_system9.h"
#include "core/rendersys/material_factory.h"

namespace mir {

Mir::Mir()
{}
Mir::~Mir()
{}
bool Mir::Initialize(HWND hWnd) {
#if 0
	mRenderSys = std::static_pointer_cast<RenderSystem>(std::make_shared<TRenderSystem9>());
#else
	mRenderSys = std::static_pointer_cast<RenderSystem>(std::make_shared<RenderSystem11>());
#endif
	if (FAILED(mRenderSys->Initialize(hWnd))) {
		mRenderSys->CleanUp();
		return false;
	}

	mMaterialFac = std::make_shared<MaterialFactory>(*mRenderSys);
	auto rs = std::static_pointer_cast<RenderSystem11>(mRenderSys);
	mSceneMng = MakePtr<SceneManager>(*mRenderSys, 
		*mMaterialFac, 
		XMINT2(rs->mScreenWidth, rs->mScreenHeight), 
		rs->mPostProcessRT, 
		Camera::CreatePerspective(rs->mScreenWidth, rs->mScreenHeight));
	rs->mSceneManager = mSceneMng;
	mRenderableFac = std::make_shared<RenderableFactory>(*mRenderSys, *mMaterialFac);
	return true;
}
void Mir::Dispose() {
	mRenderableFac = nullptr;
	mMaterialFac = nullptr;
	mSceneMng = nullptr;
	mRenderSys = nullptr;
}

}