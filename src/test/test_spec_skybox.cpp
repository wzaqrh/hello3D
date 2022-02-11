#include "test/test_case.h"
#include "test/app.h"

using namespace mir;
using namespace mir::rend;

class TestSpecSkybox : public App
{
protected:
	CoTask<bool> OnInitScene() override;
	void OnInitCamera() override {}
};
/*mCaseIndex

*/
CoTask<bool> TestSpecSkybox::OnInitScene()
{
	switch (mCaseIndex) {
	case 0: {
		CameraPtr camera = mScneMng->CreateAddCameraNode(kCameraPerspective, test1::cam::Eye(mWinCenter));
		MaterialLoadParamBuilder matname = MAT_SKYBOX;
		matname["CubeMapIsRightHandness"] = TRUE;
		if (mCaseSecondIndex == 0)
			camera->SetSkyBox(CoAwait mRendFac->CreateSkyboxT(test1::res::sky::footprint_court::Diffuse(), matname));
		else
			camera->SetSkyBox(CoAwait mRendFac->CreateSkyboxT(test1::res::sky::footprint_court::Specular(), matname));
	}break;
	case 1: {
		CameraPtr camera = mScneMng->CreateAddCameraNode(kCameraPerspective, test1::cam::Eye(mWinCenter));
		camera->SetSkyBox(CoAwait mRendFac->CreateSkyboxT(test1::res::Sky(mCaseSecondIndex % 3)));//bc1a mipmap cube
	}break;
	case 2: {
		CameraPtr camera = mScneMng->CreateAddCameraNode(kCameraOthogonal, test1::cam::Eye(mWinCenter));
		camera->SetSkyBox(CoAwait mRendFac->CreateSkyboxT(test1::res::Sky()));//bc1a mipmap cube
	}break;
	case 3: {
		CameraPtr camera = mScneMng->CreateAddCameraNode(kCameraPerspective, test1::cam::Eye(mWinCenter));
		camera->SetSkyBox(CoAwait mRendFac->CreateSkyboxT(test1::res::Sky(), MAT_SKYBOX "-Deprecate"));//bc1a mipmap cube
	}break;
	default:
		break;
	}
	CoReturn true;
}

auto reg = AppRegister<TestSpecSkybox>("test_spec_skybox");