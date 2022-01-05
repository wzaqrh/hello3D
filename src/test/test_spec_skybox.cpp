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

*/
void TestSpecSkybox::OnPostInitDevice()
{
	switch (mCaseIndex) {
	case 0: {
		CameraPtr camera = mScneMng->AddPerspectiveCamera(test1::cam::Eye(mWinCenter));
		MaterialLoadParamBuilder matname = MAT_SKYBOX;
		matname["CubeMapIsRightHandness"] = TRUE;
		camera->SetSkyBox(mRendFac->CreateSkybox(test1::res::Sky(2), matname));//bc1a mipmap cube
	}break;
	case 1: {
		CameraPtr camera = mScneMng->AddPerspectiveCamera(test1::cam::Eye(mWinCenter));
		camera->SetSkyBox(mRendFac->CreateSkybox(test1::res::Sky(mCaseSecondIndex % 3)));//bc1a mipmap cube
	}break;
	case 2: {
		CameraPtr camera = mScneMng->AddOthogonalCamera(test1::cam::Eye(mWinCenter));
		camera->SetSkyBox(mRendFac->CreateSkybox(test1::res::Sky()));//bc1a mipmap cube
	}break;
	case 3: {
		CameraPtr camera = mScneMng->AddPerspectiveCamera(test1::cam::Eye(mWinCenter));
		camera->SetSkyBox(mRendFac->CreateSkybox(test1::res::Sky(), MaterialLoadParam{MAT_SKYBOX, "Skybox/Deprecate"}));//bc1a mipmap cube
	}break;
	default:
		break;
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