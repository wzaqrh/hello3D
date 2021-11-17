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
	mContext->SceneMng()->RemoveAllCameras();
	mContext->SceneMng()->AddOthogonalCamera(Eigen::Vector3f(0,0,-10), 100);

	mSprite = mContext->RenderableFac()->CreateSprite();
	mSprite->SetTexture(mContext->RenderSys()->LoadTexture(nullptr, "model\\theyKilledKenny.jpg"));

	int win_width = mContext->RenderSys()->WinSize().x();
	int win_height = mContext->RenderSys()->WinSize().y();
	mSprite->SetPosition(Eigen::Vector3f(0, 0, 0));
	mSprite->SetSize(Eigen::Vector2f(win_width / 2, win_height / 2));
}

void TestSprite::OnRender()
{
	mContext->RenderPipe()->Draw(*mSprite, *mContext->SceneMng());
}

#if defined TEST_SPRITE && TEST_CASE == TEST_SPRITE
auto reg = AppRegister<TestSprite>("Sprite");
#endif