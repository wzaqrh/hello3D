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
	SpritePtr mSprite;
};

void TestCube::OnPostInitDevice()
{
	auto sceneMng = mContext->SceneMng();
	sceneMng->RemoveAllCameras();
	//sceneMng->AddOthogonalCamera(Eigen::Vector3f(0, 0, -1000), 1000);
	sceneMng->AddPerspectiveCamera(Eigen::Vector3f(0,0,-1500), 3000, 30);

	switch (mCaseIndex) {
	case 0: {
		auto size = mContext->ResourceMng()->WinSize() / 2;
		mCube = mContext->RenderableFac()->CreateCube(Eigen::Vector3f(0, 0, -201), Eigen::Vector3f(200, 200, 200));
		mTransform = mCube->GetTransform();

		//mSprite = mContext->RenderableFac()->CreateSprite("model/theyKilledKenny.png");
		//mSprite->SetSize(Eigen::Vector2f(win_width, win_height));
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
		if (mSprite) mSprite->GenRenderOperation(opQueue);

		mContext->RenderPipe()->Render(opQueue, *mContext->SceneMng());
		mContext->RenderPipe()->EndFrame();
	}
}

auto reg = AppRegister<TestCube>("test_cube");