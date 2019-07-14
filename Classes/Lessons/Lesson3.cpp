#include "Lesson3.h"

void Lesson3::OnPostInitDevice()
{
	auto light1 = mRenderSys->mPointLights[0];
	light1->SetPosition(20, 0, -20);
	light1->SetAttenuation(1.0, 0.1, 0);
	light1->SetSpecularPower(60);
	/*light1->SetSpecularColor(0, 0, 0, 0);
	light1->SetDiffuseColor(0, 0, 0, 0);*/

	auto light2 = mRenderSys->AddSpotLight();
	//light2->SetDiffuseColor(0, 0, 0, 0);
	//light2->SetSpecularColor(1, 0, 0, 1);
	light2->SetDirection(0, 0, 1);
	light2->SetPosition(0, 0, -10);
	light2->SetAttenuation(1.0, 0.1, 0);
	light2->SetAngle(3.14*15/180);

	/*auto light2 = mRenderSys->AddDirectLight();
	light2->SetDiffuseColor(0, 0, 0, 0);
	light2->SetSpecularColor(1, 0, 0, 1);
	light2->SetDirection(0, 0, 1);*/

	mModel = new AssimpModel(mRenderSys, "shader\\Lesson3.3.fx", "shader\\Lesson3.3.fx");
	//mModel = new AssimpModel(mRenderSys, "shader\\Lesson3.2.fx", "shader\\Lesson3.2.fx");
	//mModel = new AssimpModel(mRenderSys, "shader\\Lesson3.1.fx", "shader\\Lesson3.1.fx");
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