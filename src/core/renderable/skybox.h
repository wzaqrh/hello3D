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
	TCameraPtr mRefCam;
	IRenderSystem* mRenderSys = nullptr;
	IVertexBufferPtr mVertexBuffer;
	IIndexBufferPtr mIndexBuffer;
public:
	ITexturePtr mCubeSRV;
	TMaterialPtr mMaterial;
public:
	SkyBox(IRenderSystem* pRenderSys, TCameraPtr pCam, const std::string& imgName);
	~SkyBox();
	void SetRefCamera(TCameraPtr pCam);
	virtual int GenRenderOperation(RenderOperationQueue& opList) override;
	void Draw();
};
typedef std::shared_ptr<SkyBox> SkyBoxPtr;

}