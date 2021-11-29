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
	AssimpModelPtr mModel = nullptr;
	IFrameBufferPtr mRendTexture = nullptr;
	SpritePtr mSprite, mLayerColor;
};

/********** TestRT **********/
void TestRT::OnInitLight()
{
	auto light1 = mContext->SceneMng()->AddPointLight();
	light1->SetPosition(Eigen::Vector3f(20, 0, -20));
	light1->SetAttenuation(0.01);
	//light1->SetSpecular(60);
}

void TestRT::OnPostInitDevice()
{
	mModel = mContext->RenderableFac()->CreateAssimpModel(mTransform, E_MAT_MODEL);
	mModel->LoadModel("model/Spaceship/Spaceship.fbx"); 
	mMoveDefScale = 0.01;
	mTransform->SetScale(Eigen::Vector3f(mMoveDefScale, mMoveDefScale, mMoveDefScale));
	mTransform->SetPosition(Eigen::Vector3f(0, 0, 0));
	
	mRendTexture = mContext->ResourceMng()->CreateFrameBufferSync(
		mContext->ResourceMng()->WinSize(), 
		kFormatR32G32B32A32Float);

	mSprite = mContext->RenderableFac()->CreateSprite();
	mSprite->SetTexture(mRendTexture->GetAttachColorTexture(0));
	mSprite->SetPosition(Eigen::Vector3f(0, 0, 0));
	mSprite->SetSize(Eigen::Vector2f(5, 5));

	/*mLayerColor = std::make_shared<TSprite>(mRenderSys, E_MAT_LAYERCOLOR);
	mLayerColor->SetPosition(-5, -5, 0);
	mLayerColor->SetSize(5, 5);*/
}

//#define USE_RENDER_TARGET
void TestRT::OnRender()
{
	mModel->Update(mTimer->mDeltaTime);
	mContext->RenderPipe()->Draw(*mModel, *mContext->SceneMng());
}

auto reg = AppRegister<TestRT>("test_framebuffer");