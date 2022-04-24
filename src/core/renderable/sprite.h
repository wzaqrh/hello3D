#pragma once
#include "core/mir_export.h"
#include "core/base/declare_macros.h"
#include "core/base/launch.h"
#include "core/base/attribute_struct.h"
#include "core/rendersys/predeclare.h"
#include "core/renderable/renderable_base.h"

namespace mir {
namespace rend {

class MIR_CORE_API Sprite : public RenderableSingleRenderOp
{
	friend class RenderPipeline;
	INHERIT_RENDERABLE_SINGLE_OP(Sprite);
public:
	Sprite(Launch launchMode, ResourceManager& resourceMng, const res::MaterialInstance& material);
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	void SetTexture(const ITexturePtr& Texture) override;
	void SetPosition(const Eigen::Vector3f& pos);
	void SetAnchor(const Eigen::Vector3f& anchor);
	void SetSize(const Eigen::Vector3f& size);
	void SetColor(const Eigen::Vector4f& color);
public:
	void GenRenderOperation(RenderOperationQueue& opList) override;
	const vbSurfaceQuad* GetVertexData() const { return &mQuad; }
private:
	vbSurfaceQuad mQuad;
	bool mQuadDirty;
	Eigen::Vector3f mAnchor;
	Eigen::Vector3f mPosition;
	Eigen::Vector3f mSize;
	Eigen::Vector4f mColor;
};

}
}