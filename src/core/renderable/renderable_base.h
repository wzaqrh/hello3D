#pragma once
#include "core/mir_export.h"
#include "core/base/cppcoro.h"
#include "core/base/declare_macros.h"
#include "core/base/launch.h"
#include "core/base/material_load_param.h"
#include "core/renderable/renderable.h"

namespace mir {
namespace rend {

struct MIR_CORE_API RenderableSingleRenderOp : public IRenderable
{
#define INHERIT_RENDERABLE_SINGLE_OP(CLS) DECLARE_STATIC_TASK_CREATE_CONSTRUCTOR(CLS, Launch, ResourceManager&); typedef RenderableSingleRenderOp Super; friend class RenderableFactory
#define INHERIT_RENDERABLE_SINGLE_OP_CONSTRUCTOR(CLS) INHERIT_RENDERABLE_SINGLE_OP(CLS); CLS(Launch launchMode, ResourceManager& resourceMng) :Super(launchMode, resourceMng) {}
	INHERIT_RENDERABLE_SINGLE_OP(RenderableSingleRenderOp);
	RenderableSingleRenderOp(Launch launchMode, ResourceManager& resourceMng);
	CoTask<bool> Init(const MaterialLoadParam& matName);
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	void SetCameraMask(unsigned mask) { mCameraMask = mask; }
	virtual void SetTexture(const ITexturePtr& Texture);

	unsigned GetCameraMask() const { return mCameraMask; }
	const ITexturePtr& GetTexture() const;
	const res::MaterialInstance& GetMaterial() const { return mMaterial; }
	const TransformPtr& GetTransform() const { return mTransform; }
protected:
	bool IsLoaded() const;
	bool MakeRenderOperation(RenderOperation& op);
protected:
	ResourceManager& mResourceMng;
	const Launch mLaunchMode;
	unsigned mCameraMask;
	TransformPtr mTransform;
	res::MaterialInstance mMaterial;
	IVertexBufferPtr mVertexBuffer;
	IIndexBufferPtr mIndexBuffer;
};

}
}