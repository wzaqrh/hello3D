#include "test/test_case.h"
#if defined TEST_FOG && TEST_CASE == TEST_FOG
#include "test/app.h"
#include "core/renderable/assimp_model.h"
#include "core/base/transform.h"
#include "core/base/utility.h"

using namespace mir;

class TestFog : public App
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
private:
	AssimpModel* mModel = nullptr;
};

/********** Lesson4 **********/
void TestFog::OnPostInitDevice()
{
	auto light2 = mContext->SceneMng()->AddDirectLight();
	light2->SetDirection(0, 0, 1);

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
	mModel = new AssimpModel(mContext->RenderSys(), mMove, MAKE_MAT_NAME("Lesson4"), layouts, [&](MaterialPtr mat) {
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

void TestFog::OnRender()
{
	mModel->Update(mTimer->mDeltaTime);
	if (mContext->RenderSys()->BeginScene1()) {
		mModel->Draw();
		mContext->RenderSys()->EndScene1();
	}
}

auto reg = AppRegister<TestFog>("TestFog: Fog");
#endif