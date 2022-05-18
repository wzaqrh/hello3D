#include "test/test_case.h"
#include "test/app.h"

using namespace mir;
using namespace mir::rend;

class TestDefferedPath : public App
{
protected:
	CoTask<bool> OnInitScene() override;
	void OnInitLight() override {}
	void OnInitCamera() override {}
};
/*mCaseIndex
0�ӳ�,1����͸����� ƽ�й�+��� �Աȹ۲�ɻ�
2�ӳ�,3����: ͸����� ����ƽ�й�  �Աȹ۲�ɻ�
*/

CoTask<bool> TestDefferedPath::OnInitScene()
{
	CameraPtr camera = mScneMng->CreateCameraNode(kCameraPerspective);
	camera->SetSkyBox(CoAwait mRendFac->CreateSkyboxT(test1::res::Sky()));
	camera->SetRenderingPath(RenderingPath((mCaseIndex+1)&1));

	switch (mCaseIndex) {
	case 0:
	case 1:{
		mScneMng->CreateLightNode<DirectLight>();
		mScneMng->CreateLightNode<PointLight>()->SetPosition(test1::vec::PosLight());

		auto mModel = CoAwait mRendFac->CreateAssimpModelT(MAT_MODEL);
		mScneMng->AddRendAsNode(mModel);
		mTransform = CoAwait test1::res::model().Init("spaceship", mModel);
	}break;
	case 2:
	case 3:{
		mScneMng->CreateLightNode<DirectLight>();

		auto mModel = CoAwait mRendFac->CreateAssimpModelT(MAT_MODEL);
		mScneMng->AddRendAsNode(mModel);
		mTransform = CoAwait test1::res::model().Init("spaceship", mModel);
	}break;
	default:
		break;
	}
	CoReturn true;
}

auto reg = AppRegister<TestDefferedPath>("test_deffered_path");
