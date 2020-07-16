#include "TestCase.h"
#if defined TEST_MESH && TEST_CASE == TEST_MESH
#include "TApp.h"
#include "ISceneManager.h"
#include "TMesh.h"
#include "TSprite.h"
#include "TTransform.h"
#include "Utility.h"

class TestMesh : public TApp
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
private:
	std::vector<TSpritePtr> mSprites;
	TMeshPtr mMesh;
};


void TestMesh::OnPostInitDevice()
{
	mContext->GetSceneMng()->SetOthogonalCamera(100);

	auto texture = mContext->GetRenderSys()->LoadTexture("model\\theyKilledKenny.jpg");

	const int spriteCount = 4;
	auto size = mContext->GetRenderSys()->GetWinSize();
	float xlist[] = { -size.x / 2, 0, -size.x / 2 , 0 };
	float ylist[] = { -size.y / 2, -size.y / 2, 0 , 0 };
	for (int i = 0; i < spriteCount; ++i)
	{
		auto sprite = mContext->GetRenderableFac()->CreateSprite();
		sprite->SetTexture(texture);

		sprite->SetPosition(xlist[i], ylist[i], 0);
		sprite->SetSize(size.x / 2, size.y / 2);

		mSprites.push_back(sprite);
	}

	mMesh = mContext->GetRenderableFac()->CreateMesh(E_MAT_SPRITE);

	for (int i = 0; i < spriteCount; ++i)
		mMesh->SetVertexs((MeshVertex*)mSprites[i]->GetQuad(), 4, 4 * i);
	
	mMesh->SetSubMeshCount(spriteCount);
	unsigned int indices[6 * spriteCount] = {
		0, 1, 2, 0, 2, 3
	};
	for (int i = 0; i < spriteCount; ++i)
	{
		mMesh->SetIndices(indices, 6 * i, 6, 4 * i, i);
		mMesh->SetTexture(0, texture, i);
	}
	
}

void TestMesh::OnRender()
{
	if (mContext->GetRenderSys()->BeginScene()) {
		mContext->GetRenderSys()->Draw(mMesh.get());
		mContext->GetRenderSys()->EndScene();
	}
}

auto reg = AppRegister<TestMesh>("TestMesh");
#endif