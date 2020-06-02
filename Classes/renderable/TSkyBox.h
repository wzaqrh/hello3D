#pragma once
#include "IRenderable.h"
#include "ISceneManagerPred.h"
#include "TInterfaceType.h"

struct SKYBOX_VERTEX
{
	XMFLOAT4 pos;
};

class TSkyBox 
	: public IRenderable
{
private:
	TCameraPtr mRefCam;
	IRenderSystem* mRenderSys = nullptr;
	IVertexBufferPtr mVertexBuffer;
	IIndexBufferPtr mIndexBuffer;
public:
	ITexturePtr mCubeSRV;
	TMaterialPtr mMaterial;
public:
	TSkyBox(IRenderSystem* pRenderSys, TCameraPtr pCam, const std::string& imgName);
	~TSkyBox();
	void SetRefCamera(TCameraPtr pCam);
	virtual int GenRenderOperation(TRenderOperationQueue& opList) override;
	void Draw();
};
typedef std::shared_ptr<TSkyBox> TSkyBoxPtr;