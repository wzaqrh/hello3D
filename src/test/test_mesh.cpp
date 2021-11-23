#include "test/test_case.h"
#include "test/app.h"
#include "core/resource/material_factory.h"
#include "core/rendersys/scene_manager.h"
#include "core/renderable/renderable_factory.h"
#include "core/renderable/mesh.h"
#include "core/renderable/sprite.h"
#include "core/base/transform.h"

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
	mContext->SceneMng()->RemoveAllCameras();
	mContext->SceneMng()->AddOthogonalCamera(Eigen::Vector3f(0,0,-10), 100);

	auto texture = mContext->ResourceMng()->CreateTextureByFileSync("model/theyKilledKenny.jpg");

	constexpr int spriteCount = 4;
	auto size = mContext->ResourceMng()->WinSize();
	Eigen::Vector2f points[spriteCount] = {
		Eigen::Vector2f(0, 0),
		Eigen::Vector2f(size.x() / 2, 0),
		Eigen::Vector2f(size.x() / 2, size.y() / 2),
		Eigen::Vector2f(0, size.y() / 2)
	};
	for (int i = 0; i < spriteCount; ++i) {
		auto sprite = mContext->RenderableFac()->CreateSprite();
		sprite->SetTexture(texture);

		sprite->SetPosition(Eigen::Vector3f(points[i].x(), points[i].y(), 0.0f));
		sprite->SetSize(Eigen::Vector2f(size.x() / 4, size.y() / 4));

		mSprites.push_back(sprite);
	}

	mMesh = mContext->RenderableFac()->CreateMesh();

	for (int i = 0; i < spriteCount; ++i)
		mMesh->SetVertexs((MeshVertex*)mSprites[i]->GetVertexData(), 4, 4 * i);
	
	mMesh->SetSubMeshCount(spriteCount);
	unsigned int indices[6 * spriteCount] = { 0, 1, 2, 0, 2, 3 };
	for (int i = 0; i < spriteCount; ++i) {
		mMesh->SetIndices(indices, 6 * i, 6, 4 * i, i);
		mMesh->SetTexture(0, texture, i);
	}
}

void TestMesh::OnRender()
{
	mContext->RenderPipe()->Draw(*mMesh, *mContext->SceneMng());
}

#if defined TEST_MESH && TEST_CASE == TEST_MESH
auto reg = AppRegister<TestMesh>("TestMesh");
#endif