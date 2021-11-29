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
	virtual void OnInitLight() override;
	virtual void OnPostInitDevice() override;
};

void TestSpecSkybox::OnInitLight()
{
	auto light2 = mContext->SceneMng()->AddDirectLight();
	light2->SetDirection(Eigen::Vector3f(0, 0, 1));
}

void TestSpecSkybox::OnPostInitDevice()
{
	mContext->SceneMng()->GetDefCamera()->SetSkyBox(
		mContext->RenderableFac()->CreateSkybox("model/uffizi_cross_mip.dds"));//bc1a mipmap cube
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