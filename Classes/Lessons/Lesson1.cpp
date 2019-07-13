#include "Lesson1.h"
#include "TRenderSystem.h"

void TAppLesson1::OnPostInitDevice()
{
	mModel = new AssimpModel(mRenderSys, "shader\\Lesson1.fx", "shader\\Lesson1.fx");
	gModelPath = "Spaceship\\"; mModel->LoadModel(MakeModelPath("Spaceship.fbx"));
	//gModelPath = "Normal\\"; mModel->LoadModel(MakeModelPath("Deer.fbx"));
	mModel->PlayAnim(0);
}

void TAppLesson1::OnRender()
{
	mModel->Update(mTimer.mDeltaTime);
	mRenderSys->ApplyMaterial(mModel->mMaterial, GetWorldTransform());
	mModel->Draw();
}

//auto reg = AppRegister<TAppLesson1>("TAppLesson1");
