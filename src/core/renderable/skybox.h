#pragma once
#include "core/rendersys/predeclare.h"
#include "core/renderable/renderable.h"

namespace mir {

struct SkyboxVertex {
	XMFLOAT4 pos;
};

class SkyBox : public IRenderable 
{
private:
	CameraPtr mRefCam;
	IRenderSystem& mRenderSys;
	IVertexBufferPtr mVertexBuffer;
	IIndexBufferPtr mIndexBuffer;
public:
	ITexturePtr mCubeSRV;
	MaterialPtr mMaterial;
public:
	SkyBox(IRenderSystem& renderSys, MaterialFactory& matFac, CameraPtr pCam, const std::string& imgName);
	~SkyBox();
	void SetRefCamera(CameraPtr pCam);
	virtual int GenRenderOperation(RenderOperationQueue& opList) override;
};

}