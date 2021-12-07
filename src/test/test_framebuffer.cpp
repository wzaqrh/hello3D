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
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
	virtual void OnInitLight() override;
private:
	AssimpModelPtr mModel;
	IFrameBufferPtr mFrameBuffer;
	SpritePtr mSprite2, mFBSprite;
};

/********** TestRT **********/
void TestRT::OnInitLight()
{

}

void TestRT::OnPostInitDevice()
{
	auto sceneMng = mContext->SceneMng();
	auto rendFac = mContext->RenderableFac();
	auto resMng = mContext->ResourceMng();

	auto winSize = mContext->ResourceMng()->WinSize();
	if (mCaseIndex == 1)
	{
		auto light1 = sceneMng->AddPointLight();
		light1->SetPosition(Eigen::Vector3f(20, 0, -20));
		light1->SetAttenuation(0.01);

		mModel = rendFac->CreateAssimpModel(E_MAT_MODEL);
		mModel->LoadModel("model/Spaceship/Spaceship.fbx");
		mMoveDefScale = 0.01;
		mTransform = mModel->GetTransform();
		mTransform->SetScale(Eigen::Vector3f(mMoveDefScale, mMoveDefScale, mMoveDefScale));
		mTransform->SetPosition(Eigen::Vector3f(0, 0, 0));

		mFrameBuffer = resMng->CreateFrameBufferSync(winSize, kFormatR8G8B8A8UNorm);

		mFBSprite = rendFac->CreateSprite();
		mFBSprite->SetTexture(mFrameBuffer->GetAttachColorTexture(0));
		mFBSprite->SetPosition(Eigen::Vector3f(0, 0, 0));
		mFBSprite->SetSize(Eigen::Vector2f(5, 5));
	}
	else 
	{
		sceneMng->RemoveAllLights();
		sceneMng->RemoveAllCameras();
		auto light = sceneMng->AddDirectLight();
		
		constexpr unsigned cameraMask2 = 0x01;
		auto camera2 = sceneMng->AddOthogonalCamera(Eigen::Vector3f(0, 0, -30), 300);
		{
			camera2->GetTransform()->SetPosition(Eigen::Vector3f(0, 0, 0));
			camera2->SetDepth(1);
			camera2->SetCameraMask(cameraMask2);

			mSprite2 = mContext->RenderableFac()->CreateSprite("model/theyKilledKenny.dds");
			mSprite2->SetCameraMask(cameraMask2);
			mSprite2->SetSize(winSize.cast<float>());
		}

		constexpr unsigned cameraMask1 = 0x02;
		//if (0)
		{
			auto camera1 = sceneMng->AddOthogonalCamera(Eigen::Vector3f(0, 0, -10), 100);
			camera1->SetDepth(2);
			camera1->SetCameraMask(cameraMask1);
			auto winCenter = winSize / 2;

			mFrameBuffer = camera2->FetchOutput();
			DEBUG_SET_PRIV_DATA(mFrameBuffer, "camera2 output");

			mFBSprite = mContext->RenderableFac()->CreateSprite();
			mFBSprite->SetTexture(mFrameBuffer->GetAttachColorTexture(0));
			mFBSprite->SetPosition(Eigen::Vector3f(0, winCenter.y(), 0));
			mFBSprite->SetSize(winSize.cast<float>() / 2);
			mFBSprite->SetCameraMask(cameraMask1);
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