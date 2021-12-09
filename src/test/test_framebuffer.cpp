#include "test/test_case.h"
#include "test/app.h"
#include "core/scene/scene_manager.h"
#include "core/renderable/assimp_model.h"
#include "core/renderable/sprite.h"
#include "core/base/transform.h"

using namespace mir;

class TestRT : public App
{
protected:
	void OnRender() override;
	void OnPostInitDevice() override;
	void OnInitLight() override {}
	void OnInitCamera() override {}
private:
	AssimpModelPtr mModel;
	IFrameBufferPtr mFrameBuffer;
	SpritePtr mSprite2, mFBSprite;
};
/*mCaseIndex
0：正交相机 平行光 观察到kenney显示在左上角半区
0：透视相机 点光 观察到石头显示在右上角半区, 且模糊
*/

/********** TestRT **********/
void TestRT::OnPostInitDevice()
{
	auto sceneMng = mContext->SceneMng();
	auto rendFac = mContext->RenderableFac();
	auto resMng = mContext->ResourceMng();
	auto halfSize = mContext->ResourceMng()->WinSize() / 2;
	auto winCenter = Eigen::Vector3f(halfSize.x(), halfSize.y(), 0);

	auto winSize = mContext->ResourceMng()->WinSize();
	if (mCaseIndex == 1)
	{
		auto light = sceneMng->AddPointLight();
		light->SetPosition(winCenter + test1::cam::Eye() + test1::vec::PosLight());
		light->SetAttenuation(0.0001);

		constexpr unsigned cameraMask2 = 0x01;
		auto camera2 = sceneMng->AddPerspectiveCamera(winCenter + test1::cam::Eye(), test1::cam::NearFarFov());
		{
			camera2->SetDepth(1);
			camera2->SetCameraMask(cameraMask2);

			mModel = rendFac->CreateAssimpModel(E_MAT_MODEL);
			mModel->SetCameraMask(cameraMask2);
			mModel->LoadModel(test1::res::model_rock::Path(), test1::res::model_rock::Rd());
			mTransform = mModel->GetTransform();
			mTransform->SetScale(test1::res::model_rock::Scale());
			mTransform->SetPosition(winCenter + Eigen::Vector3f::Zero());
		}

		constexpr unsigned cameraMask1 = 0x02;
		//if (0)
		{
			auto camera1 = sceneMng->AddOthogonalCamera(winCenter + test1::cam::Eye(), test1::cam::NearFarFov());
			camera1->SetDepth(2);
			camera1->SetCameraMask(cameraMask1);

			mFrameBuffer = resMng->CreateFrameBufferSync(winSize / 4, MakeResFormats(kFormatR8G8B8A8UNorm, kFormatD24UNormS8UInt));
			camera2->SetOutput(mFrameBuffer);
			DEBUG_SET_PRIV_DATA(mFrameBuffer, "camera2 output");

			mFBSprite = rendFac->CreateSprite();
			mFBSprite->SetCameraMask(cameraMask1);
			mFBSprite->SetTexture(mFrameBuffer->GetAttachColorTexture(0));
			mFBSprite->SetPosition(winCenter);
			mFBSprite->SetSize(winSize.cast<float>() / 2);
		}
	}
	else 
	{
		auto light = sceneMng->AddDirectLight();
		
		constexpr unsigned cameraMask2 = 0x01;
		auto camera2 = sceneMng->AddOthogonalCamera(winCenter + test1::cam::Eye(), test1::cam::NearFarFov());
		{
			camera2->SetDepth(1);
			camera2->SetCameraMask(cameraMask2);

			mSprite2 = rendFac->CreateSprite(test1::res::dds::Kenny());
			mSprite2->SetCameraMask(cameraMask2);
			mSprite2->SetSize(winSize.cast<float>());
		}

		constexpr unsigned cameraMask1 = 0x02;
		//if (0)
		{
			auto camera1 = sceneMng->AddOthogonalCamera(winCenter + test1::cam::Eye(), test1::cam::NearFarFov());
			camera1->SetDepth(2);
			camera1->SetCameraMask(cameraMask1);

			mFrameBuffer = camera2->SetOutput(0.25);
			DEBUG_SET_PRIV_DATA(mFrameBuffer, "camera2 output");

			mFBSprite = mContext->RenderableFac()->CreateSprite();
			mFBSprite->SetCameraMask(cameraMask1);
			mFBSprite->SetTexture(mFrameBuffer->GetAttachColorTexture(0));
			mFBSprite->SetPosition(Eigen::Vector3f(0, halfSize.y(), 0));
			mFBSprite->SetSize(winSize.cast<float>() / 2);
		}
	}
}

//#define USE_RENDER_TARGET
void TestRT::OnRender()
{
	if (mContext->RenderPipe()->BeginFrame()) {
		if (mModel) mModel->Update(mTimer->mDeltaTime);

		RenderOperationQueue opQueue;
		if (mModel) mModel->GenRenderOperation(opQueue);
		if (mSprite2) mSprite2->GenRenderOperation(opQueue);
		if (mFBSprite) mFBSprite->GenRenderOperation(opQueue);

		mContext->RenderPipe()->Render(opQueue, *mContext->SceneMng());
		mContext->RenderPipe()->EndFrame();
	}
}

auto reg = AppRegister<TestRT>("test_framebuffer");