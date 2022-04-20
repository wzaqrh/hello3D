#pragma once
#include "core/mir_export.h"
#include "core/base/declare_macros.h"
#include "core/base/launch.h"
#include "core/renderable/renderable_base.h"

namespace mir {
namespace rend {

struct PostProcessVertex {
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	Eigen::Vector4f Pos;
	Eigen::Vector2f Tex;
};
struct PostProcessVertexQuad {
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	PostProcessVertexQuad(float x, float y, float w, float h);
	void SetRect(float x, float y, float w, float h);
	void SetFlipY(bool flipY);
	void SetZ(float z);
public:
	PostProcessVertex lb, lt, rt, rb;
};
class MIR_CORE_API PostProcess : public RenderableSingleRenderOp 
{
	INHERIT_RENDERABLE_SINGLE_OP(PostProcess);
	PostProcess(Launch launchMode, ResourceManager& resourceMng, const res::MaterialInstance& material);
public:
	void GenRenderOperation(RenderOperationQueue& ops) override;
};

class MIR_CORE_API PostProcessFactory {
public:
	PostProcessFactory(RenderableFactoryPtr rendFac) :mRendFac(rendFac) {}
	CoTask<PostProcessPtr> CreateGaussianBlur(int radius, std::string matName = "");
	CoTask<PostProcessPtr> CreateAverageBlur(int radius, std::string matName = "");
private:
	RenderableFactoryPtr mRendFac;
};

}
}