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
	typedef RenderableSingleRenderOp Super;
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	PostProcess(Launch launchMode, ResourceManager& resMng, const res::MaterialInstance& material);
};

class MIR_CORE_API PostProcessFactory {
public:
	PostProcessFactory(RenderableFactoryPtr rendFac) :mRendFac(rendFac) {}
	CoTask<PostProcessPtr> CreateGaussianBlur(int radius, std::string matName = "");
	CoTask<PostProcessPtr> CreateAverageBlur(int radius, std::string matName = "");
	CoTask<PostProcessPtr> CreateSSAO(const scene::Camera& camera);
	CoTask<PostProcessPtr> CreateBloom();
private:
	RenderableFactoryPtr mRendFac;
};

class MIR_CORE_API SSAOBuilder {
public:
	SSAOBuilder(PostProcessPtr effect) :mEffect(effect) { mMat = effect->GetMaterial(); }
	SSAOBuilder& SetAttenuation(float atten);
	SSAOBuilder& SetAngleBias(float biasAngle);
	SSAOBuilder& SetRadius(float radius);
	SSAOBuilder& SetStepNum(int stepNum);
	SSAOBuilder& SetDirNum(int dirNum);
	SSAOBuilder& SetContrast(float contrast);
	SSAOBuilder& SetBlurRadius(float blurRadius);
	SSAOBuilder& SetSharpness (float sharpness);
	PostProcessPtr Build();
private:
	res::MaterialInstance mMat;
	PostProcessPtr mEffect;
};

}
}