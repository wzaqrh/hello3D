#include "Lesson2.h"

void TAppLesson2::OnInitLight()
{
	auto light = mRenderSys->AddPointLight();
	light->SetDiffuseColor(1, 0, 0, 1);
	light->SetPosition(0, 0, -200);
}

void TAppLesson2::OnPostInitDevice()
{
	mModel = new AssimpModel(mRenderSys, mMove, "shader\\Lesson2.fx", "shader\\Lesson2.fx");
	gModelPath = "Spaceship\\"; mModel->LoadModel(MakeModelPath("Spaceship.fbx")); mMove->SetDefScale(0.01);
	//gModelPath = "Normal\\"; mModel->LoadModel(MakeModelPath("Deer.fbx")); mScale = 0.05;
	//mModel->PlayAnim(0);
}

void TAppLesson2::OnRender()
{
	mModel->Update(mTimer.mDeltaTime);
	if (mRenderSys->BeginScene()) {
		mModel->Draw();
		mRenderSys->EndScene();
	}
}

//auto reg = AppRegister<TAppLesson2>("TAppLesson2: Diffuse Light");