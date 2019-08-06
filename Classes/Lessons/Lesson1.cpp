#include "Lesson1.h"
#include "TInterfaceType.h"

void TAppLesson1::OnPostInitDevice()
{
	mModel = new AssimpModel(mRenderSys, mMove, "shader\\Lesson1.fx", "shader\\Lesson1.fx");
	gModelPath = "Spaceship\\"; mModel->LoadModel(MakeModelPath("Spaceship.fbx")); mMove->SetDefScale(0.01);
	//gModelPath = "Normal\\"; mModel->LoadModel(MakeModelPath("Deer.fbx"));
	mModel->PlayAnim(0);
}

void TAppLesson1::OnRender()
{
	mModel->Update(mTimer.mDeltaTime);
#ifdef USE_RENDER_OP
	//mRenderSys->SetWorldTransform(GetWorldTransform());
#else
	mRenderSys->ApplyMaterial(mModel->mMaterial, GetWorldTransform());
#endif
	mModel->Draw();
}

//auto reg = AppRegister<TAppLesson1>("TAppLesson1: Load Model");
