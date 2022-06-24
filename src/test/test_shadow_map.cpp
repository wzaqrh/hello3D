#include "test/framework/test_case.h"
#include "core/renderable/sprite.h"
#include "core/renderable/cube.h"
#include "core/mir_config_macros.h"
#include "core/rendersys/render_pipeline.h"

using namespace mir;
using namespace mir::rend;

class TestShadowMap : public App
{
protected:
	CoTask<bool> OnInitScene() override;
	void OnInitLight() override {}
	void OnInitCamera() override {}
	void CleanUp() override {
		mGuiDebugChannel.Dispose();
	}
private:
	GuiDebugWindow mGuiDebugChannel;
};

#define DEBUG_GL 1

CoTask<bool> TestShadowMap::OnInitScene()
{
	//SetPPU(1);
	TIME_PROFILE("TestShadowMap.OnInitScene");

	CameraPtr camera = mScneMng->CreateCameraNode(kCameraPerspective);
	camera->SetFov(45.0);

#if !DEBUG_GL
	mGuiDebugChannel.Init(mContext);
#endif

	AssimpModelPtr mModel;
	test1::res::model model;
	switch (mCaseIndex) {
	case 0:
	case 1: {
		bool isShadowVSM = true;// mContext->Config().IsShadowVSM();

		if (mCaseIndex == 0) {
			camera->SetLookAt(Eigen::Vector3f(0, 10, -10), Eigen::Vector3f::Zero());

			auto dir_light = mScneMng->CreateLightNode<DirectLight>();
			dir_light->SetLightRadius(1.0);
			dir_light->SetLookAt(Eigen::Vector3f(5, 5, -5), Eigen::Vector3f::Zero());
		}
		else {
			if (isShadowVSM) camera->SetLookAt(Eigen::Vector3f(-5, 10, -10), Eigen::Vector3f::Zero());
			else camera->SetLookAt(Eigen::Vector3f(-0.644995f, 0.614183f, -0.660632f), Eigen::Vector3f::Zero());

			auto dir_light = mScneMng->CreateLightNode<SpotLight>();
			dir_light->SetLightRadius(1.0);
			if (isShadowVSM) dir_light->SetLookAt(Eigen::Vector3f(0.5f, 1.0f, -1.0f) * 20, Eigen::Vector3f::Zero());
			else dir_light->SetLookAt(Eigen::Vector3f(3.57088f, 6.989f, -9.19698f), Eigen::Vector3f::Zero());
		}
	#if 1
		MaterialLoadParamBuilder skyMat = MAT_SKYBOX;
		camera->SetSkyBox(CoAwait mRendFac->CreateSkyboxT(test1::res::Sky(), skyMat));
	#endif

		MaterialLoadParamBuilder modelMat = MAT_MODEL;
	#if 1
		auto floor = mScneMng->AddRendAsNode(CoAwait mRendFac->CreateAssimpModelT(modelMat));
		mGuiDebugChannel.AddModel(floor);
		auto floorModel = CoAwait model.Init("floor", floor);
		floorModel->SetEulerAngles(Eigen::Vector3f(3.14 * 0.5, 0, 0));

		if (isShadowVSM) floorModel->SetScale(Eigen::Vector3f(0.8, 0.8, 0.8));
		else {
			floorModel->SetScale(Eigen::Vector3f(0.1, 0.1, 0.1));
			floor->SetCastShadow(false);
		}
	#endif

	#if 1
		mModel = mScneMng->AddRendAsNode(CoAwait mRendFac->CreateAssimpModelT(modelMat));
	#if !DEBUG_GL
		mGuiDebugChannel.AddModel(mModel);
	#endif
		if (mCaseIndex == 0) {
			mTransform = CoAwait model.Init("buddha", mModel);
			mTransform->SetPosition(Eigen::Vector3f(0, 2, 0));
			mTransform->SetEulerAngles(Eigen::Vector3f(0, 3.14, 0));

			if (isShadowVSM) mTransform->SetScale(Eigen::Vector3f(5, 5, 5));
		}
		else {
			mTransform = CoAwait model.Init("armadillo", mModel);
			if (isShadowVSM) mTransform->SetScale(Eigen::Vector3f(0.05, 0.05, 0.05));
			else mTransform->SetScale(Eigen::Vector3f(0.005, 0.005, 0.005));
		}
	#endif
	}break;
	default:
		break;
	}
#if !DEBUG_GL
	mGuiDebugChannel.AddAllCmds();
#endif
	CoReturn true;
}

auto reg = AppRegister<TestShadowMap>("test_shadow_map");