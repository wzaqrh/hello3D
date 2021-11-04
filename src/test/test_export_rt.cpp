#include "test/test_case.h"
#if defined TEST_EXPORT_RT && TEST_CASE == TEST_EXPORT_RT
#include "test/app.h"
#include "Export/Export.h"
#include "core/rendersys/interface_type.h"
#include "core/renderable/sprite.h"

using namespace mir;

class TestExportRT : public IApp
{
protected:
	virtual void Create() override;
	virtual bool Initialize(HINSTANCE hInstance, HWND hWnd) override;
	virtual void Render() override;
	virtual void CleanUp() override;
	virtual std::string GetName() override;
private:
	ExportRenderSystem mRenderSys;
	ExportSprite mSprite,mSpriteRT;
	ExportRenderTarget mRT;
public:
	std::string mName;
};

void TestExportRT::Create()
{}

bool TestExportRT::Initialize(HINSTANCE hInstance, HWND hWnd)
{
	mRenderSys = RenderSystem_Create(hWnd, true, {0,0,0,0});

	//mSprite = SpriteImage_Create(mRenderSys, "model\\smile.png");
	mSprite = SpriteColor_Create(mRenderSys, XMFLOAT4(0xA9 / 255.0, 0xA9 / 255.0, 0xA9 / 255.0, 1));
	XMINT4 size = mRenderSys->GetWinSize();
	mSprite->SetPosition(-size.x / 4, -size.y / 4, 0);
	mSprite->SetSize(size.x / 2, size.y / 2);

	mSpriteRT = SpriteImage_Create(mRenderSys, nullptr);
	mSpriteRT->SetPosition(-size.x / 4, -size.y / 4, 0);
	mSpriteRT->SetSize(size.x / 2, size.y / 2);

	mRT = RenderTarget_Create(mRenderSys, size.x, size.y, DXGI_FORMAT_B8G8R8A8_UNORM);
	mSpriteRT->SetTexture(MakePtr(RenderTarget_GetTexture(mRT)));
	return true;
}

void TestExportRT::Render()
{
	RenderSystem_RClear(mRenderSys, XMFLOAT4(0, 0, 0, 0));

	if (RenderSystem_RBeginScene(mRenderSys))
	{
		RenderSystem_SetRenderTarget(mRenderSys, mRT);
		ExportRenderable rends1[] = { mSprite };
		RenderSystem_RRender(mRenderSys, rends1, 1);

		RenderSystem_SetRenderTarget(mRenderSys, nullptr);
		ExportRenderable rends2[] = { mSpriteRT };
		RenderSystem_RRender(mRenderSys, rends2, 1);

		RenderSystem_REndScene(mRenderSys);
	}
}

void TestExportRT::CleanUp()
{
	RenderSystem_Destroy(mRenderSys);
}

std::string TestExportRT::GetName()
{
	return "TestExportRT";
}

auto reg = AppRegister<TestExportRT>("TestExportRT");
#endif