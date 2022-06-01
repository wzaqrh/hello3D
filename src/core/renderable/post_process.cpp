#include <boost/math/constants/constants.hpp>
#include "core/renderable/post_process.h"
#include "core/renderable/renderable_factory.h"
#include "core/resource/resource_manager.h"
#include "core/resource/material_name.h"
#include "core/resource/material.h"
#include "core/base/macros.h"
#include "core/base/debug.h"
#include "core/scene/camera.h"

namespace mir {
namespace rend {

/********** PostProcessVertexQuad **********/
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

/********** PostProcessFactory **********/
//https://medium.com/@aryamansharda/image-filters-gaussian-blur-eb36db6781b1
CoTask<PostProcessPtr> PostProcessFactory::CreateGaussianBlur(int radius, std::string matName)
{
	COROUTINE_VARIABLES_2(radius, matName);

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
	param["PASS_ITERATOR_COUNT"] = 32;
	auto filter = CoAwait mRendFac->CreatePostProcessEffectT(param);
	filter->GetMaterial().SetProperty("BoxKernelWeights", Data::Make(weights));

	CoReturn filter;
}

CoTask<mir::rend::PostProcessPtr> PostProcessFactory::CreateAverageBlur(int radius, std::string matName /*= ""*/)
{
	COROUTINE_VARIABLES_2(radius, matName);

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

CoTask<mir::rend::PostProcessPtr> PostProcessFactory::CreateBloom()
{
	MaterialLoadParamBuilder param = MAT_BLOOM;
	auto filter = CoAwait mRendFac->CreatePostProcessEffectT(param);
	CoReturn filter;
}

CoTask<mir::rend::PostProcessPtr> PostProcessFactory::CreateSSAO(const scene::Camera& camera)
{
	float fov = camera.GetFov();
	float aspect = camera.GetAspect();
	const Eigen::Vector2f& nf = camera.GetClippingPlane(); float n = nf.x(), f = nf.y();
	MaterialLoadParamBuilder param = MAT_SSAO;
	auto filter = CoAwait mRendFac->CreatePostProcessEffectT(param);

	float invFocalLenY = tan(fov * 0.5f);
	float invFocalLenX = invFocalLenY / aspect;
	filter->GetMaterial().SetProperty("DepthParam", Eigen::Vector4f(1/n, (n-f)/(f*n), 0, 0));
	filter->GetMaterial().SetProperty("FocalLen", Eigen::Vector4f(1.0f / invFocalLenX, 1.0f / invFocalLenY, invFocalLenX, invFocalLenY));
	filter->GetMaterial().SetProperty("AttenTanBias", Eigen::Vector4f(1.0, tanf(30.0f / 180 * boost::math::constants::pi<float>()), 0.0, 0.0));
	CoReturn filter;
}

/********** SSAOBuilder **********/
static void SetPropVectorAt(res::MaterialInstance& mat, const char* propName, int pos, float value) {
	Eigen::Vector4f varProp = mat.GetProperty<Eigen::Vector4f>(propName);
	varProp[pos] = value;
	mat.SetProperty(propName, varProp);
}
SSAOBuilder& SSAOBuilder::SetAttenuation(float atten)
{
	mMat.SetPropertyVec4At("AttenTanBias", 0, atten);
	return *this;
}
SSAOBuilder& SSAOBuilder::SetAngleBias(float biasAngle)
{
	mMat.SetPropertyVec4At("AttenTanBias", 1, tanf(biasAngle / 180 * boost::math::constants::pi<float>()));
	return *this;
}
SSAOBuilder& SSAOBuilder::SetRadius(float radius)
{
	float rSq = radius * radius;
	mMat.SetProperty("Radius", Eigen::Vector4f(radius, rSq, 1.0f / radius, 1.0f / rSq));

	float sharpness = mMat.GetProperty<Eigen::Vector4f>("BlurRadiusFallOffSharp")[3];
	float sds = sharpness / radius;
	mMat.SetPropertyVec4At("BlurRadiusFallOffSharp", 2, sds * sds);
	return *this;
}
SSAOBuilder& SSAOBuilder::SetStepNum(int stepNum)
{
	mMat.SetPropertyVec4At("NumStepDirContrast", 0, stepNum);
	return *this;
}
SSAOBuilder& SSAOBuilder::SetDirNum(int dirNum)
{
	mMat.SetPropertyVec4At("NumStepDirContrast", 1, dirNum);
	return *this;
}
SSAOBuilder& SSAOBuilder::SetContrast(float contrast)
{
	mMat.SetPropertyVec4At("NumStepDirContrast", 2, contrast);
	return *this;
}
SSAOBuilder& SSAOBuilder::SetBlurRadius(float blurRadius)
{
	float sigma = (blurRadius + 1) / 2;
	float blurFalloff = 1.0f / (2 * sigma * sigma);
	mMat.SetPropertyVec4At("BlurRadiusFallOffSharp", 0, blurRadius);
	mMat.SetPropertyVec4At("BlurRadiusFallOffSharp", 1, blurFalloff);
	return *this;
}
SSAOBuilder& SSAOBuilder::SetSharpness(float sharpness)
{
	float sds = sharpness * mMat.GetProperty<Eigen::Vector4f>("Radius")[2];
	mMat.SetPropertyVec4At("BlurRadiusFallOffSharp", 2, sds * sds);
	mMat.SetPropertyVec4At("BlurRadiusFallOffSharp", 3, sharpness);
	return *this;
}
PostProcessPtr SSAOBuilder::Build()
{
	return mEffect;
}

}
}