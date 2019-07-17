#pragma once
#include "TApp.h"
#include "AssimpModel.h"
#include "TSprite.h"

class Lesson5 : public TApp
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
private:
	AssimpModel* mModel = nullptr;
	TRenderTexturePtr mRendTexture = nullptr;
	TSpritePtr mSprite,mLayerColor;
};

