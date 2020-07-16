#include "TestCase.h"
#if defined TEST_BLOOM && TEST_CASE == TEST_BLOOM
#include "TApp.h"
#include "ISceneManager.h"
#include "TAssimpModel.h"
#include "TSprite.h"
#include "TTransform.h"
#include "Utility.h"

class TestBloom : public TApp
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
private:
	TAssimpModel* mModel = nullptr;
};

void TestBloom::OnPostInitDevice()
{
	mContext->GetSceneMng()->SetPerspectiveCamera(45, 10, 300);

	mContext->GetSceneMng()->SetSkyBox("images\\uffizi_cross.dds");
	mContext->GetSceneMng()->AddPostProcess(E_PASS_POSTPROCESS);

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
	mModel = new TAssimpModel(mContext->GetRenderSys(), mMove, MAKE_MAT_NAME("Lesson3.3"), layouts);
	gModelPath = "Spaceship\\"; if (mModel) mModel->LoadModel(MakeModelPath("Spaceship.fbx")); mMove->SetDefScale(0.01);
}

void TestBloom::OnRender()
{
	if (mModel) mModel->Update(mTimer->mDeltaTime);

	if (mContext->GetRenderSys()->BeginScene()) {
		if (mModel) mModel->Draw();
		mContext->GetRenderSys()->EndScene();
	}
}

auto reg = AppRegister<TestBloom>("TestBloom: PostProcess Bloom");
#endif