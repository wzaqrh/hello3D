#include "test/test_case.h"
#include "test/app.h"
#include "core/renderable/sprite.h"
#include "core/renderable/cube.h"
#include "test/unit_test/unit_test.h"

using namespace mir;
using namespace mir::rend;

class TestShadowMap : public App
{
protected:
	CoTask<bool> OnInitScene() override;
	void OnInitLight() override {}
	void OnInitCamera() override {}
private:
	AssimpModelPtr mModel;
};

inline MaterialLoadParamBuilder GetMatName(int secondIndex) {
	MaterialLoadParamBuilder mlpb(MAT_MODEL);
	mlpb["PBR_MODE"] = 2;
	return mlpb;
}

CoTask<bool> TestShadowMap::OnInitScene()
{
	//SetPPU(1);
	TIME_PROFILE("testSSAO.OnInitScene");

	CameraPtr camera = mScneMng->CreateAddCameraNode(kCameraPerspective, test1::cam::Eye(mWinCenter));
	camera->SetFov(45.0);
	camera->SetRenderingPath((RenderingPath)mCaseSecondIndex);

	bool isShadowVSM = mContext->Config().IsShadowVSM();

	test1::res::model model;
	switch (mCaseIndex) {
	case 0:
	case 1: {
		if (mCaseIndex == 1) {
			camera->SetLookAt(Eigen::Vector3f(0, 5, -5), Eigen::Vector3f::Zero());

			auto dir_light = mScneMng->CreateAddLightNode<DirectLight>();
			dir_light->SetLightRadius(1.0);
			dir_light->SetLookAt(Eigen::Vector3f(5, 5, -5), Eigen::Vector3f::Zero());
		}
		else {
			if (isShadowVSM) camera->SetLookAt(Eigen::Vector3f(-5, 10, -10), Eigen::Vector3f::Zero());
			else camera->SetLookAt(Eigen::Vector3f(-0.644995f, 0.614183f, -0.660632f), Eigen::Vector3f::Zero());

			auto dir_light = mScneMng->CreateAddLightNode<SpotLight>();
			dir_light->SetLightRadius(1.0);
			if (isShadowVSM) dir_light->SetLookAt(Eigen::Vector3f(0.5f, 1.0f, -1.0f) * 20, Eigen::Vector3f::Zero());
			else dir_light->SetLookAt(Eigen::Vector3f(3.57088f, 6.989f, -9.19698f), Eigen::Vector3f::Zero());
		}

		MaterialLoadParamBuilder skyMat = MAT_SKYBOX;
		skyMat["CubeMapIsRightHandness"] = TRUE;
		camera->SetSkyBox(CoAwait mRendFac->CreateSkyboxT(test1::res::Sky(2), skyMat));

		MaterialLoadParamBuilder modelMat = GetMatName(mCaseSecondIndex);
		modelMat["CubeMapIsRightHandness"] = TRUE;

	#if 1
		auto floor = mScneMng->AddRendNode(CoAwait mRendFac->CreateAssimpModelT(modelMat));
		auto floorModel = CoAwait model.Init("floor", floor);
		floorModel->SetEulerAngles(Eigen::Vector3f(3.14 * 0.5, 0, 0));

		if (isShadowVSM) floorModel->SetScale(Eigen::Vector3f(0.8, 0.8, 0.8));
		else {
			floorModel->SetScale(Eigen::Vector3f(0.1, 0.1, 0.1));
			floor->SetCastShadow(false);
		}
	#endif

	#if 1
		mModel = mScneMng->AddRendNode(CoAwait mRendFac->CreateAssimpModelT(modelMat));
		if (mCaseIndex == 1) {
			mTransform = CoAwait model.Init("buddha", mModel);
			mTransform->SetEulerAngles(Eigen::Vector3f(0, 3.14, 0));

			if (isShadowVSM) mTransform->SetScale(Eigen::Vector3f(5, 5, 5));
		}
		else {
			mTransform = CoAwait model.Init("armadillo", mModel);
			if (isShadowVSM) mTransform->SetScale(Eigen::Vector3f(0.05, 0.05, 0.05));
			else mTransform->SetScale(Eigen::Vector3f(0.005, 0.005, 0.005));
		}
	#endif

		camera->SetRenderingPath((mir::RenderingPath)mCaseSecondIndex);
	}break;
	default:
		break;
	}
	CoReturn true;
}

auto reg = AppRegister<TestShadowMap>("test_shadow_map");