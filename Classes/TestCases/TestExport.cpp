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
	ExportSprite mSprite;
public:
	std::string mName;
};

void TestExport::Create()
{

}

bool TestExport::Initialize(HINSTANCE hInstance, HWND hWnd)
{
	mRenderSys = RenderSystem_Create(hWnd, true);

	//mSprite = SpriteImage_Create(mRenderSys, "model\\smile.png");
	mSprite = SpriteColor_Create(mRenderSys, XMFLOAT4(0xA9 / 255.0, 0xA9 / 255.0, 0xA9 / 255.0, 1));
	XMINT4 size = mRenderSys->GetWinSize();
	mSprite->SetPosition(-size.x / 4, -size.y / 4, 0);
	mSprite->SetSize(size.x / 2, size.y / 2);
	return true;
}

void TestExport::Render()
{
#ifdef EXPORT_STRUCT
	ExportRenderable rends[] = { mSprite.self };
#else
	ExportRenderable rends[] = { mSprite };
#endif
	RenderSystem_Render(mRenderSys, XMFLOAT4(0,0,0,0), rends, sizeof(rends) / sizeof(rends[0]));
}

void TestExport::CleanUp()
{
	RenderSystem_Destroy(mRenderSys);
}

std::string TestExport::GetName()
{
	return "TestExport";
}

//auto reg = AppRegister<TestExport>("TestExport");