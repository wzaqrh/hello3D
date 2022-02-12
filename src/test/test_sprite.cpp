#include "test/test_case.h"
#include "test/app.h"
#include "core/renderable/sprite.h"

using namespace mir;
using namespace mir::rend;

class TestSprite : public App
{
protected:
	CoTask<bool> OnInitScene() override;
	void OnInitLight() override {}
	void OnInitCamera() override {}
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

CoTask<bool> TestSprite::OnInitScene()
{
	mScneMng->CreateAddLightNode<DirectLight>();
	mScneMng->CreateAddCameraNode(kCameraOthogonal, test1::cam_otho::Eye(mWinCenter));
	SetPPU(C_WINDOW_HEIGHT / 10.0);

	Launch sync = __LaunchSync__;
	Launch async = __LaunchAsync__;
	SpritePtr mSprite;
	switch (mCaseIndex) {
	case 0: {
		mSprite = CoAwait mRendFac->CreateSprite();
		//mSprite->SetTexture(mResMng->CreateTextureByFile(sync, test1::res::Sky(), kFormatR32G32B32A32Float));
		mSprite->SetTexture(CoAwait mResMng->CreateTextureByFileAsync(test1::res::dds::Kenny()));
		test1::res::png::SetPos(mSprite, -mCamWinHSize, mCamWinHSize * 2);
	}break;
	case 1: {
		mSprite = CoAwait mRendFac->CreateSprite();
		mSprite->SetTexture(mResMng->CreateTextureByFileSync(test1::res::png::Kenny(), kFormatUnknown, true));//auto_gen_mipmap
	
		test1::res::png::SetPos(mSprite, mCamWinHSize, mCamWinHSize, math::vec::anchor::RightTop());
	}break;
	case 2: {
		mSprite = CoAwait mRendFac->CreateSprite(test1::res::png::Kenny());

		test1::res::png::SetPos(mSprite, mWinCenter, mCamWinHSize * 2, math::vec::anchor::Center());
	}break;
	case 3: {
		mSprite = CoAwait mRendFac->CreateSprite(test1::res::hdr::Kenny());//zlib

		test1::res::png::SetPos(mSprite, Eigen::Vector3f::Zero(), mCamWinHSize, math::vec::anchor::LeftTop());
	}break;
	case 4: {
		mSprite = CoAwait mRendFac->CreateSprite(test1::res::dds::Lenna());//bc1a

		mSprite->SetPosition(mWinCenter);
		mSprite->SetAnchor(math::vec::anchor::Center());
	}break;
	case 5: {
		mSprite = CoAwait mRendFac->CreateSprite(test1::res::dds::Kenny());//bc1a + mipmap

		test1::res::png::SetPos(mSprite, -mCamWinHSize / 2, mCamWinHSize);
	}break;
	case 6: {
		mSprite = CoAwait mRendFac->CreateSprite();
		mSprite->SetTexture(CoAwait mResMng->CreateTextureByFileAsync(test1::res::png::Kenny(), kFormatUnknown, true));//auto_gen_mipmap
	
		mSprite->SetPosition(-mCamWinHSize);
	}break;
	default:
		break;
	}
	mScneMng->AddRendNode(mSprite);

	CoReturn true;
}

auto reg = AppRegister<TestSprite>("test_sprite");