#include "test/test_case.h"
#include "test/app.h"
#include "core/scene/scene_manager.h"
#include "core/renderable/assimp_model.h"
#include "core/base/transform.h"

using namespace mir;

class TestSpecSkybox : public App
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
};
/*mCaseIndex
0��͸����� �۲쵽��պ�	 
1��͸�������λ��, ����zrangeԶС���������, �۲쵽��պб����������� 
*/

void TestSpecSkybox::OnPostInitDevice()
{
	auto sceneMng = mContext->SceneMng();
	auto rendFac = mContext->RenderableFac();
	auto resMng = mContext->ResourceMng();
	auto halfSize = mContext->ResourceMng()->WinSize() / 2;
	auto winCenter = Eigen::Vector3f(halfSize.x(), halfSize.y(), 0);

	if (mCaseIndex == 1) {
		sceneMng->RemoveAllCameras();
		sceneMng->AddPerspectiveCamera(winCenter + test1::cam1::Eye(), test1::cam1::NearFarFov());
	}

	sceneMng->GetDefCamera()->SetSkyBox(rendFac->CreateSkybox(test1::res::Sky()));//bc1a mipmap cube
}

void TestSpecSkybox::OnRender()
{
	if (mContext->RenderPipe()->BeginFrame()) {
		RenderOperationQueue opQue;
		mContext->RenderPipe()->Render(opQue, *mContext->SceneMng());
		mContext->RenderPipe()->EndFrame();
	}
}

auto reg = AppRegister<TestSpecSkybox>("test_spec_skybox");