#include "TApp.h"
#include "TInterfaceType.h"
#include "TSprite.h"
#include "TRenderSystem11.h"
#include "TRenderSystem9.h"

class TestSprite : public TApp
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
private:
	TSpritePtr mSprite;
};

void TestSprite::OnPostInitDevice()
{
#if 0
	mSprite = std::make_shared<TSprite>(mRenderSys, E_MAT_SPRITE);
	mSprite->SetTexture(mRenderSys->GetTexByPath("image\\smile.png"));
	mSprite->SetPosition(-5, -5, 0);
	mSprite->SetSize(5, 5);
#else
	mRenderSys->SetOthogonalCamera(100);

	//mSprite = std::make_shared<TSprite>(mRenderSys, E_MAT_LAYERCOLOR);
	mSprite = std::make_shared<TSprite>(mRenderSys, E_MAT_SPRITE);
	mSprite->SetTexture(mRenderSys->LoadTexture("model\\theyKilledKenny.jpg"));

	mSprite->SetPosition(-mRenderSys->GetWinSize().x / 2, -mRenderSys->GetWinSize().y / 2, 0);
	mSprite->SetSize(mRenderSys->GetWinSize().x, mRenderSys->GetWinSize().y);
#endif
}

void TestSprite::OnRender()
{
	if (mRenderSys->BeginScene()) {
		mRenderSys->Draw(mSprite.get());
		mRenderSys->EndScene();
	}
}

//auto reg = AppRegister<TestSprite>("Sprite");