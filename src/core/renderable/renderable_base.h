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
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
public:
	RenderableSingleRenderOp(Launch launchMode, ResourceManager& resMng, const res::MaterialInstance& matInst);

	virtual void SetTexture(const ITexturePtr& Texture);
	ITexturePtr GetTexture() const;

	res::MaterialInstance GetMaterial() const { return mMaterial; }
	TransformPtr GetTransform() const { return GetComponent<Transform>(); }

	virtual Eigen::AlignedBox3f GetWorldAABB() const override;
	CoTask<void> UpdateFrame(float dt) override;
	void GenRenderOperation(RenderOperationQueue& ops) override;
	void GetMaterials(std::vector<res::MaterialInstance>& mtls) const override;
protected:
	virtual bool IsMaterialEnabled() const { return true; }
	bool IsLoaded() const;
	RenderOperation* MakeRenderOperation(RenderOperationQueue& ops);
protected:
	ResourceManager& mResMng;
	const Launch mLaunchMode;
	res::MaterialInstance mMaterial;
	IVertexBufferPtr mVertexBuffer;
	IIndexBufferPtr mIndexBuffer;
	Eigen::AlignedBox3f mAABB;
#if MIR_GRAPHICS_DEBUG
public:
	rend::Paint3DPtr mDebugPaint;
#endif
};

}
}