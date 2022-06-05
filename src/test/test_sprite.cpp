#include "test/test_case.h"
#include "test/app.h"
#include "core/resource/texture_factory.h"
#include "core/resource/resource_manager.h"
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
	mScneMng->CreateLightNode<DirectLight>();
	mScneMng->CreateCameraNode(kCameraOthogonal);
	SetPPU(mHalfSize.y() * 2 / 10.0);

	Launch sync = __LaunchSync__;
	Launch async = __LaunchAsync__;
	SpritePtr mSprite;
	switch (mCaseIndex) {
	case 0: {
		mSprite = CoAwait mRendFac->CreateSpriteT();
		static std::string filenames[] = {
			"rgba8un.png",//0
			"rgba8un.jpg",//1
			"rgba8un.bmp",//2
			"rgba32f.hdr",//3
			"rgba32f.exr",//4

			"bc2.dds",//5
			"bc2-mip.dds",//6
			"rgba16f.dds",//7
			"rgba16f-mip.dds",//8
			"rgba8un-mip.ktx",//9
		};
		std::string img = "model/image/formats/" + filenames[mCaseSecondIndex];
		mSprite->SetTexture(CoAwait mResMng->CreateTextureByFileT(async, img));
		test1::res::SetPos(mSprite, -mCamWinHSize, mCamWinHSize * 2);
	}break;
	case 1: {
		mSprite = CoAwait mRendFac->CreateSpriteT();
		mSprite->SetTexture(CoAwait mResMng->CreateTextureByFileT(async, test1::res::Image("theyKilledKenny.dds")));
		test1::res::SetPos(mSprite, -mCamWinHSize, mCamWinHSize * 2);

		auto effect = CoAwait PostProcessFactory(mRendFac).CreateGaussianBlur(2);
		mScneMng->GetDefCamera()->AddPostProcessEffect(effect);
		mScneMng->GetDefCamera()->SetRenderingPath(kRenderPathDeffered);
	}break;
	default:
		break;
	}
	mScneMng->AddRendAsNode(mSprite);

	CoReturn true;
}

auto reg = AppRegister<TestSprite>("test_sprite");