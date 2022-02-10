#include "test/test_case.h"
#include "test/app.h"

using namespace mir;
using namespace mir::rend;

class TestDefferedPath : public App
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
0�ӳ�,1����͸����� ƽ�й�+��� �Աȹ۲�ɻ�
2�ӳ�,3����: ͸����� ����ƽ�й�  �Աȹ۲�ɻ�
*/

CoTask<bool> TestDefferedPath::OnPostInitDevice()
{
	CameraPtr camera = mScneMng->AddPerspectiveCamera(test1::cam::Eye(mWinCenter));
	camera->SetSkyBox(CoAwait mRendFac->CreateSkybox(test1::res::Sky()));
	camera->SetRenderingPath(RenderingPath((mCaseIndex+1)&1));

	switch (mCaseIndex) {
	case 0:
	case 1:{
		mScneMng->AddDirectLight();
		mScneMng->AddPointLight()->SetPosition(test1::vec::PosLight());

		mModel = CoAwait mRendFac->CreateAssimpModel(MAT_MODEL);
		mTransform = CoAwait test1::res::model().Init("spaceship", mModel);
	}break;
	case 2:
	case 3:{
		mScneMng->AddDirectLight();

		mModel = CoAwait mRendFac->CreateAssimpModel(MAT_MODEL);
		mTransform = CoAwait test1::res::model().Init("spaceship", mModel);
	}break;
	default:
		break;
	}
	CoReturn true;
}

void TestDefferedPath::OnRender()
{
	mModel->Update(mTimer->mDeltaTime);
	mContext->RenderPipe()->Draw(*mModel, *mContext->SceneMng());
}

auto reg = AppRegister<TestDefferedPath>("test_deffered_path");
