#include "test/test_case.h"
#include "test/app.h"
#include "core/base/transform.h"
#include "core/renderable/assimp_model.h"
#include "core/renderable/sprite.h"
#include "core/scene/camera.h"

using namespace mir;

class TestCamera : public App
{
protected:
	void OnRender() override;
	void OnPostInitDevice() override;
private:
	AssimpModelPtr mModel2;
	SpritePtr mSprite1, mSpriteCam1;
};

#define SCALE_BASE 0.01
void TestCamera::OnPostInitDevice()
{
	auto sceneMng = mContext->SceneMng();
	auto rendFac = mContext->RenderableFac();
	auto resMng = mContext->ResourceMng();

	sceneMng->RemoveAllCameras();
	sceneMng->RemoveAllLights();

	constexpr unsigned cameraMask2 = 0x01;
	auto camera2 = sceneMng->AddPerspectiveCamera(Eigen::Vector3f(0, 0, -30), 300, 45);
	auto light2 = sceneMng->AddPointLight();
	{
		camera2->SetDepth(1);
		camera2->SetCameraMask(cameraMask2);
		camera2->SetSkyBox(rendFac->CreateSkybox("model/uffizi_cross.dds"));

		light2->SetCameraMask(cameraMask2);
		light2->SetPosition(Eigen::Vector3f(10, 10, -10));
		light2->SetAttenuation(0.001);

		mModel2 = mContext->RenderableFac()->CreateAssimpModel(nullptr, E_MAT_MODEL);
		mModel2->LoadModel("model/Spaceship/Spaceship.fbx", R"({"dir":"model/Spaceship/"})");
		mModel2->SetCameraMask(cameraMask2);
		mModel2->GetTransform()->SetScale(Eigen::Vector3f(SCALE_BASE, SCALE_BASE, SCALE_BASE));
		mModel2->PlayAnim(0);
	}

	constexpr unsigned cameraMask1 = 0x02;
	{
		auto camera1 = sceneMng->AddOthogonalCamera(Eigen::Vector3f(0,0,-10), 100);
		camera1->SetDepth(2);
		camera1->SetCameraMask(cameraMask1);
		auto cam1Size = camera1->GetSize() / 2;

		auto light1 = sceneMng->AddPointLight();
		light1->SetCameraMask(cameraMask1);

		mSprite1 = mContext->RenderableFac()->CreateSprite("model/lenna.dds");
		mSprite1->SetCameraMask(cameraMask1);

		mSpriteCam1 = mContext->RenderableFac()->CreateSprite();
		mSpriteCam1->SetCameraMask(cameraMask1);
		//auto fetchTexture = resMng->CreateTextureByFile(__LaunchAsync__, "model/theyKilledKenny.dds");
		auto fetchTexture = camera2->FetchOutput()->GetAttachColorTexture(0);
		fetchTexture->SetPrivInfo("camera2 output");
		mSpriteCam1->SetTexture(fetchTexture);
		mSpriteCam1->SetPosition(Eigen::Vector3f(cam1Size.x(), cam1Size.y(), 0));
		mSpriteCam1->SetSize(cam1Size.cast<float>());
	}
}

void TestCamera::OnRender()
{
	if (mContext->RenderPipe()->BeginFrame()) {
		mModel2->Update(mTimer->mDeltaTime);

		RenderOperationQueue opQueue;
		mModel2->GenRenderOperation(opQueue);
		mSprite1->GenRenderOperation(opQueue);
		mSpriteCam1->GenRenderOperation(opQueue);

		mContext->RenderPipe()->Render(opQueue, *mContext->SceneMng());
		mContext->RenderPipe()->EndFrame();
	}
}

auto reg = AppRegister<TestCamera>("test_camera");