#pragma once
#include "core/renderable/renderable.h"
#include "core/rendersys/scene_manager_pred.h"
#include "core/rendersys/interface_type.h"

namespace mir {

struct SkyboxVertex {
	XMFLOAT4 pos;
};

class SkyBox : public IRenderable {
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
	void Draw();
};
typedef std::shared_ptr<SkyBox> SkyBoxPtr;

}