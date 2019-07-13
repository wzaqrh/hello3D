#pragma once
#include "TBaseTypes.h"
#include "TMesh.h"
#include "TApp.h"

const int MAX_MATRICES = 256;
struct cbWeightedSkin
{
	XMMATRIX mModel;
	XMMATRIX Models[MAX_MATRICES];
public:
	cbWeightedSkin();
};

class AssimpModel;
struct aiNode;
class TAppLesson1 : public TApp
{
protected:
	virtual void OnRender() override;
	virtual void OnPreInitDevice() override;
	virtual void OnPostInitDevice() override;
private:
	void DrawNode(aiNode* node);
	void DrawMesh(TMesh& mesh);
private:
	int mDrawFlag = 0;
	AssimpModel* mModel = nullptr;
	cbWeightedSkin mWeightedSkin;
};