#include "Lesson3.h"

void Lesson3::OnPostInitDevice()
{
	auto light1 = mRenderSys->mPointLights[0];
	light1->SetPosition(20, 0, -20);
	light1->SetAttenuation(1.0, 0.1, 0);
	light1->SetSpecularPower(60);
	light1->SetSpecularColor(0, 0, 0, 0);
	light1->SetDiffuseColor(0, 0, 0, 0);

	mRenderSys->SetSkyBox("images\\uffizi_cross.dds");

	//auto light2 = mRenderSys->AddSpotLight();
	////light2->SetDiffuseColor(0, 0, 0, 0);
	////light2->SetSpecularColor(1, 0, 0, 1);
	//light2->SetDirection(0, 0, 1);
	//light2->SetPosition(0, 0, -10);
	//light2->SetAttenuation(1.0, 0.1, 0);
	//light2->SetAngle(3.14*15/180);

	auto light2 = mRenderSys->AddDirectLight();
	light2->SetDirection(0, 0, 1);

	mModel = new AssimpModel(mRenderSys, mMove, "shader\\Lesson3.3.fx", "shader\\Lesson3.3.fx");
	//mModel = new AssimpModel(mRenderSys, "shader\\Lesson3.2.fx", "shader\\Lesson3.2.fx");
	//mModel = new AssimpModel(mRenderSys, "shader\\Lesson3.1.fx", "shader\\Lesson3.1.fx");
	//mModel = new AssimpModel(mRenderSys, "shader\\Lesson3.fx", "shader\\Lesson3.fx");
	gModelPath = "Spaceship\\"; mModel->LoadModel(MakeModelPath("Spaceship.fbx")); mMove->SetDefScale(0.01);
	//gModelPath = "Male03\\"; mModel->LoadModel(MakeModelPath("Male02.FBX")); mScale = 0.03; mPosition = XMFLOAT3(0, -5, 0);
	//mModel->PlayAnim(0);
}

void Lesson3::OnRender()
{
	mRenderSys->RenderSkyBox();

	mModel->Update(mTimer.mDeltaTime);
#ifdef USE_RENDER_OP
	//mRenderSys->SetWorldTransform(GetWorldTransform());
#else
	mRenderSys->ApplyMaterial(mModel->mMaterial, GetWorldTransform());
#endif
	mModel->Draw();
}

auto reg = AppRegister<Lesson3>("TAppLesson3: Specular Light + skybox");