#include "Lesson2.h"

void TAppLesson2::OnPostInitDevice()
{
	mRenderSys->mDefLight->SetDiffuseColor(1, 0, 0, 1);
	mRenderSys->mDefLight->SetPosition(0, 0, -200);

	mModel = new AssimpModel(mRenderSys, "shader\\Lesson2.fx", "shader\\Lesson2.fx");
	gModelPath = "Spaceship\\"; mModel->LoadModel(MakeModelPath("Spaceship.fbx")); mScale = 0.01;
	//gModelPath = "Normal\\"; mModel->LoadModel(MakeModelPath("Deer.fbx")); mScale = 0.05;
	//mModel->PlayAnim(0);
}

void TAppLesson2::OnRender()
{
	mModel->Update(mTimer.mDeltaTime);
	mRenderSys->ApplyMaterial(mModel->mMaterial, GetWorldTransform());
	mModel->Draw();
}

//auto reg = AppRegister<TAppLesson2>("TAppLesson2: Diffuse Light");