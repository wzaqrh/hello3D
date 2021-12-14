#include "test/test_case.h"
#include "test/app.h"
#include "core/base/transform.h"
#include "core/renderable/assimp_model.h"
#include "core/renderable/sprite.h"
#include "core/renderable/cube.h"
#include "core/scene/camera.h"

using namespace mir;

class TestCameraOutput : public App
{
protected:
	void OnRender() override;
	void OnPostInitDevice() override;
	void OnInitLight() override {}
	void OnInitCamera() override {}
private:
	AssimpModelPtr mModel2;
	SpritePtr mSprite1, mSpriteCam1;
	CubePtr mCube0, mCube1;
};
/*mCaseIndex
0：透视相机 点光 带天空盒 观察到左下角lenna，右上角天空+飞机
1：正交相机 方向光 观察到飞机
*/

#define SCALE_BASE 0.01
void TestCameraOutput::OnPostInitDevice()
{
	constexpr unsigned cameraMask2 = 0x01;
	CameraPtr camera2;
	{
		if (mCaseIndex == 1) 
		{
			camera2 = mScneMng->AddOthogonalCamera(test1::cam::Eye(mWinCenter));
			camera2->SetCameraMask(cameraMask2);
			camera2->SetDepth(1);

			auto light2 = mScneMng->AddDirectLight();
			light2->SetCameraMask(cameraMask2);
			light2->SetDirection(test1::vec::DirLight());
		}
		else 
		{
			camera2 = mScneMng->AddPerspectiveCamera(test1::cam::Eye(mWinCenter));
			//camera2->GetTransform()->SetScale(Eigen::Vector3f(2, 2, 1));
			camera2->SetCameraMask(cameraMask2);
			camera2->SetDepth(1);
			camera2->SetSkyBox(mRendFac->CreateSkybox(test1::res::Sky()));

			auto light2 = mScneMng->AddPointLight();
			light2->SetCameraMask(cameraMask2);
			light2->SetPosition(mWinCenter + Eigen::Vector3f(10, 10, -10));
			light2->SetAttenuation(0.0001);
		}

		mModel2 = mRendFac->CreateAssimpModel(MAT_MODEL);
		mModel2->LoadModel(test1::res::model_sship::Path(), test1::res::model_sship::Rd());
		mModel2->SetCameraMask(cameraMask2);
		mTransform = mModel2->GetTransform();
		mTransform->SetScale(test1::res::model_sship::Scale());
		mTransform->SetPosition(mWinCenter + Eigen::Vector3f::Zero());
		mModel2->PlayAnim(0);

		if (camera2->GetType() == kCameraOthogonal)
			mTransform->SetScale(mTransform->GetScale() * 50);
	}

	if (mCaseIndex == 1)
	{
		mCube0 = test1::res::cube::far_plane::Create(mRendFac, mWinCenter);
		mCube1 = test1::res::cube::near_plane::Create(mRendFac, mWinCenter);
	}

	constexpr unsigned cameraMask1 = 0x02;
	if (mCaseIndex == 0)
	{
		auto camera1 = mScneMng->AddOthogonalCamera(test1::cam::Eye(mWinCenter));
		camera1->SetDepth(2);
		camera1->SetCameraMask(cameraMask1);

		auto light1 = mScneMng->AddPointLight();
		light1->SetCameraMask(cameraMask1);

		mSprite1 = mRendFac->CreateSprite("model/lenna.dds");
		mSprite1->SetCameraMask(cameraMask1);
		mSprite1->GetTransform()->SetPosition(-mHalfSize);

		mSpriteCam1 = mRendFac->CreateSprite();
		mSpriteCam1->SetCameraMask(cameraMask1);
		//auto fetchTexture = mResMng->CreateTextureByFile(__LaunchAsync__, "model/theyKilledKenny.dds");
		auto fetchTexture = camera2->SetOutput(0.5)->GetAttachColorTexture(0);
		DEBUG_SET_PRIV_DATA(fetchTexture, "camera2 output");
		mSpriteCam1->SetTexture(fetchTexture);
		mSpriteCam1->SetPosition(mWinCenter);
		mSpriteCam1->SetSize(mHalfSize.cast<float>());
	}
}

void TestCameraOutput::OnRender()
{
	if (mContext->RenderPipe()->BeginFrame()) {
		if (mModel2) mModel2->Update(mTimer->mDeltaTime);

		RenderOperationQueue opQueue;
		if (mModel2) mModel2->GenRenderOperation(opQueue);
		if (mSprite1) mSprite1->GenRenderOperation(opQueue);
		if (mSpriteCam1) mSpriteCam1->GenRenderOperation(opQueue);
		if (mCube0) mCube0->GenRenderOperation(opQueue);
		if (mCube1) mCube1->GenRenderOperation(opQueue);

		mContext->RenderPipe()->Render(opQueue, *mContext->SceneMng());
		mContext->RenderPipe()->EndFrame();
	}
}

auto reg = AppRegister<TestCameraOutput>("test_camera_output");