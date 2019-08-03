#include "Lesson8.h"

void Lesson8::OnPostInitDevice()
{
	mRenderSys->SetCamera(45, 10, 300);

	mRenderSys->SetSkyBox("images\\uffizi_cross.dds");
	mRenderSys->AddPostProcess(E_PASS_POSTPROCESS);

	mModel = new AssimpModel(mRenderSys, mMove, "shader\\Lesson3.3.fx", "shader\\Lesson3.3.fx");
	gModelPath = "Spaceship\\"; if (mModel) mModel->LoadModel(MakeModelPath("Spaceship.fbx")); mMove->SetDefScale(0.01);
}

void Lesson8::OnRender()
{
	if (mModel) mModel->Update(mTimer.mDeltaTime);

	if (mRenderSys->BeginScene()) {
		if (mModel) mModel->Draw();
		mRenderSys->EndScene();
	}
}

//auto reg = AppRegister<Lesson8>("Lesson8: PostProcess Bloom");