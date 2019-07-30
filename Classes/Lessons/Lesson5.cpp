#include "Lesson5.h"

/********** Lesson5 **********/
void Lesson5::OnPostInitDevice()
{
	auto light1 = mRenderSys->mPointLights[0];
	light1->SetPosition(20, 0, -20);
	light1->SetAttenuation(1.0, 0.01, 0);
	light1->SetSpecularPower(60);


	mModel = new AssimpModel(mRenderSys, "shader\\Lesson3.3.fx", "shader\\Lesson3.3.fx");
	gModelPath = "Spaceship\\"; mModel->LoadModel(MakeModelPath("Spaceship.fbx")); mScale = 0.01; mPosition = XMFLOAT3(0, 0, 0);
	

	mRendTexture = mRenderSys->CreateRenderTexture(mRenderSys->mScreenWidth, mRenderSys->mScreenHeight);


	mSprite = std::make_shared<TSprite>(mRenderSys, "shader\\Sprite.fx", "shader\\Sprite.fx");
	//mSprite->SetTexture(mRenderSys->CreateTexture("image\\smile.png"));
	mSprite->SetTexture(mRendTexture->mRenderTargetSRV);
	mSprite->SetPosition(0, 0, 0);
	mSprite->SetSize(5,5);


	mLayerColor = std::make_shared<TSprite>(mRenderSys, "shader\\LayerColor.fx", "shader\\LayerColor.fx");
	mLayerColor->SetPosition(-5, -5, 0);
	mLayerColor->SetSize(5, 5);
}

void Lesson5::OnRender()
{
	/*mRenderSys->ClearRenderTexture(mRendTexture, XMFLOAT4(0,0,0,0));
	mRenderSys->SetRenderTarget(mRendTexture);
	{
		mModel->Update(mTimer.mDeltaTime);
		mRenderSys->ApplyMaterial(mModel->mMaterial, GetWorldTransform());
		mModel->Draw();
	}
	mRenderSys->SetRenderTarget(nullptr);
	
	mSprite->Draw();*/

	mLayerColor->Draw();
}

//auto reg = AppRegister<Lesson5>("TAppLesson5: RenderTarget");