#pragma once
#include "TApp.h"
#include "TAssimpModel.h"

class Lesson3 : public TApp
{
protected:
	virtual void OnRender() override;
	virtual void OnInitLight() override;
	virtual void OnPostInitDevice() override;
private:
	TAssimpModel* mModel = nullptr;
};

