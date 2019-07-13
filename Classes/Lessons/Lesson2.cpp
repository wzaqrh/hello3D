#include "Lesson2.h"

void TAppLesson2::OnPostInitDevice()
{
	mModel = new AssimpModel(mRenderSys, "shader\\Lesson2.fx", "shader\\Lesson2.fx");
	//gModelPath = "Spaceship\\"; mModel->LoadModel(MakeModelPath("Spaceship.fbx"));
	gModelPath = "Normal\\"; mModel->LoadModel(MakeModelPath("Deer.fbx"));
	mModel->PlayAnim(0);

	mScale = 0.03;
}

void TAppLesson2::OnRender()
{
	mModel->Update(mTimer.mDeltaTime);
	mRenderSys->ApplyMaterial(mModel->mMaterial, GetWorldTransform());
	mModel->Draw();
}

auto reg = AppRegister<TAppLesson2>("TAppLesson2");