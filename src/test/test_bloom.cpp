#include "test/framework/test_case.h"
#include "core/renderable/sprite.h"
#include "core/renderable/cube.h"
#include "core/renderable/post_process.h"

using namespace mir;
using namespace mir::rend;

class TestBloom : public App
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

CoTask<bool> TestBloom::OnInitScene()
{
	//SetPPU(1);
	TIME_PROFILE("testBloom.OnInitScene");

	CameraPtr camera = mScneMng->CreateCameraNode(kCameraPerspective);
	camera->SetFov(45.0);
	camera->SetRenderingPath((RenderingPath)mCaseSecondIndex);

	mGuiDebugChannel.Init(mContext);
	
	AssimpModelPtr mModel;
	test1::res::model model;
	switch (mCaseIndex) {
	case 0:
	case 1: {
		bool isShadowVSM = mContext->Config().IsShadowVSM();

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

		MaterialLoadParamBuilder skyMat = MAT_SKYBOX;
		camera->SetSkyBox(CoAwait mRendFac->CreateSkyboxT(test1::res::Sky(), skyMat));

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
		mGuiDebugChannel.AddModel(mModel);
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

		auto effect = CoAwait PostProcessFactory(mRendFac).CreateBloom();
		camera->AddPostProcessEffect(effect);
		camera->SetRenderingPath(kRenderPathDeffered);
		mGuiDebugChannel.AddPostProcessEffect(effect);
	}break;
	default:
		break;
	}

	mGuiDebugChannel.AddPostProcessCmd();
	mGuiDebugChannel.AddAllCmds();
	CoReturn true;
}

auto reg = AppRegister<TestBloom>("test_bloom");