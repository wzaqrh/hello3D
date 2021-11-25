#include "core/renderable/post_process.h"
#include "core/resource/resource_manager.h"
#include "core/rendersys/interface_type.h"
#include "core/resource/material.h"
#include "core/resource/material_factory.h"

namespace mir {

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
PostProcess::PostProcess(Launch launchMode, ResourceManager& resourceMng, IRenderTargetPtr mainTex)
	:mResourceMng(resourceMng)
{
	mMainTex = mainTex;

	mIndexBuffer = mResourceMng.CreateIndexBuffer(launchMode, kFormatR32UInt, Data::Make(CIndices));
	PostProcessVertexQuad quad(-1, -1, 2, 2);
	mVertexBuffer = mResourceMng.CreateVertexBuffer(launchMode, sizeof(PostProcessVertex), 0, Data::Make(quad));
}

PostProcess::~PostProcess()
{
}

int PostProcess::GenRenderOperation(RenderOperationQueue& opList)
{
	if (!mMaterial->IsLoaded() 
		|| !mVertexBuffer->IsLoaded() 
		|| !mIndexBuffer->IsLoaded() 
		|| !mMainTex->IsLoaded())
		return 0;

	RenderOperation op = {};
	op.mMaterial = mMaterial;
	op.mIndexBuffer = mIndexBuffer;
	op.mVertexBuffer = mVertexBuffer;
	op.mTextures.Add(mMainTex->GetColorTexture());
	op.mWorldTransform = Eigen::Matrix4f::Identity();
	op.mVertBufferByPass = mVertBufferByPass;
	opList.AddOP(op);
	return 1;
}

/********** cbBloom **********/
cbBloom cbBloom::CreateDownScale2x2Offsets(int dwWidth, int dwHeight)
{
	cbBloom bloom = {};
	float tU = 1.0f / dwWidth;// / 2.0f;
	float tV = 1.0f / dwHeight;// / 2.0f;
	// Sample from 4 surrounding points. 
	int index = 0;
	for (int y = 0; y < 2; y++) {
		for (int x = 0; x < 2; x++) {
			bloom.SampleOffsets[index].x() = (x - 0.5f) * tU;
			bloom.SampleOffsets[index].y() = (y - 0.5f) * tV;
			index++;
		}
	}
	//return bloom;
	return CreateDownScale3x3Offsets(dwWidth, dwHeight);
}

cbBloom cbBloom::CreateDownScale3x3Offsets(int dwWidth, int dwHeight)
{
	cbBloom bloom = {};
	float tU = 1.0f / dwWidth;
	float tV = 1.0f / dwHeight;
	// Sample from the 9 surrounding points. 
	int index = 0;
	for (int y = -1; y <= 1; y++) {
		for (int x = -1; x <= 1; x++) {
			bloom.SampleOffsets[index].x() = x * tU;
			bloom.SampleOffsets[index].y() = y * tV;
			index++;
		}
	}
	return bloom;
}
inline float GaussianDistribution(float x, float y, float rho)
{
	float g = 1.0f / sqrtf(2.0f * 3.141592654f * rho * rho);
	g *= expf(-(x * x + y * y) / (2 * rho * rho));
	return g;
}

cbBloom cbBloom::CreateBloomOffsets(int dwD3DTexSize, float fDeviation, float fMultiplier)
{
	cbBloom bloom = {};
	int i = 0;
	float tu = 1.0f / (float)dwD3DTexSize;

	// Fill the center texel
	float weight = 1.0f * GaussianDistribution(0, 0, fDeviation);
	bloom.SampleOffsets[0] = Eigen::Vector4f(0.0f, 0, 0, 0);
	bloom.SampleWeights[0] = Eigen::Vector4f(weight, weight, weight, 1.0f);

	// Fill the right side
	for (i = 1; i < 8; i++) {
		weight = fMultiplier * GaussianDistribution((float)i, 0, fDeviation);
		bloom.SampleOffsets[i] = Eigen::Vector4f(i * tu, 0, 0, 0);
		bloom.SampleWeights[i] = Eigen::Vector4f(weight, weight, weight, 1.0f);
	}

	// Copy to the left side
	for (i = 8; i < 15; i++) {
		bloom.SampleOffsets[i] = Eigen::Vector4f(-bloom.SampleOffsets[i - 7].x(), 0, 0, 0);
		bloom.SampleWeights[i] = bloom.SampleWeights[i - 7];
	}
	return bloom;
}

/********** Bloom **********/
IVertexBufferPtr GetVertBufByRT(Launch launchMode, ResourceManager& resourceMng, IRenderTargetPtr target) {
	auto srv = target->GetColorTexture();
	float sx = srv->GetWidth() * 1.0 / resourceMng.WinSize().x();
	float sy = srv->GetHeight() * 1.0 / resourceMng.WinSize().y();
	assert(sx <= 1 && sy <= 1);
	PostProcessVertexQuad quad(-1, 1.0 - 2 * sy, 2 * sx, 2 * sy);
	return resourceMng.CreateVertexBuffer(launchMode, sizeof(PostProcessVertex), 0, Data::Make(quad));
}

Bloom::Bloom(Launch launchMode, ResourceManager& resourceMng, IRenderTargetPtr mainTex)
	:PostProcess(launchMode, resourceMng, mainTex)
{
	mMaterial = resourceMng.CreateMaterial(launchMode, E_MAT_POSTPROC_BLOOM);

	auto curTech = mMaterial->CurTech();
	for (auto& pass : curTech->mPasses) {
		if (pass->mRenderTarget) {
			mVertBufferByPass.insert(std::make_pair(std::make_pair(pass, -1), GetVertBufByRT(launchMode, mResourceMng, pass->mRenderTarget)));
			for (int i = 0; i < pass->mRTIterators.size(); ++i) {
				mVertBufferByPass.insert(std::make_pair(std::make_pair(pass, i), GetVertBufByRT(launchMode, mResourceMng, pass->mRTIterators[i])));
			}
		}
	}
}

}