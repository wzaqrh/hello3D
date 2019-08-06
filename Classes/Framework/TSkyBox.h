#pragma once
#include "IRenderable.h"
#include "TPredefine.h"

struct SKYBOX_VERTEX
{
	XMFLOAT4 pos;
};

class TSkyBox 
	: public IRenderable
{
private:
	TCameraPtr mRefCam;
	TRenderSystem* mRenderSys = nullptr;
	TVertexBufferPtr mVertexBuffer;
	TIndexBufferPtr mIndexBuffer;
public:
	TTexturePtr mCubeSRV;
	TMaterialPtr mMaterial;
public:
	TSkyBox(TRenderSystem* pRenderSys, TCameraPtr pCam, const std::string& imgName);
	~TSkyBox();
	void SetRefCamera(TCameraPtr pCam);
	virtual int GenRenderOperation(TRenderOperationQueue& opList) override;
	void Draw();
};
typedef std::shared_ptr<TSkyBox> TSkyBoxPtr;