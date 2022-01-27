#include "test/test_case.h"
#include "test/app.h"
#include "core/scene/scene_manager.h"
#include "core/renderable/sprite.h"

using namespace mir;
using namespace mir::renderable;

class TestSprite : public App
{
protected:
	void OnRender() override;
	cppcoro::shared_task<void> OnPostInitDevice() override;
	void OnInitLight() override {}
	void OnInitCamera() override {}
private:
	SpritePtr mSprite;
};
/*mCaseIndex
0：观察到kenny占整个屏幕
1: 观察到kenny占右上半区
2：观察到kenny占整个屏幕
3: 观察到kenny占右下半区
4：观察到lenna左下角在屏幕中心, 自然尺寸，有透明
5：观察到kenny屏幕居中, 半个屏幕尺寸
6：观察到kenny左下角, 自然尺寸
*/

cppcoro::shared_task<void> TestSprite::OnPostInitDevice()
{
	mScneMng->AddDirectLight();
	mScneMng->AddOthogonalCamera(test1::cam_otho::Eye(mWinCenter));
	SetPPU(C_WINDOW_HEIGHT / 10.0);

	Launch sync = __LaunchSync__;
	Launch async = __LaunchAsync__;
	switch (mCaseIndex) {
	case 0: {
		mSprite = co_await mRendFac->CreateSprite();
		//mSprite->SetTexture(mResMng->CreateTextureByFile(sync, test1::res::Sky(), kFormatR32G32B32A32Float));
		mSprite->SetTexture(co_await mResMng->CreateTextureByFile(async, test1::res::dds::Kenny()));
		test1::res::png::SetPos(mSprite, -mCamWinHSize, mCamWinHSize * 2);
	}break;
	case 1: {
		mSprite = co_await mRendFac->CreateSprite();
		mSprite->SetTexture(co_await mResMng->CreateTextureByFile(sync, test1::res::png::Kenny(), kFormatUnknown, true));//auto_gen_mipmap
	
		test1::res::png::SetPos(mSprite, mCamWinHSize, mCamWinHSize, math::vec::anchor::RightTop());
	}break;
	case 2: {
		mSprite = co_await mRendFac->CreateSprite(test1::res::png::Kenny());

		test1::res::png::SetPos(mSprite, mWinCenter, mCamWinHSize * 2, math::vec::anchor::Center());
	}break;
	case 3: {
		mSprite = co_await mRendFac->CreateSprite(test1::res::hdr::Kenny());//zlib

		test1::res::png::SetPos(mSprite, Eigen::Vector3f::Zero(), mCamWinHSize, math::vec::anchor::LeftTop());
	}break;
	case 4: {
		mSprite = co_await mRendFac->CreateSprite(test1::res::dds::Lenna());//bc1a

		mSprite->SetPosition(mWinCenter);
		mSprite->SetAnchor(math::vec::anchor::Center());
	}break;
	case 5: {
		mSprite = co_await mRendFac->CreateSprite(test1::res::dds::Kenny());//bc1a + mipmap

		test1::res::png::SetPos(mSprite, -mCamWinHSize / 2, mCamWinHSize);
	}break;
	case 6: {
		mSprite = co_await mRendFac->CreateSprite();
		mSprite->SetTexture(co_await mResMng->CreateTextureByFileAsync(test1::res::png::Kenny(), kFormatUnknown, true));//auto_gen_mipmap
	
		mSprite->SetPosition(-mCamWinHSize);
	}break;
	default:
		break;
	}
}

void TestSprite::OnRender()
{
	mContext->RenderPipe()->Draw(*mSprite, *mContext->SceneMng());
}

auto reg = AppRegister<TestSprite>("test_sprite");