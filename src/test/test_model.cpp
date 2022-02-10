#include "test/test_case.h"
#include "test/app.h"

using namespace mir;
using namespace mir::rend;

class TestModel : public App
{
protected:
	void OnRender() override;
	CoTask<bool> OnPostInitDevice() override;
	void OnInitLight() override {}
	void OnInitCamera() override {}
private:
	AssimpModelPtr mModel;
};
/*mCaseIndex
0: С����
1: gltf����
�Ա�https://www.khronos.org/news/press/khronos-releases-wave-of-new-gltf-pbr-3d-material-capabilities
2: ͸����� �۲쵽����սʿ: ����и߹�, �ұ߷�ȫ��
3: ͸����� �۲쵽ģ�ͣ��ɻ�
4��͸����� �۲쵽ģ�ͣ�ʯͷ�������Ͻǣ�
5: ͸����� �۲쵽ģ�ͣ��ذ�

6-11: �������
*/

inline MaterialLoadParam GetMatName(int secondIndex) {
	MaterialLoadParamBuilder mlpb(MAT_MODEL);
	mlpb["PBR_MODE"] = secondIndex % 3;
	return mlpb;
}

CoTask<bool> TestModel::OnPostInitDevice()
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
		camera->SetSkyBox(CoAwait mRendFac->CreateSkybox(test1::res::Sky(1)));

		mModel = CoAwait mRendFac->CreateAssimpModel(GetMatName(mCaseSecondIndex));
		std::string modelNameArr[] = { "toycar", "box-space", "mir" };
		mTransform = CoAwait model.Init(modelNameArr[caseIndex], mModel);
	}break;
	case 3:
	case 4: 
	case 5: {
		mModel = CoAwait mRendFac->CreateAssimpModel(GetMatName(mCaseSecondIndex));
		std::string modelNameArr[] = { "spaceship", "rock", "floor" };
		mTransform = CoAwait model.Init(modelNameArr[caseIndex-3], mModel);
	}break;
	default:
		break;
	}

	if (camera->GetType() == kCameraOthogonal)
		mTransform->SetScale(mTransform->GetLossyScale() * 50);

	mModel->PlayAnim(0);
	CoReturn true;
}

void TestModel::OnRender()
{
	mModel->Update(mTimer->mDeltaTime);
	mContext->RenderPipe()->Draw(*mModel, *mContext->SceneMng());
}

auto reg = AppRegister<TestModel>("test_model");
