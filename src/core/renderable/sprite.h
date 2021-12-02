#pragma once
#include "core/mir_export.h"
#include "core/base/declare_macros.h"
#include "core/base/launch.h"
#include "core/base/attribute_struct.h"
#include "core/rendersys/predeclare.h"
#include "core/renderable/renderable.h"

namespace mir {

class MIR_CORE_API Sprite : public IRenderable 
{
	friend class RenderableFactory;
	DECLARE_STATIC_CREATE_CONSTRUCTOR(Sprite);
	Sprite(Launch launchMode, ResourceManager& resourceMng, const std::string& matName = "");
public:
	void SetPosition(const Eigen::Vector3f& pos);
	void SetSize(const Eigen::Vector2f& size);
	void SetTexture(const ITexturePtr& Texture);
	void SetColor(const Eigen::Vector4f& color);
	void SetFlipY(bool flipY);
public:
	int GenRenderOperation(RenderOperationQueue& opList) override;
	const vbSurfaceQuad* GetVertexData() const { return &mQuad; }
	const MaterialPtr& GetMaterial() const { return mMaterial; }
	const TransformPtr& GetTransform() const { return mTransform; }
private:
	ResourceManager& mResourceMng;
	ITexturePtr mTexture = nullptr;
	IVertexBufferPtr mVertexBuffer;
	IIndexBufferPtr mIndexBuffer;
	MaterialPtr mMaterial;
	TransformPtr mTransform;
private:
	vbSurfaceQuad mQuad;
	bool mQuadDirty;
	Eigen::Vector2f mPosition;
	Eigen::Vector2f mSize;
	bool mFlipY;
};

}