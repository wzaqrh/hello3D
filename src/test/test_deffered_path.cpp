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
0延迟,1正向：透视相机 平行光+电光 对比观察飞机
2延迟,3正向: 透视相机 正向平行光  对比观察飞机
*/

CoTask<bool> TestDefferedPath::OnInitScene()
{
	CameraPtr camera = mScneMng->CreateAddCameraNode(kCameraPerspective, test1::cam::Eye(mWinCenter));
	camera->SetSkyBox(CoAwait mRendFac->CreateSkybox(test1::res::Sky()));
	camera->SetRenderingPath(RenderingPath((mCaseIndex+1)&1));

	switch (mCaseIndex) {
	case 0:
	case 1:{
		mScneMng->CreateAddLightNode<DirectLight>();
		mScneMng->CreateAddLightNode<PointLight>()->SetPosition(test1::vec::PosLight());

		auto mModel = CoAwait mRendFac->CreateAssimpModel(MAT_MODEL);
		mScneMng->AddRendNode(mModel);
		mTransform = CoAwait test1::res::model().Init("spaceship", mModel);
	}break;
	case 2:
	case 3:{
		mScneMng->CreateAddLightNode<DirectLight>();

		auto mModel = CoAwait mRendFac->CreateAssimpModel(MAT_MODEL);
		mScneMng->AddRendNode(mModel);
		mTransform = CoAwait test1::res::model().Init("spaceship", mModel);
	}break;
	default:
		break;
	}
	CoReturn true;
}

auto reg = AppRegister<TestDefferedPath>("test_deffered_path");
