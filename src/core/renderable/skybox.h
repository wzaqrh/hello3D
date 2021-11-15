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
	friend class RenderableFactory;
	DECLARE_STATIC_CREATE_CONSTRUCTOR(SkyBox);
	SkyBox(IRenderSystem& renderSys, MaterialFactory& matFac, const std::string& imgName);
public:
	~SkyBox();
	int GenRenderOperation(RenderOperationQueue& opList) override;
	const ITexturePtr& GetTexture() const { return mCubeSRV; }
	const MaterialPtr& GetMaterial() const { return mMaterial; }
private:
	IRenderSystem& mRenderSys;
	IVertexBufferPtr mVertexBuffer;
	IIndexBufferPtr mIndexBuffer;
	ITexturePtr mCubeSRV;
	MaterialPtr mMaterial;
};

}