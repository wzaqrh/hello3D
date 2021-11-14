#pragma once
#include "core/mir_export.h"
#include "core/rendersys/predeclare.h"
#include "core/renderable/renderable.h"

namespace mir {

struct SkyboxVertex {
	Eigen::Vector4f pos;
};

class MIR_CORE_API SkyBox : public IRenderable 
{
private:
	IRenderSystem& mRenderSys;
	IVertexBufferPtr mVertexBuffer;
	IIndexBufferPtr mIndexBuffer;
public:
	ITexturePtr mCubeSRV;
	MaterialPtr mMaterial;
public:
	SkyBox(IRenderSystem& renderSys, MaterialFactory& matFac, const std::string& imgName);
	~SkyBox();
	virtual int GenRenderOperation(RenderOperationQueue& opList) override;
};

}