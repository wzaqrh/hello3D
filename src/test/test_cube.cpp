#include "test/test_case.h"
#include "test/app.h"
#include "core/scene/scene_manager.h"
#include "core/renderable/cube.h"

using namespace mir;

class TestCube : public App
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
private:
	CubePtr mCube;
};

void TestCube::OnPostInitDevice()
{
	auto sceneMng = mContext->SceneMng();
	sceneMng->RemoveAllCameras();
	sceneMng->AddPerspectiveCamera(Eigen::Vector3f(0,0,-1500), 3000, 30);

	auto size = mContext->ResourceMng()->WinSize() / 2;
	switch (mCaseIndex) {
	case 0: {
		mCube = mContext->RenderableFac()->CreateCube(Eigen::Vector3f(0, 0, -201), Eigen::Vector3f(200, 200, 200), 0xffff6347);
	}break;
	case 1: {
		const int SizeInf = 10000;
		mCube = mContext->RenderableFac()->CreateCube(Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(SizeInf, SizeInf, 1), 0xffff6347);
	}break;
	default:
		break;
	}
	mTransform = mCube->GetTransform();
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