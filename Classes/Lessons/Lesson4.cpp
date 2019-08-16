#include "Lesson4.h"

/********** Lesson4 **********/
void Lesson4::OnPostInitDevice()
{
	auto light2 = mRenderSys->AddDirectLight();
	light2->SetDirection(0, 0, 1);

	mModel = new AssimpModel(mRenderSys, mMove, "shader\\Lesson4.fx", "shader\\Lesson4.fx", [&](TMaterialPtr mat) {
#if 0
		TFogExp fog;
		fog.SetColor(0.5, 0.5, 0.5);
		fog.SetExp(0.1);
		TMaterialBuilder builder(mat);
		builder.AddConstBuffer(mRenderSys->CreateConstBuffer(MAKE_CBDESC(TFogExp), &fog), MAKE_CBNAME(TFogExp), true);
#endif
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