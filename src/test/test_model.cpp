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
0,1：透视相机 观察到传奇战士: 左半有高光, 右边非全暗 
对比https://www.khronos.org/news/press/khronos-releases-wave-of-new-gltf-pbr-3d-material-capabilities

2,3: 透视相机 观察到模型：飞机
4,5：透视相机 观察到模型：石头（在右上角）
6,7: 透视相机 观察到模型：地板

8-15: 正交相机
*/

void TestModel::OnPostInitDevice()
{
	int caseIndex = mCaseIndex % 8;
	bool useOtho = mCaseIndex >= 8;

	switch (caseIndex) {
	case 0:
	case 1: {
		auto pt_light = mScneMng->AddPointLight();
		pt_light->SetPosition(Eigen::Vector3f(0, 15, -5));
		pt_light->SetAttenuation(0.001);
	}break;
	case 2:
	case 3: {
		auto pt_light = mScneMng->AddPointLight();
		pt_light->SetPosition(Eigen::Vector3f(25, 0, -5));
		pt_light->SetAttenuation(0.005);

		auto dir_light = mScneMng->AddDirectLight();
		dir_light->SetDirection(Eigen::Vector3f(25, 0, 0));
	}break;
	case 6:
	case 7:{
		auto dir_light = mScneMng->AddDirectLight();
		dir_light->SetDirection(Eigen::Vector3f(0, -1, 1));
	}break;
	default: {
		auto dir_light = mScneMng->AddDirectLight();
		dir_light->SetDirection(Eigen::Vector3f(0, 0, 1));
	}break;
	}

	mir::CameraPtr camera = useOtho 
		? mScneMng->AddOthogonalCamera(test1::cam::Eye(mWinCenter))
		: mScneMng->AddPerspectiveCamera(test1::cam::Eye(mWinCenter));

	test1::res::model model;
	switch (caseIndex) {
	case 0:
	case 1: {
		camera->SetLookAt(Eigen::Vector3f(0, 15, 0), Eigen::Vector3f::Zero());
		camera->SetSkyBox(mRendFac->CreateSkybox(test1::res::Sky()));

		mModel = mRendFac->CreateAssimpModel(!(caseIndex&1) ? MAT_MODEL : MAT_MODEL_PBR);
		if (caseIndex == 0) mTransform = model.Init("toycar", mModel);
		else mTransform = model.Init("box-space", mModel);
	}break;
	case 2:
	case 3: {
		mModel = mRendFac->CreateAssimpModel(!(caseIndex&1) ? MAT_MODEL : MAT_MODEL_PBR);
		mTransform = model.Init("spaceship", mModel);
	}break;
	case 4:
	case 5: {
		mModel = mRendFac->CreateAssimpModel(!(caseIndex&1) ? MAT_MODEL : MAT_MODEL_PBR);
		mTransform = model.Init("rock", mModel);
	}break;
	case 6:
	case 7: {
		mModel = mRendFac->CreateAssimpModel(!(caseIndex & 1) ? MAT_MODEL : MAT_MODEL_PBR);
		mTransform = model.Init("floor", mModel);
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
