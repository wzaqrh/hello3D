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

	mSprite = Sprite_Create(mRenderSys, "model\\smile.png");
	mSprite->SetPosition(-mRenderSys->GetWinSize().x / 2, -mRenderSys->GetWinSize().y / 2, 0);
	mSprite->SetSize(mRenderSys->GetWinSize().x / 2, mRenderSys->GetWinSize().y / 2);
	
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