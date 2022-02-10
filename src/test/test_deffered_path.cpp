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
0延迟,1正向：透视相机 平行光+电光 对比观察飞机
2延迟,3正向: 透视相机 正向平行光  对比观察飞机
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
