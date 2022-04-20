#include "core/renderable/post_process.h"
#include "core/resource/resource_manager.h"
#include "core/resource/material.h"
#include "core/base/macros.h"

namespace mir {
namespace rend {

/********** POSTPROCESS_VERTEX_QUAD **********/
PostProcessVertexQuad::PostProcessVertexQuad(float x, float y, float w, float h)
{
	SetRect(x, y, w, h);
	SetFlipY(true);
	SetZ(1);
}

void PostProcessVertexQuad::SetRect(float x, float y, float w, float h)
{
	lb.Pos = Eigen::Vector4f(x, y, 0, 1);
	lt.Pos = Eigen::Vector4f(x, y + h, 0, 1);
	rt.Pos = Eigen::Vector4f(x + w, y + h, 0, 1);
	rb.Pos = Eigen::Vector4f(x + w, y, 0, 1);
}

void PostProcessVertexQuad::SetFlipY(bool flipY)
{
	int pl = 0;
	int pr = 1;
	int pt = 1;
	int pb = 0;
	if (flipY) std::swap(pt, pb);

	lb.Tex = Eigen::Vector2f(pl, pb);
	lt.Tex = Eigen::Vector2f(pl, pt);
	rt.Tex = Eigen::Vector2f(pr, pt);
	rb.Tex = Eigen::Vector2f(pr, pb);
}

void PostProcessVertexQuad::SetZ(float z)
{
	lb.Pos.z() = z;
	lt.Pos.z() = z;
	rt.Pos.z() = z;
	rb.Pos.z() = z;
}

/********** PostProcess **********/
constexpr uint32_t CIndices[] = {
	0, 1, 2, 0, 2, 3
};
PostProcess::PostProcess(Launch launchMode, ResourceManager& resourceMng, const res::MaterialInstance& material)
	: Super(launchMode, resourceMng, material)
{
	mIndexBuffer = resourceMng.CreateIndexBuffer(launchMode, kFormatR32UInt, Data::Make(CIndices));
	
	PostProcessVertexQuad quad(-1, -1, 2, 2);
	mVertexBuffer = resourceMng.CreateVertexBuffer(launchMode, sizeof(PostProcessVertex), 0, Data::Make(quad));
}

void PostProcess::GenRenderOperation(RenderOperationQueue& ops)
{
	RenderOperation op = {};
	if (MakeRenderOperation(op))
		ops.AddOP(op);
}

}
}