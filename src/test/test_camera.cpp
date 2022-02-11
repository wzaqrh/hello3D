#include "test/test_case.h"
#include "test/app.h"
#include "core/renderable/sprite.h"
#include "core/renderable/cube.h"

using namespace mir;
using namespace mir::rend;

class TestCamera : public App
{
protected:
	CoTask<bool> OnPostInitDevice() override;
	void OnInitLight() override {}
	void OnInitCamera() override {}
};
/*mCaseIndex
0-5: 透视相机
0: 相机朝下			观察到俯视飞机
1：相机移动			观察到飞机偏左上
2：相机移动			观察到飞机变小(远离飞机)
3：相机缩放			观察到飞机与背景扁平
4：相机缩放(1,-1,1)  观察到屏幕翻转
5：相机绕z轴180度转动	观察到屏幕180旋转

6-11: 正交相机
6->0
7->1
8->2
9->3
10->4
11->5
*/

CoTask<bool> TestCamera::OnPostInitDevice()
{
	constexpr int CaseCountMod = 6;
	int caseIndex = mCaseIndex % CaseCountMod;
	int cameraType = mCaseIndex / CaseCountMod;
	SetPPU(1);

	if (1)
	{
		auto mModel = mScneMng->AddRendNode(CoAwait mContext->RenderableFac()->CreateAssimpModel(MAT_MODEL));
		mTransform = CoAwait test1::res::model().Init("spaceship", mModel);
		if (cameraType == 0)
			mTransform->SetScale(mTransform->GetLossyScale() * 2);
	}

	mScneMng->CreateAddLightNode<DirectLight>()->SetDirection(Eigen::Vector3f(0, -1, 1));

	mControlCamera = false;
	CameraPtr camera = mScneMng->CreateAddCameraNode((CameraType)cameraType, Eigen::Vector3f(0, 0, -10));
	camera->SetSkyBox(CoAwait mRendFac->CreateSkybox(test1::res::Sky()));
	switch (caseIndex) {
	case 0: {
		camera->SetForward(mir::math::vec::Down());
		camera->GetTransform()->SetPosition(Eigen::Vector3f(0, 10, 0));

		mTransform->SetPosition(mTransform->GetPosition() + Eigen::Vector3f(0, -30, 0));
	}break;
	case 1: {
		camera->GetTransform()->SetPosition(Eigen::Vector3f(2.5, -2.5, -30));
	}break;
	case 2: {
		camera->GetTransform()->SetPosition(Eigen::Vector3f(0, 0, -100));
	}break;
	case 3: {
		camera->GetTransform()->SetScale(Eigen::Vector3f(0.5, 2, 1));
	}break;
	case 4: {
		camera->GetTransform()->SetScale(Eigen::Vector3f(1, -1, 1));
	}break;
	case 5: {
		camera->GetTransform()->Rotate(Eigen::Vector3f(0, 0, math::ToRadian(180)));
	}break;
	default:
		break;
	}
	CoReturn true;
}

auto reg = AppRegister<TestCamera>("test_camera");