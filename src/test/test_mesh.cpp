#include "test/test_case.h"
#include "test/app.h"
#include "core/rendersys/material_factory.h"
#include "core/rendersys/scene_manager.h"
#include "core/renderable/renderable_factory.h"
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
	mContext->SceneMng()->SetOthogonalCamera(XMFLOAT3(0,0,-10), 100);

	auto texture = mContext->RenderSys()->LoadTexture("model\\theyKilledKenny.jpg");

	constexpr int spriteCount = 4;
	auto size = mContext->RenderSys()->GetWinSize();
	XMFLOAT2 points[spriteCount] = {
		XMFLOAT2(0, 0),
		XMFLOAT2(size.x / 2, 0),
		XMFLOAT2(size.x / 2, size.y / 2),
		XMFLOAT2(0, size.y / 2)
	};
	for (int i = 0; i < spriteCount; ++i) {
		auto sprite = mContext->RenderableFac()->CreateSprite();
		sprite->SetTexture(texture);

		sprite->SetPosition(points[i].x, points[i].y, 0);
		sprite->SetSize(size.x / 4, size.y / 4);

		mSprites.push_back(sprite);
	}

	mMesh = mContext->RenderableFac()->CreateMesh(E_MAT_SPRITE);

	for (int i = 0; i < spriteCount; ++i)
		mMesh->SetVertexs((MeshVertex*)mSprites[i]->GetQuad(), 4, 4 * i);
	
	mMesh->SetSubMeshCount(spriteCount);
	unsigned int indices[6 * spriteCount] = { 0, 1, 2, 0, 2, 3 };
	for (int i = 0; i < spriteCount; ++i) {
		mMesh->SetIndices(indices, 6 * i, 6, 4 * i, i);
		mMesh->SetTexture(0, texture, i);
	}
}

void TestMesh::OnRender()
{
	mContext->RenderPipe()->Draw(*mMesh);
}

#if defined TEST_MESH && TEST_CASE == TEST_MESH
auto reg = AppRegister<TestMesh>("TestMesh");
#endif