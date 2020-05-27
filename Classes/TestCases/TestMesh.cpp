#include "TApp.h"
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
	mRenderSys->SetOthogonalCamera(100);

	auto texture = mRenderSys->LoadTexture("model\\theyKilledKenny.jpg");

	const int spriteCount = 4;
	auto size = mRenderSys->GetWinSize();
	float xlist[] = { -size.x / 2, 0, -size.x / 2 , 0 };
	float ylist[] = { -size.y / 2, -size.y / 2, 0 , 0 };
	for (int i = 0; i < spriteCount; ++i)
	{
		auto sprite = std::make_shared<TSprite>(mRenderSys, E_MAT_SPRITE);
		sprite->SetTexture(texture);

		sprite->SetPosition(xlist[i], ylist[i], 0);
		sprite->SetSize(size.x / 2, size.y / 2);

		mSprites.push_back(sprite);
	}


	mMesh = std::make_shared<TMesh>(mRenderSys, E_MAT_SPRITE);

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
	if (mRenderSys->BeginScene()) {
		mRenderSys->Draw(mMesh.get());
		mRenderSys->EndScene();
	}
}

//auto reg = AppRegister<TestMesh>("estMesh");