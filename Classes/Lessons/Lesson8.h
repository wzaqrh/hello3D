#pragma once
#include "TApp.h"
#include "AssimpModel.h"
#include "TSprite.h"

class Lesson8 : public TApp
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
private:
	AssimpModel* mModel = nullptr;
};