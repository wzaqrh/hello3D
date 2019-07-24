#include "Lesson4.h"

/********** TFogExp **********/
TFogExp::TFogExp()
{
	SetColor(0.5, 0.5, 0.5);
	SetExp(1);
}

void TFogExp::SetColor(float r, float g, float b)
{
	mFogColorExp = XMFLOAT4(r, g, b, mFogColorExp.w);
}

void TFogExp::SetExp(float exp)
{
	mFogColorExp.w = exp;
}

/********** Lesson4 **********/
void Lesson4::OnPostInitDevice()
{
	auto light1 = mRenderSys->mPointLights[0];
	light1->SetPosition(20, 0, -20);
	light1->SetAttenuation(1.0, 0.1, 0);
	light1->SetSpecularPower(60);
	light1->SetSpecularColor(0, 0, 0, 0);
	light1->SetDiffuseColor(0, 0, 0, 0);

	auto light2 = mRenderSys->AddDirectLight();
	light2->SetDirection(0, 0, 1);

	mModel = new AssimpModel(mRenderSys, "shader\\Lesson4.fx", "shader\\Lesson4.fx");
	gModelPath = "Spaceship\\"; mModel->LoadModel(MakeModelPath("Spaceship.fbx")); mScale = 0.01; mPosition = XMFLOAT3(0, 0, 0);
	//mModel->PlayAnim(0);

	TFogExp fog;
	fog.SetColor(0.5,0.5,0.5);
	fog.SetExp(0.1);
	auto buffer = mModel->mMaterial->AddConstBuffer(mRenderSys->CreateConstBuffer(sizeof(TFogExp)));
	mRenderSys->UpdateConstBuffer(buffer, &fog);
}

void Lesson4::OnRender()
{
	mModel->Update(mTimer.mDeltaTime);
	mRenderSys->ApplyMaterial(mModel->mMaterial, GetWorldTransform());
	mModel->Draw();
}

//auto reg = AppRegister<Lesson4>("TAppLesson4: Fog");
