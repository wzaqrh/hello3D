#include "test/test_case.h"
#include "test/app.h"
#include "core/base/transform.h"
#include "core/renderable/assimp_model.h"
#include "core/renderable/sprite.h"
#include "core/renderable/cube.h"
#include "core/scene/camera.h"

using namespace mir;
using namespace mir::rend;

class TestCamera : public App
{
protected:
	void OnRender() override;
	cppcoro::shared_task<void> OnPostInitDevice() override;
	void OnInitLight() override {}
	void OnInitCamera() override {}
private:
	AssimpModelPtr mModel;
	SpritePtr mSprite;
};
/*mCaseIndex
0-5: ͸�����
0: �������			�۲쵽���ӷɻ�
1������ƶ�			�۲쵽�ɻ�ƫ����
2������ƶ�			�۲쵽�ɻ���С(Զ��ɻ�)
3���������			�۲쵽�ɻ��뱳����ƽ
4���������(1,-1,1)  �۲쵽��Ļ��ת
5�������z��180��ת��	�۲쵽��Ļ180��ת

6-11: �������
6->0
7->1
8->2
9->3
10->4
11->5
*/

cppcoro::shared_task<void> TestCamera::OnPostInitDevice()
{
	constexpr int CaseCountMod = 6;
	int caseIndex = mCaseIndex % CaseCountMod;
	int cameraType = mCaseIndex / CaseCountMod;
	SetPPU(1);

	if (1)
	{
		mModel = co_await mContext->RenderableFac()->CreateAssimpModel(MAT_MODEL);
		mTransform = co_await test1::res::model().Init("spaceship", mModel);
		if (cameraType == 0)
			mTransform->SetScale(mTransform->GetScale() * 2);
	}

	mScneMng->AddDirectLight()->SetDirection(Eigen::Vector3f(0, -1, 1));

	mControlCamera = false;
	CameraPtr camera = mScneMng->AddCameraByType((CameraType)cameraType, Eigen::Vector3f(0, 0, -10));
	camera->SetSkyBox(co_await mRendFac->CreateSkybox(test1::res::Sky()));
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
}

void TestCamera::OnRender()
{
	if (mContext->RenderPipe()->BeginFrame()) {
		if (mModel) mModel->Update(mTimer->mDeltaTime);

		RenderOperationQueue opQueue;
		if (mModel) mModel->GenRenderOperation(opQueue);
		if (mSprite) mSprite->GenRenderOperation(opQueue);

		mContext->RenderPipe()->Render(opQueue, *mContext->SceneMng());
		mContext->RenderPipe()->EndFrame();
	}
}

auto reg = AppRegister<TestCamera>("test_camera");