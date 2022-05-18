#include "test/test_case.h"
#include "test/app.h"

using namespace mir;
using namespace mir::rend;

class TestLight : public App
{
protected:
	CoTask<bool> OnInitScene() override;
	void OnInitLight() override {}
	void OnInitCamera() override {}
private:
	AssimpModelPtr mModel;
};
/*mCaseIndex
0��͸����� ��ǰ����		�۲쵽�ɻ��߹�ƫ��
1: ͸����� ��ǰ����		�۲쵽�ɻ��߹�ƫ��
2��͸����� �Ҳ���			�۲쵽�ɻ��߹�����, ��߼���ȫ��
3��͸����� ����ǰ��Զ���		�۲쵽�ɻ��߹�ƫ���µ��ǳ���

4: �������
5: �������
6: �������
7: �������
*/

CoTask<bool> TestLight::OnInitScene()
{
	constexpr int CaseCountMod = 4;
	int caseIndex = mCaseIndex % CaseCountMod;
	int cameraType = mCaseIndex / CaseCountMod;
	//SetPPU(1);

	switch (caseIndex) {
	case 0: {
		mScneMng->CreateLightNode<PointLight>()->SetPosition(Eigen::Vector3f(0, 10, -5));
	}break;
	case 1: {
		mScneMng->CreateLightNode<PointLight>()->SetPosition(Eigen::Vector3f(-10, 0, -5));
	}break;
	case 2: {
		mScneMng->CreateLightNode<PointLight>()->SetPosition(Eigen::Vector3f(10, 0, 0));
	}break;
	case 3: {
		auto pt_light = mScneMng->CreateLightNode<PointLight>();
		pt_light->SetPosition(Eigen::Vector3f(15, -15, -10));
		pt_light->SetAttenuation(0.01);
	}break;
	default: {
		mScneMng->CreateLightNode<DirectLight>()->SetDirection(Eigen::Vector3f(25, 0, 5));
		mScneMng->CreateLightNode<DirectLight>()->SetDirection(Eigen::Vector3f(0, 0, 1));
	}break;
	}

	CameraPtr camera = mScneMng->CreateCameraNode((CameraType)cameraType);
	camera->SetSkyBox(CoAwait mRendFac->CreateSkyboxT(test1::res::Sky()));

	mModel = CoAwait mRendFac->CreateAssimpModelT(MAT_MODEL);
	mScneMng->AddRendAsNode(mModel);
	mTransform = CoAwait test1::res::model().Init("spaceship", mModel);
	CoReturn true;
}

auto reg = AppRegister<TestLight>("test_light");
