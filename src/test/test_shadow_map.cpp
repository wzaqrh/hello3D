#include "test/test_case.h"
#include "test/app.h"
#include "core/scene/scene_manager.h"
#include "core/renderable/assimp_model.h"
#include "core/renderable/sprite.h"
#include "core/renderable/cube.h"
#include "core/base/transform.h"
#include "test/unit_test/unit_test.h"

using namespace mir;

class TestShadowMap : public App
{
protected:
	void OnRender() override;
	void OnPostInitDevice() override;
	void OnInitLight() override {}
	void OnInitCamera() override {}
private:
	AssimpModelPtr mModel1, mModel2;
	CubePtr mCube0, mCube1;
};
/*mCaseIndex
0：透视相机, 观察到正常阴影: 小球阴影打到石头 (LayerColor.xml DEBUG_SHADOW_MAP=0)
1: 正交相机, 观察到正常阴影: 小球阴影打到石头 (LayerColor.xml DEBUG_SHADOW_MAP=0)
2：正交相机, '飞机'阴影打到'平面', 观察到阴影位置是否在'飞机'右侧 (LayerColor.xml DEBUG_SHADOW_MAP=1)
3: 正交相机, '摄像头'与'平行光'重合, 观察到'小平面'颜色比'大平面'深 (LayerColor.xml DEBUG_SHADOW_MAP=2)
*/

#define USE_RENDER_TEXTURE
#define SCALE_BASE 100
void TestShadowMap::OnPostInitDevice()
{
	auto sceneMng = mContext->SceneMng();
	auto rendFac = mContext->RenderableFac();
	auto halfSize = mContext->ResourceMng()->WinSize() / 2;
	auto winCenter = Eigen::Vector3f(halfSize.x(), halfSize.y(), 0);

	if (mCaseIndex == 2 || mCaseIndex == 3)
	{
		CameraPtr camera = sceneMng->AddOthogonalCamera(winCenter + test1::cam::Eye(), test1::cam::NearFarFov());

		auto light = sceneMng->AddDirectLight();
		if (mCaseIndex == 2) 
		{
			light->SetDirection(Eigen::Vector3f(2, 0, 10));
		}
		else 
		{
			light->SetDirection(test1::vec::DirLight());
			test::CompareLightCameraByViewProjection(*light, *camera, {
				Eigen::Vector4f(0, 768, test1::cam::Eye().z() + 1, 1.0),
				Eigen::Vector4f(1024, 0, test1::cam::Eye().z() + test1::cam::NearFarFov().y() - 1, 1.0),
				Eigen::Vector4f(77, 400, 0, 1.0),
				Eigen::Vector4f(600, 77, 0, 1.0),
			});
		}

		if (1) 
		{
			const int SizeBig = 10000, SizeSmall = 25;
			mCube0 = rendFac->CreateCube(
				winCenter + Eigen::Vector3f(-SizeBig*0.5, -SizeBig*0.5, test1::cam::Eye().z() + test1::cam::NearFarFov().y() - 10),
				Eigen::Vector3f(SizeBig, SizeBig, 1),
				0xffff6347
			);
			mCube1 = rendFac->CreateCube(
				winCenter + Eigen::Vector3f(-SizeSmall*0.5, -SizeSmall*0.5, test1::cam::Eye().z() + 1),
				Eigen::Vector3f(SizeSmall, SizeSmall, 1),
				0xffff4763
			);
		}

		if (mCaseIndex == 2 && 0)
		{
			mModel2 = rendFac->CreateAssimpModel(E_MAT_MODEL);
			mModel2->LoadModel(test1::res::model_sship::Path(), test1::res::model_sship::Rd());
			mTransform = mModel2->GetTransform();
			mTransform->SetScale(test1::res::model_sship::Scale() * 0.2);
			mTransform->SetPosition(winCenter + Eigen::Vector3f(0, 0, 0));
			mModel2->PlayAnim(0);
		}
	}
	else if (mCaseIndex == 0 || mCaseIndex == 1)
	{
		auto dir_light = sceneMng->AddDirectLight();
		dir_light->SetDirection(Eigen::Vector3f(-1, -1, 3));

		if (mCaseIndex == 0) 
		{
			auto camera = sceneMng->AddOthogonalCamera(winCenter + test1::cam::Eye(), test1::cam::NearFarFov());
			test::CompareLightCameraByViewProjection(*dir_light, *camera, {});
		}
		else 
		{
			sceneMng->AddPerspectiveCamera(winCenter + test1::cam::Eye(), test1::cam::NearFarFov());
		}

		if (1) 
		{
			const int SizeInf = 10000;
			mCube0 = mContext->RenderableFac()->CreateCube(
				winCenter + Eigen::Vector3f(-SizeInf*0.5, -SizeInf*0.5, 200), 
				Eigen::Vector3f(SizeInf, SizeInf, 2), 
				0xffff6347
			);
		}

		if (1) 
		{
			mModel1 = rendFac->CreateAssimpModel(E_MAT_MODEL);
			mModel1->LoadModel(test1::res::model_rock::Path(), test1::res::model_rock::Rd());
			mTransform = mModel1->GetTransform();
			mTransform->SetScale(test1::res::model_rock::Scale());
			mTransform->SetPosition(winCenter + Eigen::Vector3f(100, 100, 10));
		}

		if (1) 
		{
			mModel2 = rendFac->CreateAssimpModel(E_MAT_MODEL);
			mModel2->LoadModel("model/planet/planet.obj", R"({"dir":"model/planet/"})");
			float scale = SCALE_BASE * 0.2;
			mModel2->GetTransform()->SetScale(Eigen::Vector3f(scale, scale, scale));
			mModel2->GetTransform()->SetPosition(winCenter + Eigen::Vector3f(100, 100, 10) + Eigen::Vector3f(100, 100, -300));
		}
	}
}

void TestShadowMap::OnRender()
{
	if (mContext->RenderPipe()->BeginFrame()) {
		if (mModel1) mModel1->Update(mTimer->mDeltaTime);
		if (mModel2) mModel2->Update(mTimer->mDeltaTime);

		RenderOperationQueue opQueue;
		if (mCube0) mCube0->GenRenderOperation(opQueue);
		if (mCube1) mCube1->GenRenderOperation(opQueue);
		if (mModel1) mModel1->GenRenderOperation(opQueue);
		if (mModel2) mModel2->GenRenderOperation(opQueue);

		mContext->RenderPipe()->Render(opQueue, *mContext->SceneMng());
		mContext->RenderPipe()->EndFrame();
	}
}

auto reg = AppRegister<TestShadowMap>("test_shadow_map");