#pragma once
#include "core/mir_export.h"
#include "core/base/declare_macros.h"
#include "core/base/launch.h"
#include "core/rendersys/predeclare.h"
#include "core/renderable/renderable_base.h"

namespace mir {
namespace rend {

struct SkyboxVertex {
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	Eigen::Vector3f Pos;
};

class MIR_CORE_API SkyBox : public RenderableSingleRenderOp
{
	typedef RenderableSingleRenderOp Super;
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	SkyBox(Launch launchMode, ResourceManager& resMng, const res::MaterialInstance& material);
	void SetDiffuseEnvMap(const ITexturePtr& texture);
	void SetLutMap(const ITexturePtr& texture);
public:
	const ITexturePtr& GetDiffuseEnvMap() const { return mDiffuseEnvMap; }
	const ITexturePtr& GetLutMap() const { return mLutMap; }
public:
	void GenRenderOperation(RenderOperationQueue& opList) override;
private:
	ITexturePtr mLutMap, mDiffuseEnvMap;
};

}
}