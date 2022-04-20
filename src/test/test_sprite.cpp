#include "test/test_case.h"
#include "test/app.h"
#include "core/renderable/sprite.h"
#include "core/renderable/post_process.h"

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
0：观察到kenny占整个屏幕
1: 观察到kenny占右上半区
2：观察到kenny占整个屏幕
3: 观察到kenny占右下半区
4：观察到lenna左下角在屏幕中心, 自然尺寸，有透明
5：观察到kenny屏幕居中, 半个屏幕尺寸
6：观察到kenny左下角, 自然尺寸
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
		mSprite = CoAwait mRendFac->CreateSpriteT();
		//mSprite->SetTexture(mResMng->CreateTextureByFile(sync, test1::res::Sky(), kFormatR32G32B32A32Float));
		mSprite->SetTexture(CoAwait mResMng->CreateTextureByFileT(async, test1::res::dds::Kenny()));
		test1::res::png::SetPos(mSprite, -mCamWinHSize, mCamWinHSize * 2);

		MaterialLoadParamBuilder param = MAT_BOX_BLUR;
		constexpr int kernelSize = 5;
		param["BOX_KERNEL_SIZE"] = kernelSize;
		auto effect = CoAwait mRendFac->CreatePostProcessEffectT(param);
		std::array<float, kernelSize * kernelSize> weights;
		for (size_t i = 0; i < weights.size(); ++i)
			weights[i] = 1.0 / weights.size();
		effect->GetMaterial().SetProperty("BoxKernelWeights", weights);

		mScneMng->GetDefCamera()->AddPostProcessEffect(effect);
		mScneMng->GetDefCamera()->SetRenderingPath(kRenderPathDeffered);
	}break;
	case 1: {
		mSprite = CoAwait mRendFac->CreateSpriteT();
		mSprite->SetTexture(mResMng->CreateTextureByFileS(async, test1::res::png::Kenny(), kFormatUnknown, true));//auto_gen_mipmap
	
		test1::res::png::SetPos(mSprite, mCamWinHSize, mCamWinHSize, math::vec::anchor::RightTop());
	}break;
	case 2: {
		mSprite = CoAwait mRendFac->CreateSpriteT(test1::res::png::Kenny());

		test1::res::png::SetPos(mSprite, mWinCenter, mCamWinHSize * 2, math::vec::anchor::Center());
	}break;
	case 3: {
		mSprite = CoAwait mRendFac->CreateSpriteT(test1::res::hdr::Kenny());//zlib

		test1::res::png::SetPos(mSprite, Eigen::Vector3f::Zero(), mCamWinHSize, math::vec::anchor::LeftTop());
	}break;
	case 4: {
		mSprite = CoAwait mRendFac->CreateSpriteT(test1::res::dds::Lenna());//bc1a

		mSprite->SetPosition(mWinCenter);
		mSprite->SetAnchor(math::vec::anchor::Center());
	}break;
	case 5: {
		mSprite = CoAwait mRendFac->CreateSpriteT(test1::res::dds::Kenny());//bc1a + mipmap

		test1::res::png::SetPos(mSprite, -mCamWinHSize / 2, mCamWinHSize);
	}break;
	case 6: {
		mSprite = CoAwait mRendFac->CreateSpriteT();
		mSprite->SetTexture(CoAwait mResMng->CreateTextureByFileT(async, test1::res::png::Kenny(), kFormatUnknown, true));//auto_gen_mipmap
	
		mSprite->SetPosition(-mCamWinHSize);
	}break;
	default:
		break;
	}
	mScneMng->AddRendNode(mSprite);

	CoReturn true;
}

auto reg = AppRegister<TestSprite>("test_sprite");