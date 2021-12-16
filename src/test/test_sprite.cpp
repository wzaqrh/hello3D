#include "test/test_case.h"
#include "test/app.h"
#include "core/scene/scene_manager.h"
#include "core/renderable/sprite.h"

using namespace mir;

class TestSprite : public App
{
protected:
	void OnRender() override;
	void OnPostInitDevice() override;
	void OnInitLight() override {}
	void OnInitCamera() override {}
private:
	SpritePtr mSprite;
};
/*mCaseIndex
0���۲쵽kennyռ������Ļ
1: �۲쵽kennyռ���ϰ���
2���۲쵽kennyռ������Ļ
3: �۲쵽kennyռ���°���
4���۲쵽lenna���½�����Ļ����, ��Ȼ�ߴ磬��͸��
5���۲쵽kenny��Ļ����, �����Ļ�ߴ�
6���۲쵽kenny���½�, ��Ȼ�ߴ�
*/

void TestSprite::OnPostInitDevice()
{
	mScneMng->AddDirectLight();
	mScneMng->AddOthogonalCamera(test1::cam_otho::Eye(mWinCenter));
	SetPPU(C_WINDOW_HEIGHT / 10.0);

	Launch sync = __LaunchSync__;
	switch (mCaseIndex) {
	case 0: {
		mSprite = mRendFac->CreateSprite();
		//mSprite->SetTexture(mResMng->CreateTextureByFile(sync, test1::res::Sky(), kFormatR32G32B32A32Float));
		mSprite->SetTexture(mResMng->CreateTextureByFile(sync, test1::res::dds::Kenny()));

		test1::res::png::SetPos(mSprite, -mCamWinHSize, mCamWinHSize * 2);
	}break;
	case 1: {
		mSprite = mRendFac->CreateSprite();
		mSprite->SetTexture(mResMng->CreateTextureByFile(sync, test1::res::png::Kenny(), kFormatUnknown, true));//auto_gen_mipmap
	
		test1::res::png::SetPos(mSprite, mCamWinHSize, mCamWinHSize, mir::math::vec::anchor::RightTop());
	}break;
	case 2: {
		mSprite = mRendFac->CreateSprite(test1::res::png::Kenny());

		test1::res::png::SetPos(mSprite, mWinCenter, mCamWinHSize * 2, mir::math::vec::anchor::Center());
	}break;
	case 3: {
		mSprite = mRendFac->CreateSprite(test1::res::hdr::Kenny());//zlib

		test1::res::png::SetPos(mSprite, Eigen::Vector3f::Zero(), mCamWinHSize, mir::math::vec::anchor::LeftTop());
	}break;
	case 4: {
		mSprite = mRendFac->CreateSprite(test1::res::dds::Lenna());//bc1a

		mSprite->SetPosition(mWinCenter);
		mSprite->SetAnchor(mir::math::vec::anchor::Center());
	}break;
	case 5: {
		mSprite = mRendFac->CreateSprite(test1::res::dds::Kenny());//bc1a + mipmap

		test1::res::png::SetPos(mSprite, -mCamWinHSize / 2, mCamWinHSize);
	}break;
	case 6: {
		mSprite = mRendFac->CreateSprite();
		mSprite->SetTexture(mResMng->CreateTextureByFileAsync(test1::res::png::Kenny(), kFormatUnknown, true));//auto_gen_mipmap
	
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