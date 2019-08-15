#pragma once
#include "TApp.h"

class TAppLesson0_1 : public TApp
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
	virtual std::string OnCreateRenderSys() override;
private:
	TSpritePtr mSprite;
};

