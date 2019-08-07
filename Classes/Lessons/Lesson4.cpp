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
	auto light2 = mRenderSys->AddDirectLight();
	light2->SetDirection(0, 0, 1);

	mModel = new AssimpModel(mRenderSys, mMove, "shader\\Lesson4.fx", "shader\\Lesson4.fx", [&](TMaterialPtr mat) {
		TFogExp fog;
		fog.SetColor(0.5, 0.5, 0.5);
		fog.SetExp(0.1);
		TMaterialBuilder builder(mat);
		builder.AddConstBuffer(mRenderSys->CreateConstBuffer(sizeof(TFogExp), &fog), MAKE_CBNAME(TFogExp), true);
	});
	gModelPath = "Spaceship\\"; mModel->LoadModel(MakeModelPath("Spaceship.fbx")); mMove->SetDefScale(0.01); mMove->SetPosition(0, 0, 0);
	//mModel->PlayAnim(0);
}

void Lesson4::OnRender()
{
	mModel->Update(mTimer.mDeltaTime);
	if (mRenderSys->BeginScene()) {
		mModel->Draw();
		mRenderSys->EndScene();
	}
}

//auto reg = AppRegister<Lesson4>("TAppLesson4: Fog");
