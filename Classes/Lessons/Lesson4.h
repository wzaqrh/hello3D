#pragma once
#include "TApp.h"
#include "AssimpModel.h"

class Lesson4 : public TApp
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
private:
	AssimpModel* mModel = nullptr;
};

