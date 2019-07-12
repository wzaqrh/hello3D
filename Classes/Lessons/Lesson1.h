#pragma once
#include "TBaseTypes.h"
#include "TMesh.h"
#include "TApp.h"

const int MAX_MATRICES = 256;
struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
	XMMATRIX mModel;
	XMMATRIX Models[MAX_MATRICES];
public:
	ConstantBuffer();
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
	ConstantBuffer mConstBuf;
};