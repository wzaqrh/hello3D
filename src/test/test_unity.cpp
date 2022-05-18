#include "test/test_case.h"
#include "test/app.h"
#include "core/renderable/skybox.h"

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
	CameraPtr camera = mScneMng->CreateCameraNode(kCameraPerspective);
	camera->SetFov(60);
	camera->SetRenderingPath((RenderingPath)mCaseSecondIndex);

	test1::res::model model;
	switch (mCaseIndex) {
	case 0:
	case 1: {
		camera->SetLookAt(Eigen::Vector3f(0, 0, -10), Eigen::Vector3f::Zero());

		auto dir_light = mScneMng->CreateLightNode<DirectLight>();
		dir_light->SetLookAt(Eigen::Vector3f(0, 3, 0), Eigen::Vector3f::Zero());

		MaterialLoadParamBuilder skyMat = MAT_SKYBOX;
		auto skybox = CoAwait mRendFac->CreateSkyboxT(test1::res::Sky(2), skyMat);
		Eigen::Matrix4f c0c1; 
		c0c1 <<
			-0.015, -0.022, -0.035, 0,
			   0.1,   0.12,   0.15, 0,
			-0.096, 0.0035,  0.011, 0,
			  0.17,   0.15,   0.16, 0;
		skybox->SetSphericalHarmonicsConstants(c0c1);
		camera->SetSkyBox(skybox);

		MaterialLoadParamBuilder modelMat = MAT_MODEL;
		mModel = mScneMng->AddRendAsNode(CoAwait mRendFac->CreateAssimpModelT(modelMat));
		
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
