#pragma once
#include "TApp.h"
#include "AssimpModel.h"

class Lesson3 : public TApp
{
protected:
	virtual void OnRender() override;
	virtual void OnInitLight() override;
	virtual void OnPostInitDevice() override;
private:
	AssimpModel* mModel = nullptr;
};

