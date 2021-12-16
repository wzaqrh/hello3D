#pragma once
#include "core/mir_export.h"
#include "core/base/declare_macros.h"
#include "core/base/launch.h"
#include "core/base/material_load_param.h"
#include "core/renderable/renderable.h"

namespace mir {

struct MIR_CORE_API RenderableSingleRenderOp : public IRenderable
{
	friend class RenderableFactory;
	DECLARE_STATIC_CREATE_CONSTRUCTOR(RenderableSingleRenderOp);
	RenderableSingleRenderOp(Launch launchMode, ResourceManager& resourceMng, const MaterialLoadParam& matName);
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	void SetCameraMask(unsigned mask) { mCameraMask = mask; }
	virtual void SetTexture(const ITexturePtr& Texture);

	unsigned GetCameraMask() const { return mCameraMask; }
	const ITexturePtr& GetTexture() const { return mTexture; }
	const MaterialPtr& GetMaterial() const { return mMaterial; }
	const TransformPtr& GetTransform() const { return mTransform; }
protected:
	bool IsLoaded() const;
	bool MakeRenderOperation(RenderOperation& op);
protected:
	ResourceManager& mResourceMng;
	const Launch mLaunchMode;
	unsigned mCameraMask;
	TransformPtr mTransform;
	MaterialPtr mMaterial;
	ITexturePtr mTexture;
	IVertexBufferPtr mVertexBuffer;
	IIndexBufferPtr mIndexBuffer;
};

}