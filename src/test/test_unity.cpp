#include "test/test_case.h"
#include "test/app.h"

using namespace mir;
using namespace mir::rend;

class TestUnity : public App
{
protected:
	CoTask<bool> OnInitScene() override;
	void OnInitLight() override {}
	void OnInitCamera() override {}
private:
	AssimpModelPtr mModel;
};

CoTask<bool> TestUnity::OnInitScene()
{
	CameraPtr camera = mScneMng->CreateAddCameraNode(kCameraPerspective, test1::cam::Eye(mWinCenter));
	camera->SetFov(60);
	camera->SetRenderingPath((RenderingPath)mCaseSecondIndex);

	test1::res::model model;
	switch (mCaseIndex) {
	case 0:
	case 1: {
		camera->SetLookAt(Eigen::Vector3f(0, 1, -10), Eigen::Vector3f::Zero());

		auto dir_light = mScneMng->CreateAddLightNode<DirectLight>();
		dir_light->SetLookAt(Eigen::Vector3f(0, 3, 0), Eigen::Vector3f::Zero());

		MaterialLoadParamBuilder skyMat = MAT_SKYBOX;
		camera->SetSkyBox(CoAwait mRendFac->CreateSkyboxT(test1::res::Sky(2), skyMat));

		MaterialLoadParamBuilder modelMat = MAT_MODEL;
		mModel = mScneMng->AddRendNode(CoAwait mRendFac->CreateAssimpModelT(modelMat));
		
	#define PPI 3.14159265358979323846264338327
	#if 1
		mTransform = CoAwait model.Init("Spaceship", mModel);
		//mTransform->SetEulerAngles(Eigen::Vector3f(0, PPI, 0));
		#define MODEL_SCALE 0.01
		mTransform->SetScale(Eigen::Vector3f(MODEL_SCALE, MODEL_SCALE, MODEL_SCALE));
	#else
		mTransform = CoAwait model.Init("buddha", mModel);
		mTransform->SetEulerAngles(Eigen::Vector3f(0, PPI, 0));
	#define MODEL_SCALE 10
		mTransform->SetScale(Eigen::Vector3f(MODEL_SCALE, MODEL_SCALE, MODEL_SCALE));
	#endif

		mModel->PlayAnim(0);
	}break;
	default:
		break;
	}
	CoReturn true;
}

auto reg = AppRegister<TestUnity>("test_unity");
