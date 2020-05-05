#include "Lesson8.h"

void Lesson8::OnPostInitDevice()
{
	mRenderSys->SetPerspectiveCamera(45, 10, 300);

	mRenderSys->SetSkyBox("images\\uffizi_cross.dds");
	mRenderSys->AddPostProcess(E_PASS_POSTPROCESS);

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
	mModel = new TAssimpModel(mRenderSys, mMove, MAKE_MAT_NAME("Lesson3.3"), layouts);
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