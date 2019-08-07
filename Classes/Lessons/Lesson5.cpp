#include "Lesson5.h"
#include "TInterfaceType.h"

/********** Lesson5 **********/
void Lesson5::OnInitLight()
{
	auto light1 = mRenderSys->AddPointLight();
	light1->SetPosition(20, 0, -20);
	light1->SetAttenuation(1.0, 0.01, 0);
	light1->SetSpecularPower(60);
}

void Lesson5::OnPostInitDevice()
{
	mModel = new AssimpModel(mRenderSys, mMove, "shader\\Lesson3.3.fx", "shader\\Lesson3.3.fx");
	gModelPath = "Spaceship\\"; mModel->LoadModel(MakeModelPath("Spaceship.fbx")); mMove->SetDefScale(0.01); mMove->SetPosition(0, 0, 0);
	

	mRendTexture = mRenderSys->CreateRenderTexture(mRenderSys->mScreenWidth, mRenderSys->mScreenHeight);


	mSprite = std::make_shared<TSprite>(mRenderSys, "shader\\Sprite.fx", "shader\\Sprite.fx");
	//mSprite->SetTexture(mRenderSys->CreateTexture("image\\smile.png"));
	mSprite->SetTexture(mRendTexture->GetRenderTargetSRV());
	mSprite->SetPosition(0, 0, 0);
	mSprite->SetSize(5,5);


	mLayerColor = std::make_shared<TSprite>(mRenderSys, "shader\\LayerColor.fx", "shader\\LayerColor.fx");
	mLayerColor->SetPosition(-5, -5, 0);
	mLayerColor->SetSize(5, 5);
}

void Lesson5::OnRender()
{
	mRenderSys->ClearRenderTexture(mRendTexture, XMFLOAT4(0,0,0,0));
	mRenderSys->SetRenderTarget(mRendTexture);
	{
		mModel->Update(mTimer.mDeltaTime);
#ifdef USE_RENDER_OP
		//mRenderSys->SetWorldTransform(GetWorldTransform());
#else
		mRenderSys->ApplyMaterial(mModel->mMaterial, GetWorldTransform());
#endif
		mModel->Draw();
	}
	mRenderSys->SetRenderTarget(nullptr);
	
	mSprite->Draw();

	mLayerColor->Draw();
}

//auto reg = AppRegister<Lesson5>("TAppLesson5: RenderTarget");