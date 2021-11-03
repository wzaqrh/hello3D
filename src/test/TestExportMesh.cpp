#include "TApp.h"
#include "Export/Export.h"
#include "TSprite.h"

class TestExportMesh : public IApp
{
protected:
	virtual void Create() override;
	virtual bool Initialize(HINSTANCE hInstance, HWND hWnd) override;
	virtual void Render() override;
	virtual void CleanUp() override;
	virtual std::string GetName() override;
private:
	ExportRenderSystem mRenderSys;
	std::vector<ExportSprite> mSprites;
	ExportMesh mMesh;
public:
	std::string mName;
};

void TestExportMesh::Create()
{

}

bool TestExportMesh::Initialize(HINSTANCE hInstance, HWND hWnd)
{
	RECT rc;
	GetClientRect(hWnd, &rc);
	rc.left = rc.bottom / 2; rc.top = rc.bottom / 2;
	//rc.bottom /= 2; rc.right /= 2;

	mRenderSys = RenderSystem_Create(hWnd, true, rc);

	auto texture = Texture_Load(mRenderSys, "model\\theyKilledKenny.jpg", false);

	const int spriteCount = 4;
	auto size = mRenderSys->GetWinSize();
	float xlist[] = { -size.x / 2, 0, -size.x / 2 , 0 };
	float ylist[] = { -size.y / 2, -size.y / 2, 0 , 0 };
	for (int i = 0; i < spriteCount; ++i)
	{
		auto sprite = SpriteImage_Create(mRenderSys, nullptr);
		Sprite_SetTexture(sprite, texture);
		Sprite_SetRect(sprite, XMFLOAT2(xlist[i], ylist[i]), XMFLOAT2(size.x / 2, size.y / 2));
		mSprites.push_back(sprite);
	}

	mMesh = Mesh_Create(mRenderSys, 1024, 1024);
	{
		for (int i = 0; i < spriteCount; ++i)
			Mesh_SetVertexsPC(mMesh, (MeshVertex*)mSprites[i]->GetQuad(), 4, 4 * i);

		Mesh_SetSubMeshCount(mMesh, spriteCount);
		unsigned int indices[6 * spriteCount] = {
			0, 1, 2, 0, 2, 3
		};
		for (int i = 0; i < spriteCount; ++i)
		{
			for (int j = 0; j < 6; ++j)
				indices[i * 6 + j] = indices[j] + 4 * i;
			Mesh_SetIndices(mMesh, indices + 6 * i, 6 * i, 6, i);
			Mesh_SetTexture(mMesh, 0, texture, i);
		}
	}

	return true;
}

void TestExportMesh::Render()
{
	ExportRenderable rends[1] = { mMesh };
	RenderSystem_Render(mRenderSys, XMFLOAT4(0, 0, 0, 0), rends, ARRAYSIZE(rends));
}

void TestExportMesh::CleanUp()
{
	RenderSystem_Destroy(mRenderSys);
}

std::string TestExportMesh::GetName()
{
	return "TestExportMesh";
}

auto reg = AppRegister<TestExportMesh>("TestExportMesh");