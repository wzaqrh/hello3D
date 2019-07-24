#pragma once
#include "TApp.h"
#include "AssimpModel.h"
#include "TSprite.h"

struct cbShadowMap
{
	XMMATRIX LightView;
	XMMATRIX LightProjection;
};

class Lesson7 : public TApp
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
	virtual void OnInitLight() override;
private:
	AssimpModel *mModel1, *mModel2 = nullptr;
	TRenderTexturePtr mPass1RT = nullptr;
	TSpritePtr mSecondPass, mLayerColor;
	TPointLightPtr mLight;
	TProgramPtr mProgShadowMap;
};

