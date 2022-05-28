#include "test/test_case.h"
#include "test/app.h"
#include "core/renderable/skybox.h"
#include "core/renderable/cube.h"

using namespace mir;
using namespace mir::rend;

class TestUnity : public App
{
protected:
	CoTask<bool> OnInitScene() override;
	void OnInitLight() override {}
	void OnInitCamera() override {}
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
		mir::rend::SphericalHarmonicsConstants shc;
		shc.C0C1 <<
			-0.0152, -0.0218, -0.0346, 0,
			 0.1034,  0.1204,  0.1464, 0,
			-0.0096,  0.0035,  0.0114, 0,
			 0.1712,  0.1500,  0.1627, 0;
		skybox->SetSphericalHarmonicsConstants(shc);
		camera->SetSkyBox(skybox);

	#if 1
		MaterialLoadParamBuilder modelMat = MAT_MODEL;
		auto mModel = mScneMng->AddRendAsNode(CoAwait mRendFac->CreateAssimpModelT(modelMat));

		mTransform = CoAwait model.Init("Spaceship", mModel);
		#define MODEL_SCALE 0.01
		mTransform->SetScale(Eigen::Vector3f(MODEL_SCALE, MODEL_SCALE, MODEL_SCALE));

		mModel->PlayAnim(0);
	#elif 1
		MaterialLoadParamBuilder modelMat = MAT_MODEL;
		auto mModel = mScneMng->AddRendAsNode(CoAwait mRendFac->CreateAssimpModelT(modelMat));

		mTransform = CoAwait model.Init("buddha", mModel);
		//mTransform->SetEulerAngles(Eigen::Vector3f(0, PPI, 0));
		#define MODEL_SCALE 10
		mTransform->SetScale(Eigen::Vector3f(MODEL_SCALE, MODEL_SCALE, MODEL_SCALE));
	#else
		mScneMng->AddRendAsNode(CoAwait mRendFac->CreateCubeT(Eigen::Vector3f::Zero(), Eigen::Vector3f(3.5, 3.5, 3.5)));
	#endif
	}break;
	default:
		break;
	}
	CoReturn true;
}

auto reg = AppRegister<TestUnity>("test_unity");
