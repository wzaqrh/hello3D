#include "test/test_case.h"
#include "test/app.h"
#include "core/scene/scene_manager.h"
#include "core/resource/material_factory.h"
#include "core/renderable/sprite.h"

using namespace mir;

class TestSprite : public App
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
private:
	SpritePtr mSprite;
	int mCaseIndex = 6;
};

void TestSprite::OnPostInitDevice()
{
	mContext->SceneMng()->RemoveAllCameras();
	mContext->SceneMng()->AddOthogonalCamera(Eigen::Vector3f(0,0,-10), 100);

	Launch sync = __LaunchSync__;
	switch (mCaseIndex) {
	case 0: {
		mSprite = mContext->RenderableFac()->CreateSprite();
		mSprite->SetTexture(mContext->ResourceMng()->CreateTextureByFile(sync, "model/uffizi_cross.dds", kFormatR32G32B32A32Float));
	}break;
	case 1: {
		mSprite = mContext->RenderableFac()->CreateSprite();
		mSprite->SetTexture(mContext->ResourceMng()->CreateTextureByFile(
			sync, "model/theyKilledKenny.png", kFormatUnknown, true));//auto_gen_mipmap
	}break;
	case 2: {
		mSprite = mContext->RenderableFac()->CreateSprite("model/theyKilledKenny.png");
	}break;
	case 3: {
		mSprite = mContext->RenderableFac()->CreateSprite("model/theyKilledKenny.hdr");//zlib
	}break;
	case 4: {
		mSprite = mContext->RenderableFac()->CreateSprite("model/lenna.dds");//bc1a
	}break;
	case 5: {
		mSprite = mContext->RenderableFac()->CreateSprite("model/theyKilledKenny.dds");//bc1a + mipmap
	}break;
	case 6: {
		mSprite = mContext->RenderableFac()->CreateSprite();
		mSprite->SetTexture(mContext->ResourceMng()->CreateTextureByFileAsync(
			"model/theyKilledKenny.png", kFormatUnknown, true));//auto_gen_mipmap
	}break;
	default:
		break;
	}
	int win_width = mContext->ResourceMng()->WinSize().x();
	int win_height = mContext->ResourceMng()->WinSize().y();
	mSprite->SetPosition(Eigen::Vector3f(0, 0, 0));
	mSprite->SetSize(Eigen::Vector2f(win_width / 2, win_height / 2));
}

void TestSprite::OnRender()
{
	mContext->RenderPipe()->Draw(*mSprite, *mContext->SceneMng());
}

auto reg = AppRegister<TestSprite>("test_sprite");