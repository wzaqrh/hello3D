#include "test/test_case.h"
#include "test/app.h"
#include "core/resource/material_factory.h"
#include "core/renderable/assimp_model.h"
#include "core/base/transform.h"

using namespace mir;

class TestCamera : public App
{
protected:
	void OnRender() override;
	void OnPostInitDevice() override;
private:
	AssimpModelPtr mModel = nullptr;
	SpritePtr mSprite = nullptr;
};

#define SCALE_BASE 0.01
void TestCamera::OnPostInitDevice()
{
	mContext->SceneMng()->RemoveAllCameras();
	mContext->SceneMng()->AddPerspectiveCamera(Eigen::Vector3f(0, 0, -30), 300, 45);
	mContext->SceneMng()->GetDefCamera()->SetSkyBox(
		mContext->RenderableFac()->CreateSkybox("model/uffizi_cross.dds"));

	mTransform->SetScale(Eigen::Vector3f(SCALE_BASE, SCALE_BASE, SCALE_BASE));
	mModel = mContext->RenderableFac()->CreateAssimpModel(mTransform, E_MAT_MODEL);
	mModel->PlayAnim(0);


}

void TestCamera::OnRender()
{

}

auto reg = AppRegister<TestCamera>("test_camera");