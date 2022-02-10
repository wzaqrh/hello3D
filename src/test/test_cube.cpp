#include "test/test_case.h"
#include "test/app.h"
#include "core/renderable/cube.h"

using namespace mir;
using namespace mir::rend;

class TestCube : public App
{
protected:
	void OnRender() override;
	CoTask<bool> OnPostInitDevice() override;
private:
	CubePtr mCube;
};
/*mCaseIndex
0：透视相机 观察到淡蓝盒子
1：透视相机 观察到淡蓝平面(滚动鼠标滚轮)
*/

CoTask<bool> TestCube::OnPostInitDevice()
{
	switch (mCaseIndex) {
	case 0: {
		mCube = CoAwait mRendFac->CreateCube(mWinCenter, Eigen::Vector3f(1,1,1), 0xff87CEFA);
		mTransform = mCube->GetTransform();
		mTransform->SetEuler(Eigen::Vector3f(45*0.174, 45*0.174, 45*0.174));
	}break;
	case 1: {
		const int SizeInf = 256;
		mCube = CoAwait mRendFac->CreateCube(mWinCenter, Eigen::Vector3f(SizeInf, SizeInf, 1), 0xff87CEFA);
		mTransform = mCube->GetTransform();
	}break;
	default:
		break;
	}
	CoReturn true;
}

void TestCube::OnRender()
{
	if (mContext->RenderPipe()->BeginFrame()) {
		RenderOperationQueue opQueue;
		if (mCube) mCube->GenRenderOperation(opQueue);
		mContext->RenderPipe()->Render(opQueue, *mContext->SceneMng());
		mContext->RenderPipe()->EndFrame();
	}
}

auto reg = AppRegister<TestCube>("test_cube");