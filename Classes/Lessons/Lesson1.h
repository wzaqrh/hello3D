#pragma once
#include "TApp.h"
#include "AssimpModel.h"

class TAppLesson1 : public TApp
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
private:
	int mDrawFlag = 0;
	AssimpModel* mModel = nullptr;
};