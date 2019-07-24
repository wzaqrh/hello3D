#pragma once
#include "TApp.h"
#include "AssimpModel.h"
#include "TSprite.h"

#if 1
class Lesson6 : public TApp
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
private:
	AssimpModel* mModel = nullptr;
};
#else
class Lesson6 : public TApp
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
private:
	TSpritePtr mLayerColor;
};
#endif

