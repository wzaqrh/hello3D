#pragma once
#include "TApp.h"
#include "TAssimpModel.h"

class TAppLesson1 : public TApp
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
private:
	int mDrawFlag = 0;
	TAssimpModel* mModel = nullptr;
};