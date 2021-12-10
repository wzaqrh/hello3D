#include "test/test_case.h"
#include "test/app.h"
#include "core/scene/scene_manager.h"
#include "core/renderable/assimp_model.h"
#include "core/base/transform.h"

using namespace mir;

class TestSpecSkybox : public App
{
protected:
	void OnRender() override;
	void OnPostInitDevice() override;
	void OnInitCamera() override {}
};
/*mCaseIndex
0：透视相机 观察到天空盒	 
1：正交相机 观察到天空盒	 
*/

void TestSpecSkybox::OnPostInitDevice()
{
	if (mCaseIndex == 0) {
		CameraPtr camera = mScneMng->AddPerspectiveCamera(mWinCenter + test1::cam::Eye(mWinCenter), 
			test1::cam::NearFarFov());
		camera->SetSkyBox(mRendFac->CreateSkybox(test1::res::Sky()));//bc1a mipmap cube
	}
	else {
		CameraPtr camera = mScneMng->AddOthogonalCamera(mWinCenter + test1::cam::Eye(mWinCenter),
			test1::cam::NearFarFov());
		camera->SetSkyBox(mRendFac->CreateSkybox(test1::res::Sky()));//bc1a mipmap cube
	}
}

void TestSpecSkybox::OnRender()
{
	if (mContext->RenderPipe()->BeginFrame()) {
		RenderOperationQueue opQue;
		mContext->RenderPipe()->Render(opQue, *mContext->SceneMng());
		mContext->RenderPipe()->EndFrame();
	}
}

auto reg = AppRegister<TestSpecSkybox>("test_spec_skybox");