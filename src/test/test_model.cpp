#include "test/test_case.h"
#include "test/app.h"
#include "core/renderable/assimp_model.h"
#include "core/base/transform.h"

using namespace mir;

class TestModel : public App
{
protected:
	void OnRender() override;
	void OnPostInitDevice() override;
	void OnInitLight() override;
private:
	int mDrawFlag = 0;
	AssimpModelPtr mModel = nullptr;
};

void TestModel::OnInitLight()
{
	if (mCaseIndex == 0 || mCaseIndex == 1) {
		auto pt_light = mContext->SceneMng()->AddPointLight();
		pt_light->SetPosition(Eigen::Vector3f(0, 5, -5));

		auto dir_light = mContext->SceneMng()->AddDirectLight();
		dir_light->SetDirection(Eigen::Vector3f(25, 0, 5));
	}
	else {
		mContext->SceneMng()->RemoveAllLights();
		auto dir_light = mContext->SceneMng()->AddDirectLight();
		dir_light->SetDirection(Eigen::Vector3f(0, 0, 1));
	}
}

void TestModel::OnPostInitDevice()
{
	auto sceneMng = mContext->SceneMng();
	auto rendFac = mContext->RenderableFac();
	auto resMng = mContext->ResourceMng();
	auto winCenter = resMng->WinSize() / 2;

	switch (mCaseIndex) {
	case 0:
	case 1: {
		sceneMng->GetDefCamera()->SetSkyBox(rendFac->CreateSkybox("model/uffizi_cross.dds"));

		mModel = rendFac->CreateAssimpModel(!(mCaseIndex&1) ? E_MAT_MODEL : E_MAT_MODEL_PBR);
		mModel->LoadModel("model/Male03/Male02.FBX", R"({"ext":"png","dir":"model/Male03/"})"); mMoveDefScale = 0.07;

		mTransform = mModel->GetTransform();
		mTransform->SetScale(Eigen::Vector3f(mMoveDefScale, mMoveDefScale, mMoveDefScale));
		mTransform->SetPosition(Eigen::Vector3f(0, -5, 0));
	}break;
	case 2:
	case 3:
	case 4:
	case 5:{
		sceneMng->RemoveAllCameras();
		sceneMng->AddPerspectiveCamera(Eigen::Vector3f(0, 0, -1500), 3000, 30);

		mModel = rendFac->CreateAssimpModel(!(mCaseIndex&1) ? E_MAT_MODEL : E_MAT_MODEL_PBR);
		mTransform = mModel->GetTransform();
		if (mCaseIndex < 4) {
			mModel->LoadModel("model/Spaceship/Spaceship.fbx", R"({"dir":"model/Spaceship/"})"); 
			mMoveDefScale = 1;
			mModel->GetTransform()->SetPosition(Eigen::Vector3f(0, -100, 0));
		}
		else {
			mModel->LoadModel("model/rock/rock.obj", R"({"dir":"model/rock/"})");
			mMoveDefScale = 100;
			mModel->GetTransform()->SetPosition(Eigen::Vector3f(100, 100, 10));
		}
	}break;
	default:
		break;
	}
	mModel->GetTransform()->SetScale(Eigen::Vector3f(mMoveDefScale, mMoveDefScale, mMoveDefScale));
	mTransform = mModel->GetTransform();
	mModel->PlayAnim(0);
}

void TestModel::OnRender()
{
	mModel->Update(mTimer->mDeltaTime);
	mContext->RenderPipe()->Draw(*mModel, *mContext->SceneMng());
}

auto reg = AppRegister<TestModel>("test_model");
