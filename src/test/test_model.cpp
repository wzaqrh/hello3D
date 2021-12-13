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

6,7：正交相机 观察到模型：传奇战士 + 天空
8,9: 正交相机 观察到模型：飞机
10,11：正交相机 观察到模型：石头（在右上角）
*/

void TestModel::OnPostInitDevice()
{
	int caseIndex = mCaseIndex % 6;
	bool useOtho = mCaseIndex >= 6;

	if (caseIndex == 0 || caseIndex == 1) 
	{
		auto pt_light = mScneMng->AddPointLight();
		pt_light->SetPosition(mWinCenter + Eigen::Vector3f(0, 5, -5));

		auto dir_light = mScneMng->AddDirectLight();
		dir_light->SetDirection(Eigen::Vector3f(25, 0, 5));
	}
	else 
	{
		auto dir_light = mScneMng->AddDirectLight();
		dir_light->SetDirection(Eigen::Vector3f(0, 0, 1));
	}

	mir::CameraPtr camera = useOtho 
		? mScneMng->AddOthogonalCamera(test1::cam::Eye(mWinCenter), test1::cam::NearFarFov())
		: mScneMng->AddPerspectiveCamera(test1::cam::Eye(mWinCenter), test1::cam::NearFarFov());

	switch (caseIndex) {
	case 0:
	case 1: {
		camera->SetSkyBox(mRendFac->CreateSkybox(test1::res::Sky()));

		mModel = mRendFac->CreateAssimpModel(!(caseIndex&1) ? MAT_MODEL : MAT_MODEL_PBR);
		mTransform = test1::res::model_mir::Init(mModel, mWinCenter);
	}break;
	case 2:
	case 3:
	case 4:
	case 5: {
		mModel = mRendFac->CreateAssimpModel(!(caseIndex&1) ? MAT_MODEL : MAT_MODEL_PBR);
		if (caseIndex == 2 || caseIndex == 3) mTransform = test1::res::model_sship::Init(mModel, mWinCenter);
		else mTransform = test1::res::model_rock::Init(mModel, mWinCenter);
	}break;
	default:
		break;
	}

	if (camera->GetType() == kCameraOthogonal)
		mTransform->SetScale(mTransform->GetScale() * 50);

	mModel->PlayAnim(0);
}

void TestModel::OnRender()
{
	mModel->Update(mTimer->mDeltaTime);
	mContext->RenderPipe()->Draw(*mModel, *mContext->SceneMng());
}

auto reg = AppRegister<TestModel>("test_model");
