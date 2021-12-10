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
0��������� ƽ�й� �۲쵽kenney��ʾ�����Ͻǰ���, ��ģ��
1��������� ƽ�й� �۲쵽ʯͷ��ʾ�����Ͻǰ���, ��ģ��
2��͸����� ƽ�й� �۲쵽ʯͷ��ʾ�����Ͻǰ���, ��ģ��
3��͸����� ��� �۲쵽ʯͷ��ʾ�����Ͻǰ���, ��ģ��
*/

/********** TestRT **********/
void TestRT::OnPostInitDevice()
{
	if (mCaseIndex == 0)
	{
		auto light = mScneMng->AddDirectLight();

		constexpr unsigned cameraMask2 = 0x01;
		auto camera2 = mScneMng->AddOthogonalCamera(test1::cam::Eye(mWinCenter), test1::cam::NearFarFov());
		{
			camera2->SetDepth(1);
			camera2->SetCameraMask(cameraMask2);

			mSprite2 = mRendFac->CreateSprite(test1::res::dds::Kenny());
			mSprite2->SetCameraMask(cameraMask2);
			mSprite2->SetSize(mHalfSize * 2);
			mSprite2->SetAnchor(mir::math::vec::anchor::Center());
		}

		constexpr unsigned cameraMask1 = 0x02;
		{
			auto camera1 = mScneMng->AddOthogonalCamera(test1::cam::Eye(mWinCenter), test1::cam::NearFarFov());
			camera1->SetDepth(2);
			camera1->SetCameraMask(cameraMask1);

			mFrameBuffer = camera2->SetOutput(0.25);
			DEBUG_SET_PRIV_DATA(mFrameBuffer, "camera2 output");

			mFBSprite = mContext->RenderableFac()->CreateSprite();
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
		if (mCaseIndex == 1) camera2 = mScneMng->AddOthogonalCamera(mWinCenter + Eigen::Vector3f(0, 0, -100), test1::cam::NearFarFov());
		else camera2 = mScneMng->AddPerspectiveCamera(mWinCenter + Eigen::Vector3f(0, 0, -100), test1::cam::NearFarFov());

		{
			camera2->SetDepth(1);
			camera2->SetCameraMask(cameraMask2);
			camera2->SetSkyBox(mRendFac->CreateSkybox(test1::res::Sky()));

			mModel = mRendFac->CreateAssimpModel(E_MAT_MODEL);
			mModel->SetCameraMask(cameraMask2);
			mModel->LoadModel(test1::res::model_rock::Path(), test1::res::model_rock::Rd());
			mTransform = mModel->GetTransform();
			mTransform->SetScale(test1::res::model_rock::Scale());
			mTransform->SetPosition(mWinCenter);

			if (camera2->GetType() == kCameraOthogonal)
				mTransform->SetScale(mTransform->GetScale() * 50);
			else
				mTransform->SetScale(mTransform->GetScale() * 5);
		}

		constexpr unsigned cameraMask1 = 0x02;
		//if (0)
		{
			auto camera1 = mScneMng->AddOthogonalCamera(test1::cam::Eye(mWinCenter), test1::cam::NearFarFov());
			camera1->SetDepth(2);
			camera1->SetCameraMask(cameraMask1);

			mFrameBuffer = mResMng->CreateFrameBufferSync(mHalfSize.cast<int>().head<2>() / 2, 
				MakeResFormats(kFormatR8G8B8A8UNorm, kFormatD24UNormS8UInt));
			camera2->SetOutput(mFrameBuffer);
			DEBUG_SET_PRIV_DATA(mFrameBuffer, "camera2 output");

			mFBSprite = mRendFac->CreateSprite();
			mFBSprite->SetCameraMask(cameraMask1);
			mFBSprite->SetTexture(mFrameBuffer->GetAttachColorTexture(0));
			mFBSprite->SetPosition(mWinCenter);
			mFBSprite->SetSize(mHalfSize);
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