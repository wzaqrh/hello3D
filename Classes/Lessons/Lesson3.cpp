#include "Lesson3.h"

void Lesson3::OnPostInitDevice()
{
	auto light1 = mRenderSys->mPointLights[0];
	light1->SetSpecularPower(60);
	light1->SetSpecularColor(1, 1, 1, 1);
	light1->SetDiffuseColor(1, 1, 1, 0);
	light1->SetPosition(200, 0, -200);

	/*auto light2 = mRenderSys->AddDirectLight();
	light2->SetDiffuseColor(0, 0, 0, 0);
	light2->SetSpecularColor(1, 0, 0, 1);
	light2->SetDirection(0, 0, 1);*/

	mModel = new AssimpModel(mRenderSys, "shader\\Lesson3.1.fx", "shader\\Lesson3.1.fx");
	//mModel = new AssimpModel(mRenderSys, "shader\\Lesson3.fx", "shader\\Lesson3.fx");
	gModelPath = "Spaceship\\"; mModel->LoadModel(MakeModelPath("Spaceship.fbx")); mScale = 0.01;
	//mModel->PlayAnim(0);
}

void Lesson3::OnRender()
{
	mModel->Update(mTimer.mDeltaTime);
	mRenderSys->ApplyMaterial(mModel->mMaterial, GetWorldTransform());
	mModel->Draw();
}

auto reg = AppRegister<Lesson3>("TAppLesson3: Specular Light");