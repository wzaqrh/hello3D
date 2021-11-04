#include "test/test_case.h"
#if defined TEST_MODEL && TEST_CASE == TEST_MODEL
#include "test/app.h"
#include "core/renderable/assimp_model.h"
#include "core/base/transform.h"
#include "core/base/utility.h"

using namespace mir;

class TestModel : public TApp
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
private:
	int mDrawFlag = 0;
	TAssimpModel* mModel = nullptr;
};

void TestModel::OnPostInitDevice()
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
	mModel = new AssimpModel(mContext->GetRenderSys(), mMove, MAKE_MAT_NAME("Lesson1"), layouts);
#else
	mModel = new TAssimpModel(mContext->GetRenderSys(), mMove, E_MAT_MODEL);
#endif
	//gModelPath = "Spaceship\\"; mModel->LoadModel(MakeModelPath("Spaceship.fbx")); mMove->SetDefScale(0.01); mModel->PlayAnim(0);
	//gModelPath = "Normal\\"; mModel->LoadModel(MakeModelPath("Deer.fbx")); 
	//gModelPath = "handgun\\"; mModel->LoadModel(MakeModelPath("handgun.fbx")); mMove->SetDefScale(0.01);
	gModelPath = "Male03\\"; mModel->LoadModel(MakeModelPath("Male02.FBX")); mMove->SetDefScale(0.07); mMove->SetPosition(0, -5, 0);
}

void TestModel::OnRender()
{
	mModel->Update(mTimer->mDeltaTime);
	if (mContext->GetRenderSys()->BeginScene()) {
		mContext->GetRenderSys()->Draw(mModel);
		mContext->GetRenderSys()->EndScene();
	}
}

auto reg = AppRegister<TestModel>("TestModel: Assimp Model");
#endif
