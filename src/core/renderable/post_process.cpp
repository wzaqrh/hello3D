#include "core/renderable/post_process.h"
#include "core/rendersys/resource_manager.h"
#include "core/rendersys/interface_type.h"
#include "core/rendersys/material.h"
#include "core/rendersys/material_cb.h"
#include "core/rendersys/material_factory.h"

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
constexpr unsigned int CIndices[] = {
	0, 1, 2, 0, 2, 3
};
PostProcess::PostProcess(Launch launchMode, ResourceManager& resourceMng, IRenderTexturePtr mainTex)
	:mResourceMng(resourceMng)
{
	mMainTex = mainTex;

	mIndexBuffer = mResourceMng.CreateIndexBuffer(launchMode, sizeof(CIndices), kFormatR32UInt, (void*)&CIndices[0]);
	PostProcessVertexQuad quad(-1, -1, 2, 2);
	mVertexBuffer = mResourceMng.CreateVertexBuffer(launchMode, sizeof(PostProcessVertexQuad), sizeof(PostProcessVertex), 0, &quad);
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

/********** TBloom **********/
IVertexBufferPtr GetVertBufByRT(Launch launchMode, ResourceManager& resourceMng, IRenderTexturePtr target) {
	auto srv = target->GetColorTexture();
	float sx = srv->GetWidth() * 1.0 / resourceMng.WinSize().x();
	float sy = srv->GetHeight() * 1.0 / resourceMng.WinSize().y();
	assert(sx <= 1 && sy <= 1);
	PostProcessVertexQuad quad(-1, 1.0 - 2 * sy, 2 * sx, 2 * sy);
	IVertexBufferPtr vertBuf = resourceMng.CreateVertexBuffer(launchMode, sizeof(PostProcessVertexQuad), sizeof(PostProcessVertex), 0, &quad);
	return vertBuf;
}

Bloom::Bloom(Launch launchMode, ResourceManager& resourceMng, IRenderTexturePtr mainTex)
	:PostProcess(launchMode, resourceMng, mainTex)
{
	mMaterial = resourceMng.CreateMaterial(launchMode, E_MAT_POSTPROC_BLOOM);

	auto curTech = mMaterial->CurTech();
	for (auto& pass : curTech->mPasses) {
		if (pass->mRenderTarget) {
			mVertBufferByPass.insert(std::make_pair(std::make_pair(pass, -1), GetVertBufByRT(launchMode, mResourceMng, pass->mRenderTarget)));
			for (int i = 0; i < pass->mIterTargets.size(); ++i) {
				mVertBufferByPass.insert(std::make_pair(std::make_pair(pass, i), GetVertBufByRT(launchMode, mResourceMng, pass->mIterTargets[i])));
			}
		}
	}
}

}