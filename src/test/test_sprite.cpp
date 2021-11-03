#include "test/test_case.h"
#if defined TEST_SPRITE && TEST_CASE == TEST_SPRITE
#include "test/app.h"
#include "core/rendersys/scene_manager.h"
#include "core/renderable/sprite.h"

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
	mContext->GetSceneMng()->SetOthogonalCamera(100);

	//mSprite = std::make_shared<TSprite>(mRenderSys, E_MAT_LAYERCOLOR);
	mSprite = std::make_shared<TSprite>(mRenderSys, E_MAT_SPRITE);
	mSprite->SetTexture(mRenderSys->LoadTexture("model\\theyKilledKenny.jpg"));

	mSprite->SetPosition(0, 0, 0);
	mSprite->SetSize(mRenderSys->GetWinSize().x, mRenderSys->GetWinSize().y);
#endif
}

void TestSprite::OnRender()
{
	if (mContext->GetRenderSys()->BeginScene()) {
		mRenderSys->Draw(mSprite.get());
		mContext->GetRenderSys()->EndScene();
	}
}

auto reg = AppRegister<TestSprite>("Sprite");
#endif