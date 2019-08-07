#pragma once
#include "TApp.h"
#include "AssimpModel.h"

class TAppLesson2 : public TApp
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
	virtual void OnInitLight() override;
private:
	AssimpModel* mModel = nullptr;
};

