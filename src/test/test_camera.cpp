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

#define SCALE_BASE 0.01
void TestCamera::OnPostInitDevice()
{
	auto sceneMng = mContext->SceneMng();
	auto rendFac = mContext->RenderableFac();
	auto resMng = mContext->ResourceMng();
	auto screenCenter = resMng->WinSize() / 2;

	sceneMng->RemoveAllCameras();
	sceneMng->RemoveAllLights();

	if (mCaseIndex >= 4) {
		CameraPtr camera = sceneMng->AddOthogonalCamera(Eigen::Vector3f(0, 0, -30), 300);
		camera->GetTransform()->SetScale(Eigen::Vector3f(0.5, 0.5, 1));
		//camera->GetTransform()->SetPosition(Eigen::Vector3f(-screenCenter.x(), 0, 0));

		auto light = sceneMng->AddDirectLight();
		light->SetDirection(Eigen::Vector3f(0, 0, 10));

		mSprite = mContext->RenderableFac()->CreateSprite("model/smile.png");
		mSprite->SetPosition(Eigen::Vector3f(screenCenter.x() - 240, screenCenter.y() - 240, 0));
	}
	else {
		CameraPtr camera = sceneMng->AddPerspectiveCamera(Eigen::Vector3f(0, 0, -30), 300, 45);
		switch (mCaseIndex) {
		case 0: {
			camera->GetTransform()->SetPosition(Eigen::Vector3f(5, -5, -30));
		}break;
		case 1: {
			camera->GetTransform()->SetScale(Eigen::Vector3f(0.5, 2, 1));
		}break;
		case 2: {
			camera->GetTransform()->SetPosition(Eigen::Vector3f(0, 0, -30));
		}break;
		default:
			break;
		}
		camera->SetSkyBox(rendFac->CreateSkybox("model/uffizi_cross.dds"));

		auto light = sceneMng->AddPointLight();
		light->SetPosition(Eigen::Vector3f(10, 10, -10));
		light->SetAttenuation(0.001);

		mModel = mContext->RenderableFac()->CreateAssimpModel(E_MAT_MODEL);
		mModel->LoadModel("model/Spaceship/Spaceship.fbx", R"({"dir":"model/Spaceship/"})");
		float scale = SCALE_BASE;
		mModel->GetTransform()->SetScale(Eigen::Vector3f(scale, scale, scale));
		mModel->PlayAnim(0);
		mTransform = mModel->GetTransform();
		mTransform->SetPosition(Eigen::Vector3f(0, 0, 0));
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