#include "test/test_case.h"
#include "test/app.h"
#include "core/rendersys/framebuffer.h"
#include "core/renderable/assimp_model.h"
#include "core/renderable/sprite.h"
#include "core/renderable/cube.h"
#include "core/scene/transform.h"
#include "core/scene/camera.h"

using namespace mir;
using namespace mir::rend;

class TestCameraOutput : public App
{
protected:
	CoTask<bool> OnInitScene() override;
	void OnInitLight() override {}
	void OnInitCamera() override {}
};
/*mCaseIndex
0：透视相机 点光 带天空盒 观察到左下角lenna，右上角天空+飞机
1：正交相机 方向光 观察到飞机
*/

#define SCALE_BASE 0.01
CoTask<bool> TestCameraOutput::OnInitScene()
{
	constexpr unsigned cameraMask2 = 0x01;
	CameraPtr camera2;
	{
		if (mCaseIndex == 1) 
		{
			camera2 = mScneMng->CreateCameraNode(kCameraOthogonal);
			camera2->SetCullingMask(cameraMask2);
			camera2->SetDepth(1);

			auto light2 = mScneMng->CreateLightNode<DirectLight>();
			light2->SetCameraMask(cameraMask2);
			light2->SetDirection(test1::vec::DirLight());
		}
		else 
		{
			camera2 = mScneMng->CreateCameraNode(kCameraPerspective);
			//camera2->GetTransform()->SetScale(Eigen::Vector3f(2, 2, 1));
			camera2->SetCullingMask(cameraMask2);
			camera2->SetDepth(1);
			camera2->SetSkyBox(CoAwait mRendFac->CreateSkyboxT(test1::res::Sky()));

			auto light2 = mScneMng->CreateLightNode<PointLight>();
			light2->SetCameraMask(cameraMask2);
			light2->SetPosition(mWinCenter + Eigen::Vector3f(10, 10, -10));
			light2->SetAttenuation(0.0001);
		}

		auto mModel2 = mScneMng->AddRendAsNode(CoAwait mRendFac->CreateAssimpModelT(MAT_MODEL));
		mTransform = CoAwait test1::res::model().Init("spaceship", mModel2);
		mModel2->SetCameraMask(cameraMask2);
		mModel2->PlayAnim(0);

		if (camera2->GetType() == kCameraOthogonal)
			mTransform->SetScale(mTransform->GetLossyScale() * 50);
	}

	if (mCaseIndex == 1)
	{
		mScneMng->AddRendAsNode(CoAwait test1::res::cube::far_plane::Create(mRendFac, mWinCenter));
		mScneMng->AddRendAsNode(CoAwait test1::res::cube::near_plane::Create(mRendFac, mWinCenter));
	}

	constexpr unsigned cameraMask1 = 0x02;
	if (mCaseIndex == 0)
	{
		auto camera1 = mScneMng->CreateCameraNode(kCameraOthogonal);
		camera1->SetDepth(2);
		camera1->SetCullingMask(cameraMask1);

		auto light1 = mScneMng->CreateLightNode<PointLight>();
		light1->SetCameraMask(cameraMask1);

		auto mSprite1 = mScneMng->AddRendAsNode(CoAwait mRendFac->CreateSpriteT("model/lenna.dds"));
		mSprite1->SetCameraMask(cameraMask1);
		mSprite1->GetTransform()->SetPosition(-mHalfSize);

		auto mSpriteCam1 = CoAwait mRendFac->CreateSpriteT();
		mSpriteCam1->SetCameraMask(cameraMask1);
		//auto fetchTexture = mResMng->CreateTextureByFile(__LaunchAsync__, "model/theyKilledKenny.dds");
		auto fetchTexture = camera2->SetOutput(0.5)->GetAttachColorTexture(0);
		DEBUG_SET_PRIV_DATA(fetchTexture, "camera2 output");
		mSpriteCam1->SetTexture(fetchTexture);
		mSpriteCam1->SetPosition(mWinCenter);
		mSpriteCam1->SetSize(mHalfSize.cast<float>());
	}
	CoReturn true;
}

auto reg = AppRegister<TestCameraOutput>("test_camera_output");