#include "test/test_case.h"
#include "test/app.h"
#include "core/scene/scene_manager.h"
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
/*mCaseIndex
0：
1: 观察到kenny占右上半区
2：观察到kenny占整个屏幕
3: 观察到kenny占右下半区
4：观察到lenna左下角在屏幕中心, 自然尺寸，有透明
5：观察到kenny屏幕居中, 半个屏幕尺寸
6：观察到kenny左下角, 自然尺寸
*/

void TestSprite::OnPostInitDevice()
{
	auto sceneMng = mContext->SceneMng();
	auto rendFac = mContext->RenderableFac();
	auto resMng = mContext->ResourceMng();
	auto halfSize = mContext->ResourceMng()->WinSize().cast<float>() / 2;
	auto winCenter = Eigen::Vector3f(halfSize.x(), halfSize.y(), 0);

	sceneMng->RemoveAllCameras();
	sceneMng->AddOthogonalCamera(winCenter + Eigen::Vector3f(0, 0, -10));

	Launch sync = __LaunchSync__;
	switch (mCaseIndex) {
	case 0: {
		mSprite = rendFac->CreateSprite();
		mSprite->SetTexture(resMng->CreateTextureByFile(sync, test1::res::Sky(), kFormatR32G32B32A32Float));

		mSprite->SetPosition(Eigen::Vector3f::Zero());
		mSprite->SetSize(halfSize);
	}break;
	case 1: {
		mSprite = rendFac->CreateSprite();
		mSprite->SetTexture(resMng->CreateTextureByFile(sync, test1::res::png::Kenny(), kFormatUnknown, true));//auto_gen_mipmap
	
		mSprite->SetPosition(winCenter);
		mSprite->SetSize(halfSize);
	}break;
	case 2: {
		mSprite = rendFac->CreateSprite(test1::res::png::Kenny());

		mSprite->SetPosition(Eigen::Vector3f::Zero());
		mSprite->SetSize(halfSize * 2);
	}break;
	case 3: {
		mSprite = rendFac->CreateSprite(test1::res::hdr::Kenny());//zlib

		mSprite->SetPosition(Eigen::Vector3f(halfSize.x(), 0, 0));
		mSprite->SetSize(halfSize);
	}break;
	case 4: {
		mSprite = rendFac->CreateSprite(test1::res::dds::Lenna());//bc1a

		mSprite->SetPosition(winCenter);
	}break;
	case 5: {
		mSprite = rendFac->CreateSprite(test1::res::dds::Kenny());//bc1a + mipmap

		mSprite->SetPosition(winCenter - Eigen::Vector3f(halfSize.x()/2, halfSize.y()/2, 0));
		mSprite->SetSize(halfSize);
	}break;
	case 6: {
		mSprite = rendFac->CreateSprite();
		mSprite->SetTexture(resMng->CreateTextureByFileAsync(test1::res::png::Kenny(), kFormatUnknown, true));//auto_gen_mipmap
	
		mSprite->SetPosition(Eigen::Vector3f::Zero());
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