#include "test/test_case.h"
#if defined TEST_SPRITE && TEST_CASE == TEST_SPRITE
#include "test/app.h"
#include "core/rendersys/scene_manager.h"
#include "core/rendersys/material_factory.h"
#include "core/renderable/sprite.h"

using namespace mir;

class TestSprite : public App
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
private:
	SpritePtr mSprite;
};

void TestSprite::OnPostInitDevice()
{
#if 0
	mSprite = std::make_shared<TSprite>(mContext->GetRenderSys(), E_MAT_SPRITE);
	mSprite->SetTexture(mContext->GetRenderSys()->GetTexByPath("image\\smile.png"));
	mSprite->SetPosition(-5, -5, 0);
	mSprite->SetSize(5, 5);
#else
	mContext->GetSceneMng()->SetOthogonalCamera(100);

	//mSprite = std::make_shared<TSprite>(mContext->GetRenderSys(), E_MAT_LAYERCOLOR);
	mSprite = std::make_shared<Sprite>(mContext->GetRenderSys(), E_MAT_SPRITE);
	mSprite->SetTexture(mContext->GetRenderSys()->LoadTexture("model\\theyKilledKenny.jpg"));

	int win_width = mContext->GetRenderSys()->GetWinSize().x;
	int win_height = mContext->GetRenderSys()->GetWinSize().y;
	mSprite->SetPosition(0, 0, 0);
	mSprite->SetSize(win_width / 2, win_height / 2);
#endif
}

void TestSprite::OnRender()
{
	if (mContext->GetRenderSys()->BeginScene()) {
		mContext->GetRenderSys()->Draw(mSprite.get());
		mContext->GetRenderSys()->EndScene();
	}
}

auto reg = AppRegister<TestSprite>("Sprite");
#endif