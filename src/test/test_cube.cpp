#include "test/test_case.h"
#include "test/app.h"
#include "core/renderable/cube.h"

using namespace mir;
using namespace mir::rend;

class TestCube : public App
{
protected:
	CoTask<bool> OnPostInitDevice() override;
};
/*mCaseIndex
0��͸����� �۲쵽��������
1��͸����� �۲쵽����ƽ��(����������)
*/

CoTask<bool> TestCube::OnPostInitDevice()
{
	switch (mCaseIndex) {
	case 0: {
		auto mCube = CoAwait mRendFac->CreateCube(mWinCenter, Eigen::Vector3f(1,1,1), 0xff87CEFA);
		mScneMng->AddRendNode(mCube);
		mTransform = mCube->GetTransform();
		mTransform->SetEulerAngles(Eigen::Vector3f(45*0.174, 45*0.174, 45*0.174));
	}break;
	case 1: {
		const int SizeInf = 256;
		auto mCube = CoAwait mRendFac->CreateCube(mWinCenter, Eigen::Vector3f(SizeInf, SizeInf, 1), 0xff87CEFA);
		mScneMng->AddRendNode(mCube);
		mTransform = mCube->GetTransform();
	}break;
	default:
		break;
	}
	CoReturn true;
}

auto reg = AppRegister<TestCube>("test_cube");