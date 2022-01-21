#include "test/test_case.h"
#include "test/app.h"
#include "core/renderable/assimp_model.h"
#include "core/base/transform.h"

using namespace mir;
using namespace mir::renderable;

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
0: 小汽车
1: gltf盒子
对比https://www.khronos.org/news/press/khronos-releases-wave-of-new-gltf-pbr-3d-material-capabilities
2: 透视相机 观察到传奇战士: 左半有高光, 右边非全暗
3: 透视相机 观察到模型：飞机
4：透视相机 观察到模型：石头（在右上角）
5: 透视相机 观察到模型：地板

6-11: 正交相机
*/

inline MaterialLoadParam GetMatName(int secondIndex) {
	MaterialLoadParamBuilder mlpb(MAT_MODEL);
	mlpb["PBR_MODE"] = secondIndex % 3;
	return mlpb;
}

void TestModel::OnPostInitDevice()
{
	int caseIndex = mCaseIndex % 6;
	bool useOtho = mCaseIndex >= 6;

	switch (caseIndex) {
	case 0: {
		auto dir_light = mScneMng->AddDirectLight();
		dir_light->SetDirection(Eigen::Vector3f(0, -1, 0));
	}break;
	case 1:
	case 2: {
		auto pt_light = mScneMng->AddPointLight();
		pt_light->SetPosition(Eigen::Vector3f(0, 15, -5));
		pt_light->SetAttenuation(0.001);
	}break;
	case 3: {
		auto pt_light = mScneMng->AddPointLight();
		pt_light->SetPosition(Eigen::Vector3f(25, 0, -5));
		pt_light->SetAttenuation(0.005);

		auto dir_light = mScneMng->AddDirectLight();
		dir_light->SetDirection(Eigen::Vector3f(25, 0, 0));
	}break;
	case 4:{
		auto dir_light = mScneMng->AddDirectLight();
		dir_light->SetDirection(Eigen::Vector3f(0, -1, 1));
	}break;
	default: {
		auto dir_light = mScneMng->AddDirectLight();
		dir_light->SetDirection(Eigen::Vector3f(0, 0, 1));
	}break;
	}

	CameraPtr camera = useOtho 
		? mScneMng->AddOthogonalCamera(test1::cam::Eye(mWinCenter))
		: mScneMng->AddPerspectiveCamera(test1::cam::Eye(mWinCenter));

	test1::res::model model;
	switch (caseIndex) {
	case 0:
	case 1:
	case 2: {
		camera->SetLookAt(Eigen::Vector3f(0, 15, 0), Eigen::Vector3f::Zero());
		camera->SetSkyBox(mRendFac->CreateSkybox(test1::res::Sky(1)));

		mModel = mRendFac->CreateAssimpModel(GetMatName(mCaseSecondIndex));
		std::string modelNameArr[] = { "toycar", "box-space", "mir" };
		mTransform = model.Init(modelNameArr[caseIndex], mModel);
	}break;
	case 3:
	case 4: 
	case 5: {
		mModel = mRendFac->CreateAssimpModel(GetMatName(mCaseSecondIndex));
		std::string modelNameArr[] = { "spaceship", "rock", "floor" };
		mTransform = model.Init(modelNameArr[caseIndex-3], mModel);
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
