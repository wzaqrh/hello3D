#include "TApp.h"
#include "Export/Export.h"
#include "TSprite.h"

class TestExport : public IApp
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
public:
	std::string mName;
};

void TestExport::Create()
{

}

bool TestExport::Initialize(HINSTANCE hInstance, HWND hWnd)
{
	mRenderSys = RenderSystem_Create(hWnd, true);

	float poslists[3 * 4] = {
		-206.5, -110.5,155, 219,
		-108.5, -10.5, 155, 219,
		-8.5,  89.5,  155,  219
	};

	for (int i = 0; i < 3; ++i)
	{
		auto sp = SpriteImage_Create(mRenderSys, "model\\smile.png");
		//auto sp = SpriteColor_Create(mRenderSys, XMFLOAT4(0xA9 / 255.0, 0xA9 / 255.0, 0xA9 / 255.0, 1));
		XMINT4 size = mRenderSys->GetWinSize();
		sp->SetPosition(poslists[i * 4 + 0], poslists[i * 4 + 2], 0);
		sp->SetSize(poslists[i * 4 + 1] - poslists[i * 4 + 0], 
			poslists[i * 4 + 3] - poslists[i * 4 + 2]);

		mSprites.push_back(sp);
	}
	return true;
}

void TestExport::Render()
{
#ifdef EXPORT_STRUCT
	ExportRenderable rends[] = { mSprite.self };
#else
	ExportRenderable rends[3] = {};
	for (int i = 0; i < mSprites.size(); ++i)
		rends[i] = mSprites[i];
#endif
	RenderSystem_Render(mRenderSys, XMFLOAT4(0,0,0,0), rends, mSprites.size());
}

void TestExport::CleanUp()
{
	RenderSystem_Destroy(mRenderSys);
}

std::string TestExport::GetName()
{
	return "TestExport";
}

auto reg = AppRegister<TestExport>("TestExport");