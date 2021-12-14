#include "test/test_case.h"
#include "test/app.h"
#include "core/renderable/assimp_model.h"
#include "core/base/transform.h"

using namespace mir;

class TestLight : public App
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
0��͸����� ��ǰ����		�۲쵽�ɻ��߹�ƫ��
1: ͸����� ��ǰ����		�۲쵽�ɻ��߹�ƫ��
2��͸����� �Ҳ���			�۲쵽�ɻ��߹�����, ��߼���ȫ��
3��͸����� ����ǰ��Զ���		�۲쵽�ɻ��߹�ƫ���µ��ǳ���

4: �������
5: �������
6: �������
7: �������
*/

void TestLight::OnPostInitDevice()
{
	constexpr int LightTypeCount = 4;
	mScneMng->SetPixelPerUnit(1);

	switch (mCaseIndex % LightTypeCount) {
	case 0: {
		mScneMng->AddPointLight()->SetPosition(Eigen::Vector3f(0, 10, -5));
	}break;
	case 1: {
		mScneMng->AddPointLight()->SetPosition(Eigen::Vector3f(-10, 0, -5));
	}break;
	case 2: {
		mScneMng->AddPointLight()->SetPosition(Eigen::Vector3f(10, 0, 0));
	}break;
	case 3: {
		auto pt_light = mScneMng->AddPointLight();
		pt_light->SetPosition(Eigen::Vector3f(15, -15, -10));
		pt_light->SetAttenuation(0.01);
	}break;
	default: {
		mScneMng->AddDirectLight()->SetDirection(Eigen::Vector3f(25, 0, 5));
		mScneMng->AddDirectLight()->SetDirection(Eigen::Vector3f(0, 0, 1));
	}break;
	}

	CameraPtr camera;
	if (mCaseIndex / LightTypeCount == 0)
		camera = mScneMng->AddPerspectiveCamera(test1::cam::Eye(mWinCenter));
	else
		camera = mScneMng->AddOthogonalCamera(test1::cam::Eye(mWinCenter));

	mModel = mRendFac->CreateAssimpModel(MAT_MODEL);
	mTransform = test1::res::model_sship::Init(mModel, mWinCenter);
}

void TestLight::OnRender()
{
	mModel->Update(mTimer->mDeltaTime);
	mContext->RenderPipe()->Draw(*mModel, *mContext->SceneMng());
}

auto reg = AppRegister<TestLight>("test_light");
