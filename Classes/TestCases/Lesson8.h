#pragma once
#include "TApp.h"
#include "TAssimpModel.h"
#include "TSprite.h"

class Lesson8 : public TApp
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
private:
	TAssimpModel* mModel = nullptr;
};