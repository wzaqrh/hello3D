#include "test/test_case.h"
#if defined TEST_DIFFUSE && TEST_CASE == TEST_DIFFUSE
#include "test/app.h"
#include "core/rendersys/scene_manager.h"
#include "core/renderable/assimp_model.h"
#include "core/base/transform.h"
#include "core/base/utility.h"

class TestDiffuse : public TApp
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
	virtual void OnInitLight() override;
private:
	TAssimpModel* mModel = nullptr;
};

void TestDiffuse::OnInitLight()
{
	auto light = mContext->GetSceneMng()->AddPointLight();
	light->SetDiffuseColor(1, 1, 1, 1);
	light->SetPosition(0, 0, -200);
}

void TestDiffuse::OnPostInitDevice()
{
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
	mModel = new TAssimpModel(mContext->GetRenderSys(), mMove, MAKE_MAT_NAME("Lesson2"), layouts);
	gModelPath = "Spaceship\\"; mModel->LoadModel(MakeModelPath("Spaceship.fbx")); mMove->SetDefScale(0.01);
	//gModelPath = "Normal\\"; mModel->LoadModel(MakeModelPath("Deer.fbx")); mScale = 0.05;
	//mModel->PlayAnim(0);
}

void TestDiffuse::OnRender()
{
	mModel->Update(mTimer->mDeltaTime);
	if (mContext->GetRenderSys()->BeginScene()) {
		mModel->Draw();
		mContext->GetRenderSys()->EndScene();
	}
}

auto reg = AppRegister<TestDiffuse>("TestDiffuse: Diffuse Light");
#endif