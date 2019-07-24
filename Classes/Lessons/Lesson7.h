#pragma once
#include "TApp.h"
#include "AssimpModel.h"
#include "TSprite.h"

class Lesson7 : public TApp
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
	virtual void OnInitLight() override;
private:
	AssimpModel* mModel = nullptr;
	TRenderTexturePtr mRendTexture = nullptr;
	TSpritePtr mSprite, mLayerColor;
};

