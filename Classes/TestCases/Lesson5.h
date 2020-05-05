#pragma once
#include "TApp.h"
#include "TAssimpModel.h"
#include "TSprite.h"

class Lesson5 : public TApp
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
	virtual void OnInitLight() override;
private:
	TAssimpModel* mModel = nullptr;
	IRenderTexturePtr mRendTexture = nullptr;
	TSpritePtr mSprite,mLayerColor;
};

