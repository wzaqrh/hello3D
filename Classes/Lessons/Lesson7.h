#pragma once
#include "TApp.h"
#include "TAssimpModel.h"
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
	TAssimpModel *mModel1, *mModel2 = nullptr;
	TPointLightPtr mLight;
};

