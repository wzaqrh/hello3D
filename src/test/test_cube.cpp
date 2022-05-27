#include "test/test_case.h"
#include "test/app.h"
#include "core/resource/texture_factory.h"
#include "core/resource/resource_manager.h"
#include "core/renderable/cube.h"

using namespace mir;
using namespace mir::rend;

class TestCube : public App
{
protected:
	CoTask<bool> OnInitScene() override;
};
/*mCaseIndex
0：透视相机 观察到淡蓝盒子
1：透视相机 观察到淡蓝平面(滚动鼠标滚轮)
*/

CoTask<bool> TestCube::OnInitScene()
{
	switch (mCaseIndex) {
	case 0: {
		auto mCube = CoAwait mRendFac->CreateCubeT(mWinCenter, Eigen::Vector3f(1,1,1), 0xff87CEFA);
		mScneMng->AddRendAsNode(mCube);
		mTransform = mCube->GetTransform();
		mTransform->SetEulerAngles(Eigen::Vector3f(45*0.174, 45*0.174, 45*0.174));
	}break;
	case 1: {
		const int SizeInf = 256;
		auto mCube = CoAwait mRendFac->CreateCubeT(mWinCenter, Eigen::Vector3f(SizeInf, 1, SizeInf), 0xff87CEFA);
		mScneMng->AddRendAsNode(mCube);
		mTransform = mCube->GetTransform();
	}break;
	case 2: {
		MaterialLoadParamBuilder param = MAT_MODEL;
		param["USE_NORMAL"] = FALSE;
		param["PBR_MODE"] = 0;
		auto mCube = CoAwait mRendFac->CreateCubeT(mWinCenter, Eigen::Vector3f(1, 1, 1), 0xff87CEFA, param);
		mCube->SetTexture(CoAwait mResMng->CreateTextureByFileT(__LaunchAsync__, test1::res::dds::Kenny()));
		mCube->GetMaterial().SetProperty<BOOL>("EnableAlbedoMap", TRUE);
		mScneMng->AddRendAsNode(mCube);
		mTransform = mCube->GetTransform();
		mTransform->SetEulerAngles(Eigen::Vector3f(45 * 0.174, 45 * 0.174, 45 * 0.174));
	}break;
	case 3: {
		const int SizeInf = 256;
		MaterialLoadParamBuilder param = MAT_MODEL;
		param["USE_NORMAL"] = FALSE;
		param["PBR_MODE"] = 0;
		auto mCube = CoAwait mRendFac->CreateCubeT(mWinCenter, Eigen::Vector3f(SizeInf, 1, SizeInf), 0xff87CEFA, param);
		mScneMng->AddRendAsNode(mCube);
	}break;
	default:
		break;
	}
	CoReturn true;
}

auto reg = AppRegister<TestCube>("test_cube");