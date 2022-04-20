#include <boost/math/constants/constants.hpp>
#include "core/renderable/post_process.h"
#include "core/renderable/renderable_factory.h"
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

/********** GaussianBlurBuilder **********/
//https://medium.com/@aryamansharda/image-filters-gaussian-blur-eb36db6781b1
CoTask<PostProcessPtr> PostProcessFactory::CreateGaussianBlur(int radius, std::string matName)
{
	radius = __max(radius, 1);
	int kernelSize = 0;
	std::vector<float> weights;
	{
		float sigma = __max(radius / 2, 1);
		kernelSize = (radius * 2) + 1;
		weights.resize(kernelSize * kernelSize);

		float sum = 0.0f;
		for (int x = -radius; x <= radius; ++x) {
			for (int y = -radius; y <= radius; ++y) {
				float expNum = -(x * x + y * y);
				float expDenom = 2 * sigma * sigma;
				float eExpression = exp(expNum / expDenom);
				float kernelValue = (eExpression / (2 * boost::math::constants::pi<float>() * sigma * sigma));
				weights[x + radius + (y + radius) * kernelSize] = kernelValue;
				sum += kernelValue;
			}
		}
		for (size_t i = 0; i < weights.size(); ++i)
			weights[i] /= sum;
	}
	MaterialLoadParamBuilder param = IF_AND_OR(matName.empty(), MAT_BOX_BLUR, matName.c_str());
	param["BOX_KERNEL_SIZE"] = kernelSize;
	auto filter = CoAwait mRendFac->CreatePostProcessEffectT(param);
	filter->GetMaterial().SetProperty("BoxKernelWeights", Data::Make(weights));

	CoReturn filter;
}

CoTask<mir::rend::PostProcessPtr> PostProcessFactory::CreateAverageBlur(int radius, std::string matName /*= ""*/)
{
	radius = __max(radius, 1);
	int kernelSize = 0;
	std::vector<float> weights;
	{
		kernelSize = (radius * 2) + 1;
		weights.resize(kernelSize * kernelSize);

		for (int x = -radius; x <= radius; ++x) {
			for (int y = -radius; y <= radius; ++y) {
				weights[x + radius + (y + radius) * kernelSize] = 1.0f / weights.size();
			}
		}
	}
	MaterialLoadParamBuilder param = IF_AND_OR(matName.empty(), MAT_BOX_BLUR, matName.c_str());
	param["BOX_KERNEL_SIZE"] = kernelSize;
	auto filter = CoAwait mRendFac->CreatePostProcessEffectT(param);
	filter->GetMaterial().SetProperty("BoxKernelWeights", Data::Make(weights));

	CoReturn filter;
}

}
}