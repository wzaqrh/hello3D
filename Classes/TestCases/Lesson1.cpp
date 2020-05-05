#include "Lesson1.h"
#include "TInterfaceType.h"

void TAppLesson1::OnPostInitDevice()
{
#if 0
	std::vector<D3D11_INPUT_ELEMENT_DESC> layouts =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 3 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, 6 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 9 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 11 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 15 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 2, DXGI_FORMAT_R32G32B32_FLOAT, 0, 19 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	mModel = new AssimpModel(mRenderSys, mMove, MAKE_MAT_NAME("Lesson1"), layouts);
#else
	mModel = new TAssimpModel(mRenderSys, mMove, E_MAT_MODEL);
#endif
	//gModelPath = "Spaceship\\"; mModel->LoadModel(MakeModelPath("Spaceship.fbx")); mMove->SetDefScale(0.01); mModel->PlayAnim(0);
	//gModelPath = "Normal\\"; mModel->LoadModel(MakeModelPath("Deer.fbx")); 
	//gModelPath = "handgun\\"; mModel->LoadModel(MakeModelPath("handgun.fbx")); mMove->SetDefScale(0.01);
	gModelPath = "Male03\\"; mModel->LoadModel(MakeModelPath("Male02.FBX")); mMove->SetDefScale(0.07); mMove->SetPosition(0, -5, 0);
}

void TAppLesson1::OnRender()
{
	mModel->Update(mTimer.mDeltaTime);
	if (mRenderSys->BeginScene()) {
		mModel->Draw();
		mRenderSys->EndScene();
	}
}

//auto reg = AppRegister<TAppLesson1>("TAppLesson1: Load Model");
