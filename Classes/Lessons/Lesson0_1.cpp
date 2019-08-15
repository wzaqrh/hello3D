#include "Lesson0_1.h"
#include "TInterfaceType.h"
#include "TSprite.h"
#include "TRenderSystem11.h"
#include "TRenderSystem9.h"

std::string TAppLesson0_1::OnCreateRenderSys()
{
	return "d3d11";
}

void TAppLesson0_1::OnPostInitDevice()
{
#if 0
	mSprite = std::make_shared<TSprite>(mRenderSys, E_MAT_SPRITE);
	mSprite->SetTexture(mRenderSys->GetTexByPath("image\\smile.png"));
	mSprite->SetPosition(-5, -5, 0);
	mSprite->SetSize(5, 5);
#else
	mRenderSys->SetOthogonalCamera(100);

	mSprite = std::make_shared<TSprite>(mRenderSys, E_MAT_LAYERCOLOR);
	mSprite = std::make_shared<TSprite>(mRenderSys, E_MAT_SPRITE);
	mSprite->SetTexture(mRenderSys->GetTexByPath("image\\smile.png"));

	mSprite->SetPosition(-mRenderSys->mScreenWidth / 2, -mRenderSys->mScreenHeight / 2, 0);
	mSprite->SetSize(mRenderSys->mScreenWidth / 2, mRenderSys->mScreenHeight / 2);
#endif
}

void TAppLesson0_1::OnRender()
{
	if (mRenderSys->BeginScene()) {
		mRenderSys->Draw(mSprite.get());
		mRenderSys->EndScene();
	}
}

auto reg = AppRegister<TAppLesson0_1>("TAppLesson0_1: LayerColor");