#pragma once
#include "core/mir_export.h"
#include "core/base/declare_macros.h"
#include "core/base/launch.h"
#include "core/base/attribute_struct.h"
#include "core/rendersys/predeclare.h"
#include "core/renderable/renderable_base.h"

namespace mir {

class MIR_CORE_API Sprite : public RenderableSingleRenderOp 
{
	typedef RenderableSingleRenderOp Super;
	friend class RenderableFactory;
	DECLARE_STATIC_CREATE_CONSTRUCTOR(Sprite);
	Sprite(Launch launchMode, ResourceManager& resourceMng, const MaterialLoadParam& matName);
public:
	void SetTexture(const ITexturePtr& Texture) override;
	void SetPosition(const Eigen::Vector3f& pos);
	void SetSize(const Eigen::Vector2f& size);
	void SetColor(const Eigen::Vector4f& color);
	void SetFlipY(bool flipY);
public:
	void GenRenderOperation(RenderOperationQueue& opList) override;
	const vbSurfaceQuad* GetVertexData() const { return &mQuad; }
private:
	vbSurfaceQuad mQuad;
	bool mQuadDirty;
	Eigen::Vector2f mPosition;
	Eigen::Vector2f mSize;
	bool mFlipY;
};

}