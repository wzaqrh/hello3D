#include "test/test_case.h"

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
	mContext->SceneMng()->RemoveAllCameras();
	mContext->SceneMng()->AddOthogonalCamera(XMFLOAT3(0,0,-10), 100);

	//mSprite = std::make_shared<TSprite>(mContext->GetRenderSys(), E_MAT_LAYERCOLOR);
	mSprite = std::make_shared<Sprite>(*mContext->RenderSys(), *mContext->MaterialFac(), E_MAT_SPRITE);
	mSprite->SetTexture(mContext->RenderSys()->LoadTexture("model\\theyKilledKenny.jpg"));

	int win_width = mContext->RenderSys()->GetWinSize().x;
	int win_height = mContext->RenderSys()->GetWinSize().y;
	mSprite->SetPosition(0, 0, 0);
	mSprite->SetSize(win_width / 2, win_height / 2);
#endif
}

void TestSprite::OnRender()
{
	mContext->RenderPipe()->Draw(*mSprite, *mContext->SceneMng());
}

#if defined TEST_SPRITE && TEST_CASE == TEST_SPRITE
auto reg = AppRegister<TestSprite>("Sprite");
#endif