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
	void OnInitLight() override {}
	void OnInitCamera() override {}
private:
	AssimpModelPtr mModel;
};
/*mCaseIndex
0,1：透视相机 观察到模型：传奇战士 + 天空
2,3: 透视相机 观察到模型：飞机
4,5：透视相机 观察到模型：石头（在右上角）
*/

void TestModel::OnPostInitDevice()
{
	auto sceneMng = mContext->SceneMng();
	auto rendFac = mContext->RenderableFac();
	auto resMng = mContext->ResourceMng();
	auto halfSize = mContext->ResourceMng()->WinSize() / 2;
	auto winCenter = Eigen::Vector3f(halfSize.x(), halfSize.y(), 0);

	if (mCaseIndex == 0 || mCaseIndex == 1) 
	{
		auto pt_light = sceneMng->AddPointLight();
		pt_light->SetPosition(winCenter + Eigen::Vector3f(0, 5, -5));

		auto dir_light = sceneMng->AddDirectLight();
		dir_light->SetDirection(Eigen::Vector3f(25, 0, 5));
	}
	else 
	{
		auto dir_light = sceneMng->AddDirectLight();
		dir_light->SetDirection(Eigen::Vector3f(0, 0, 1));
	}

	switch (mCaseIndex) {
	case 0:
	case 1: {
		sceneMng->GetDefCamera()->SetSkyBox(rendFac->CreateSkybox(test1::res::Sky()));

		mModel = rendFac->CreateAssimpModel(!(mCaseIndex&1) ? E_MAT_MODEL : E_MAT_MODEL_PBR);
		mModel->LoadModel(test1::res::model_mir::Path(), test1::res::model_mir::Rd());
		mTransform = mModel->GetTransform();
		mTransform->SetScale(test1::res::model_mir::Scale());
		mTransform->SetPosition(Eigen::Vector3f(0, -5, 0));
	}break;
	case 2:
	case 3:
	case 4:
	case 5: {
		sceneMng->AddPerspectiveCamera(winCenter + test1::cam::Eye(), test1::cam::NearFarFov());

		mModel = rendFac->CreateAssimpModel(!(mCaseIndex&1) ? E_MAT_MODEL : E_MAT_MODEL_PBR);
		if (mCaseIndex < 4) {
			mModel->LoadModel(test1::res::model_sship::Path(), test1::res::model_sship::Rd()); 
			mTransform = mModel->GetTransform();
			mTransform->SetScale(test1::res::model_sship::Scale());
			mTransform->SetPosition(winCenter + Eigen::Vector3f(0, -100, 0));
		}
		else {
			mModel->LoadModel(test1::res::model_rock::Path(), test1::res::model_rock::Rd());
			mTransform = mModel->GetTransform();
			mTransform->SetScale(test1::res::model_rock::Scale());
			mTransform->SetPosition(winCenter + Eigen::Vector3f(100, 100, 10));
		}
	}break;
	default:
		break;
	}
	mModel->PlayAnim(0);
}

void TestModel::OnRender()
{
	mModel->Update(mTimer->mDeltaTime);
	mContext->RenderPipe()->Draw(*mModel, *mContext->SceneMng());
}

auto reg = AppRegister<TestModel>("test_model");
