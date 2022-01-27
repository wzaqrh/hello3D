#include "test/test_case.h"
#include "test/app.h"
#include "core/scene/scene_manager.h"
#include "core/renderable/assimp_model.h"
#include "core/renderable/sprite.h"
#include "core/base/transform.h"

using namespace mir;
using namespace mir::rend;

class TestRT : public App
{
protected:
	void OnRender() override;
	CoTask<void> OnPostInitDevice() override;
	void OnInitLight() override {}
	void OnInitCamera() override {}
private:
	AssimpModelPtr mModel;
	IFrameBufferPtr mFrameBuffer;
	SpritePtr mSprite2, mFBSprite;
};
/*mCaseIndex
0：正交相机 平行光 观察到kenney显示在左上角半区, 且模糊
1：正交相机 平行光 观察到石头显示在右上角半区, 且模糊
2：透视相机 平行光 观察到石头显示在右上角半区, 且模糊
3：透视相机 点光 观察到石头显示在右上角半区, 且模糊
*/

/********** TestRT **********/
CoTask<void> TestRT::OnPostInitDevice()
{
	if (mCaseIndex == 0)
	{
		auto light = mScneMng->AddDirectLight();

		constexpr unsigned cameraMask2 = 0x01;
		auto camera2 = mScneMng->AddOthogonalCamera(test1::cam::Eye(mWinCenter));
		{
			camera2->SetDepth(1);
			camera2->SetCullingMask(cameraMask2);

			mSprite2 = CoAwait mRendFac->CreateSprite(test1::res::dds::Kenny());
			mSprite2->SetCameraMask(cameraMask2);
			mSprite2->SetSize(mHalfSize * 2);
			mSprite2->SetAnchor(mir::math::vec::anchor::Center());
		}

		constexpr unsigned cameraMask1 = 0x02;
		{
			auto camera1 = mScneMng->AddOthogonalCamera(test1::cam::Eye(mWinCenter));
			camera1->SetDepth(2);
			camera1->SetCullingMask(cameraMask1);

			mFrameBuffer = camera2->SetOutput(0.25);
			DEBUG_SET_PRIV_DATA(mFrameBuffer, "camera2 output");

			mFBSprite = CoAwait mContext->RenderableFac()->CreateSprite();
			mFBSprite->SetCameraMask(cameraMask1);
			mFBSprite->SetTexture(mFrameBuffer->GetAttachColorTexture(0));
			mFBSprite->SetPosition(Eigen::Vector3f(-mHalfSize.x(), 0, 0));
			mFBSprite->SetSize(mHalfSize);
		}
	}
	else if (mCaseIndex == 1 || mCaseIndex == 2 || mCaseIndex == 3)
	{
		if (mCaseIndex == 1 || mCaseIndex == 2) {
			auto light = mScneMng->AddDirectLight();
			light->SetDirection(test1::vec::DirLight());
		}
		else {
			auto light = mScneMng->AddPointLight();
			light->SetPosition(test1::cam::Eye(mWinCenter) + test1::vec::PosLight());
			light->SetAttenuation(0.0001);
		}

		constexpr unsigned cameraMask2 = 0x01;
		CameraPtr camera2;
		if (mCaseIndex == 1) camera2 = mScneMng->AddOthogonalCamera(mWinCenter + Eigen::Vector3f(0, 0, -100));
		else camera2 = mScneMng->AddPerspectiveCamera(mWinCenter + Eigen::Vector3f(0, 0, -100));

		{
			camera2->SetDepth(1);
			camera2->SetCullingMask(cameraMask2);
			camera2->SetSkyBox(CoAwait mRendFac->CreateSkybox(test1::res::Sky()));

			mModel = CoAwait mRendFac->CreateAssimpModel(MAT_MODEL);
			mModel->SetCameraMask(cameraMask2);
			mTransform = CoAwait test1::res::model().Init("rock", mModel);

			if (camera2->GetType() == kCameraOthogonal)
				mTransform->SetScale(mTransform->GetScale() * 50);
			else
				mTransform->SetScale(mTransform->GetScale() * 5);
		}

		constexpr unsigned cameraMask1 = 0x02;
		//if (0)
		{
			auto camera1 = mScneMng->AddOthogonalCamera(test1::cam::Eye(mWinCenter));
			camera1->SetDepth(2);
			camera1->SetCullingMask(cameraMask1);

			mFrameBuffer = mResMng->CreateFrameBufferSync(mHalfSize.cast<int>().head<2>() / 2, 
				MakeResFormats(kFormatR8G8B8A8UNorm, kFormatD24UNormS8UInt));
			camera2->SetOutput(mFrameBuffer);
			DEBUG_SET_PRIV_DATA(mFrameBuffer, "camera2 output");

			mFBSprite = CoAwait mRendFac->CreateSprite();
			mFBSprite->SetCameraMask(cameraMask1);
			mFBSprite->SetTexture(mFrameBuffer->GetAttachColorTexture(0));
			mFBSprite->SetPosition(mWinCenter);
			mFBSprite->SetSize(mHalfSize);
		}
	}
	CoReturnVoid;
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