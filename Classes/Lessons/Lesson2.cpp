#include "Lesson2.h"

void TAppLesson2::OnPostInitDevice()
{
	mRenderSys->mPointLights[0]->SetDiffuseColor(1, 0, 0, 1);
	mRenderSys->mPointLights[0]->SetPosition(0, 0, -200);

	mModel = new AssimpModel(mRenderSys, mMove, "shader\\Lesson2.fx", "shader\\Lesson2.fx");
	gModelPath = "Spaceship\\"; mModel->LoadModel(MakeModelPath("Spaceship.fbx")); mMove->SetDefScale(0.01);
	//gModelPath = "Normal\\"; mModel->LoadModel(MakeModelPath("Deer.fbx")); mScale = 0.05;
	//mModel->PlayAnim(0);
}

void TAppLesson2::OnRender()
{
	mModel->Update(mTimer.mDeltaTime);
#ifdef USE_RENDER_OP
	//mRenderSys->SetWorldTransform(GetWorldTransform());
#else
	mRenderSys->ApplyMaterial(mModel->mMaterial, GetWorldTransform());
#endif
	mModel->Draw();
}

//auto reg = AppRegister<TAppLesson2>("TAppLesson2: Diffuse Light");