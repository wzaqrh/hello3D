#include "Lesson3.h"

void Lesson3::OnPostInitDevice()
{
	mRenderSys->mDefLight->SetSpecularPower(60);
	mRenderSys->mDefLight->SetSpecularColor(1, 1, 1, 1);
	mRenderSys->mDefLight->SetDiffuseColor(1, 1, 1, 1);
	//mRenderSys->mDefLight->SetPosition(mRenderSys->mScreenWidth/2, mRenderSys->mScreenHeight/2, -1500);
	mRenderSys->mDefLight->SetPosition(-2000, 0, 0);

	mModel = new AssimpModel(mRenderSys, "shader\\Lesson3.fx", "shader\\Lesson3.fx");
	//gModelPath = "handgun\\"; mModel->LoadModel(MakeModelPath("handgun.fbx")); mScale = 0.03;
	gModelPath = "Spaceship\\"; mModel->LoadModel(MakeModelPath("Spaceship.fbx")); mScale = 0.01;
	//gModelPath = "Normal\\"; mModel->LoadModel(MakeModelPath("Deer.fbx")); mScale = 0.05;
	//mModel->PlayAnim(0);
}

void Lesson3::OnRender()
{
	mModel->Update(mTimer.mDeltaTime);
	mRenderSys->ApplyMaterial(mModel->mMaterial, GetWorldTransform());
	mModel->Draw();
}

auto reg = AppRegister<Lesson3>("TAppLesson3: Specular Light");