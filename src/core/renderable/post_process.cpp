#include "core/renderable/post_process.h"
#include "core/rendersys/render_system.h"
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
	lb.Pos = XMFLOAT4(x, y, 0, 1);
	lt.Pos = XMFLOAT4(x, y + h, 0, 1);
	rt.Pos = XMFLOAT4(x + w, y + h, 0, 1);
	rb.Pos = XMFLOAT4(x + w, y, 0, 1);
}

void PostProcessVertexQuad::SetFlipY(bool flipY)
{
	int pl = 0;
	int pr = 1;
	int pt = 1;
	int pb = 0;

	if (flipY) std::swap(pt, pb);

	lb.Tex = XMFLOAT2(pl, pb);
	lt.Tex = XMFLOAT2(pl, pt);
	rt.Tex = XMFLOAT2(pr, pt);
	rb.Tex = XMFLOAT2(pr, pb);
}

void PostProcessVertexQuad::SetZ(float z)
{
	lb.Pos.z = z;
	lt.Pos.z = z;
	rt.Pos.z = z;
	rb.Pos.z = z;
}

const unsigned int indices[] = {
	0, 1, 2, 0, 2, 3
};

/********** TPostProcess **********/
PostProcess::PostProcess(IRenderSystem& RenderSys, IRenderTexturePtr mainTex)
	:mRenderSys(RenderSys) 
{
	mMainTex = mainTex;

	mIndexBuffer = mRenderSys.CreateIndexBuffer(sizeof(indices), DXGI_FORMAT_R32_UINT, (void*)&indices[0]);
	PostProcessVertexQuad quad(-1, -1, 2, 2);
	mVertexBuffer = mRenderSys.CreateVertexBuffer(sizeof(PostProcessVertexQuad), sizeof(PostProcessVertex), 0, &quad);
}

PostProcess::~PostProcess()
{
}

int PostProcess::GenRenderOperation(RenderOperationQueue& opList)
{
	RenderOperation op = {};
	op.mMaterial = mMaterial;
	op.mIndexBuffer = mIndexBuffer;
	op.mVertexBuffer = mVertexBuffer;
	op.mTextures.Add(mMainTex->GetColorTexture());
	op.mWorldTransform = XMMatrixIdentity();
	op.mVertBufferByPass = mVertBufferByPass;
	opList.AddOP(op);
	return 1;
}

/********** TBloom **********/
IVertexBufferPtr GetVertBufByRT(IRenderSystem& RenderSys, IRenderTexturePtr target) {
	auto srv = target->GetColorTexture();
	float sx = srv->GetWidth() * 1.0 / RenderSys.GetWinSize().x;
	float sy = srv->GetHeight() * 1.0 / RenderSys.GetWinSize().y;
	assert(sx <= 1 && sy <= 1);
	PostProcessVertexQuad quad(-1, 1.0 - 2 * sy, 2 * sx, 2 * sy);
	IVertexBufferPtr vertBuf = RenderSys.CreateVertexBuffer(sizeof(PostProcessVertexQuad), sizeof(PostProcessVertex), 0, &quad);
	return vertBuf;
}

Bloom::Bloom(IRenderSystem& renderSys, MaterialFactory& matFac, IRenderTexturePtr mainTex)
	:PostProcess(renderSys, mainTex)
{
	mMaterial = matFac.GetMaterial(E_MAT_POSTPROC_BLOOM);

	auto curTech = mMaterial->CurTech();
	for (auto& pass : curTech->mPasses) {
		if (pass->mRenderTarget) {
			mVertBufferByPass.insert(std::make_pair(std::make_pair(pass, -1), GetVertBufByRT(mRenderSys, pass->mRenderTarget)));
			for (int i = 0; i < pass->mIterTargets.size(); ++i) {
				mVertBufferByPass.insert(std::make_pair(std::make_pair(pass, i), GetVertBufByRT(mRenderSys, pass->mIterTargets[i])));
			}
		}
	}
}

}