#pragma once
#include "core/mir_export.h"
#include "core/base/cppcoro.h"
#include "core/base/declare_macros.h"
#include "core/base/launch.h"
#include "core/base/material_load_param.h"
#include "core/renderable/renderable.h"

namespace mir {
namespace rend {

struct MIR_CORE_API RenderableSingleRenderOp : public Renderable
{
#define INHERIT_RENDERABLE_SINGLE_OP(CLS) DECLARE_STATIC_TASK_CREATE_CONSTRUCTOR(CLS, Launch, ResourceManager&, const res::MaterialInstance& matInst); typedef RenderableSingleRenderOp Super; friend class RenderableFactory
#define INHERIT_RENDERABLE_SINGLE_OP_CONSTRUCTOR(CLS) INHERIT_RENDERABLE_SINGLE_OP(CLS); CLS(Launch launchMode, ResourceManager& resourceMng, const res::MaterialInstance& matInst) :Super(launchMode, resourceMng, matInst) {}
	INHERIT_RENDERABLE_SINGLE_OP(RenderableSingleRenderOp);
	RenderableSingleRenderOp(Launch launchMode, ResourceManager& resourceMng, const res::MaterialInstance& matInst);
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	virtual void SetTexture(const ITexturePtr& Texture);
	
	const ITexturePtr& GetTexture() const;
	const res::MaterialInstance& GetMaterial() const { return mMaterial; }
	const TransformPtr& GetTransform() const { return mTransform; }
protected:
	virtual bool IsMaterialEnabled() const { return true; }
	bool IsLoaded() const;
	bool MakeRenderOperation(RenderOperation& op);
protected:
	ResourceManager& mResourceMng;
	const Launch mLaunchMode;
	TransformPtr mTransform;
	res::MaterialInstance mMaterial;
	IVertexBufferPtr mVertexBuffer;
	IIndexBufferPtr mIndexBuffer;
};

}
}