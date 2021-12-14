#include "test/test_case.h"
#include "test/app.h"
#include "core/base/transform.h"
#include "core/renderable/assimp_model.h"
#include "core/renderable/sprite.h"
#include "core/renderable/cube.h"
#include "core/scene/camera.h"

using namespace mir;

class TestCamera : public App
{
protected:
	void OnRender() override;
	void OnPostInitDevice() override;
private:
	AssimpModelPtr mModel;
	SpritePtr mSprite;
};
/*mCaseIndex
0：观察到飞机偏左上
1：观察到飞机与背景扁平
2：观察到飞机变小(远离飞机)
*/

#define SCALE_BASE 0.01
void TestCamera::OnPostInitDevice()
{
	mScneMng->RemoveAllCameras();
	mScneMng->RemoveAllLights();

	if (mCaseIndex >= 4) {
		CameraPtr camera = mScneMng->AddOthogonalCamera(test1::cam::Eye(mWinCenter));
		camera->GetTransform()->SetScale(Eigen::Vector3f(0.5, 0.5, 1));
		//camera->GetTransform()->SetPosition(Eigen::Vector3f(-screenCenter.x(), 0, 0));

		auto light = mScneMng->AddDirectLight();
		light->SetDirection(test1::vec::DirLight());

		mSprite = mContext->RenderableFac()->CreateSprite("model/smile.png");
		mSprite->SetPosition(mWinCenter + Eigen::Vector3f(-mHalfSize.x()/2, -mHalfSize.y()/2, 0));
	}
	else {
		CameraPtr camera = mScneMng->AddPerspectiveCamera(test1::cam::Eye(mWinCenter));
		switch (mCaseIndex) {
		case 0: {
			camera->GetTransform()->SetPosition(mWinCenter + Eigen::Vector3f(5, -5, -30));
		}break;
		case 1: {
			camera->GetTransform()->SetScale(mWinCenter + Eigen::Vector3f(0.5, 2, 1));
		}break;
		case 2: {
			camera->GetTransform()->SetPosition(mWinCenter + Eigen::Vector3f(0, 0, -100));
		}break;
		default:
			break;
		}
		camera->SetSkyBox(mRendFac->CreateSkybox(test1::res::Sky()));

		auto light = mScneMng->AddPointLight();
		light->SetPosition(Eigen::Vector3f(10, 10, -10));
		light->SetAttenuation(0.001);

		mModel = mContext->RenderableFac()->CreateAssimpModel(MAT_MODEL);
		mModel->LoadModel(test1::res::model_sship::Path(), test1::res::model_sship::Rd());
		mTransform = mModel->GetTransform();
		mTransform->SetScale(test1::res::model_sship::Scale());
		mTransform->SetPosition(mWinCenter + Eigen::Vector3f::Zero());
		mModel->PlayAnim(0);
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