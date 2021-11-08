#include "test/test_case.h"
#if defined TEST_MESH && TEST_CASE == TEST_MESH
#include "test/app.h"
#include "core/rendersys/scene_manager.h"
#include "core/renderable/mesh.h"
#include "core/renderable/sprite.h"
#include "core/base/transform.h"
#include "core/base/utility.h"

using namespace mir;

class TestMesh : public App
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
private:
	std::vector<SpritePtr> mSprites;
	MeshPtr mMesh;
};


void TestMesh::OnPostInitDevice()
{
	mContext->SceneMng()->SetOthogonalCamera(100);

	auto texture = mContext->RenderSys()->LoadTexture("model\\theyKilledKenny.jpg");

	const int spriteCount = 4;
	auto size = mContext->RenderSys()->GetWinSize();
	float xlist[] = { -size.x / 2, 0, -size.x / 2 , 0 };
	float ylist[] = { -size.y / 2, -size.y / 2, 0 , 0 };
	for (int i = 0; i < spriteCount; ++i)
	{
		auto sprite = mContext->RenderableFac()->CreateSprite();
		sprite->SetTexture(texture);

		sprite->SetPosition(xlist[i], ylist[i], 0);
		sprite->SetSize(size.x / 2, size.y / 2);

		mSprites.push_back(sprite);
	}

	mMesh = mContext->RenderableFac()->CreateMesh(E_MAT_SPRITE);

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
	if (mContext->RenderSys()->BeginScene1()) {
		mContext->RenderSys()->Draw1(mMesh.get());
		mContext->RenderSys()->EndScene1();
	}
}

auto reg = AppRegister<TestMesh>("TestMesh");
#endif