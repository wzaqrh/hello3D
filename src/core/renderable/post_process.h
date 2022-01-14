#pragma once
#include "core/mir_export.h"
#include "core/base/declare_macros.h"
#include "core/base/launch.h"
#include "core/renderable/renderable_base.h"

namespace mir {

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
	friend class RenderableFactory;
protected:
	PostProcess(Launch launchMode, ResourceManager& resourceMng, const MaterialLoadParam& matName, IFrameBufferPtr mainTex);
public:
	void GenRenderOperation(RenderOperationQueue& opList) override;
protected:
	IFrameBufferPtr mMainTex;
	std::map<std::pair<PassPtr, int>, IVertexBufferPtr> mVertBufferByPass;
};

}