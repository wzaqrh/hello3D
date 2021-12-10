#include "test/test_case.h"
#include "test/app.h"
#include "core/base/transform.h"
#include "core/renderable/cube.h"
#include "core/scene/scene_manager.h"

using namespace mir;

class TestCube : public App
{
protected:
	void OnRender() override;
	void OnPostInitDevice() override;
private:
	CubePtr mCube;
};
/*mCaseIndex
0��͸����� �۲쵽��������
1��͸����� �۲쵽����ƽ��(����������)
*/

void TestCube::OnPostInitDevice()
{
	switch (mCaseIndex) {
	case 0: {
		mCube = mRendFac->CreateCube(mWinCenter, Eigen::Vector3f(1,1,1), 0xff87CEFA);
		mTransform = mCube->GetTransform();
		mTransform->SetEuler(Eigen::Vector3f(45*0.174, 45*0.174, 45*0.174));
	}break;
	case 1: {
		const int SizeInf = 256;
		mCube = mRendFac->CreateCube(mWinCenter, Eigen::Vector3f(SizeInf, SizeInf, 1), 0xff87CEFA);
		mTransform = mCube->GetTransform();
	}break;
	default:
		break;
	}
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