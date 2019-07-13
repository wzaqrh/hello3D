#pragma once
#include "TApp.h"
#include "AssimpModel.h"

class TAppLesson2 : public TApp
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
private:
	void DrawNode(aiNode* node);
private:
	AssimpModel* mModel = nullptr;
	cbWeightedSkin mWeightedSkin;
};

