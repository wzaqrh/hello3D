#include "core/renderable/skybox.h"
#include "core/rendersys/render_system.h"
#include "core/rendersys/material_factory.h"

namespace mir {

SkyBox::SkyBox(IRenderSystem* pRenderSys, TCameraPtr pCam, const std::string& imgName)
{
	mRenderSys = pRenderSys;
	mRefCam = pCam;

	mMaterial = mRenderSys->GetMaterial(E_MAT_SKYBOX);
	mIndexBuffer = nullptr;

	SkyboxVertex Vertexs[4];
	float fHighW = -1.0f - (1.0f / (float)pRenderSys->GetWinSize().x);
	float fHighH = -1.0f - (1.0f / (float)pRenderSys->GetWinSize().y);
	float fLowW = 1.0f + (1.0f / (float)pRenderSys->GetWinSize().x);
	float fLowH = 1.0f + (1.0f / (float)pRenderSys->GetWinSize().y);
	Vertexs[0].pos = XMFLOAT4(fLowW, fLowH, 1.0f, 1.0f);
	Vertexs[1].pos = XMFLOAT4(fLowW, fHighH, 1.0f, 1.0f);
	Vertexs[2].pos = XMFLOAT4(fHighW, fLowH, 1.0f, 1.0f);
	Vertexs[3].pos = XMFLOAT4(fHighW, fHighH, 1.0f, 1.0f);
	mVertexBuffer = mRenderSys->CreateVertexBuffer(sizeof(SkyboxVertex) * 4, sizeof(SkyboxVertex), 0, Vertexs);

	//DXGI_FORMAT format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	mCubeSRV = mRenderSys->LoadTexture(imgName, format, true, true);
#if 0
	auto pCam1 = mContext->GetSceneMng()->GetDefCamera();
	XMFLOAT3 pos0 = pCam->CalNDC(XMFLOAT3(fLowW, fLowH, 1.0f));
	XMFLOAT3 pos1 = pCam->CalNDC(XMFLOAT3(fLowW, fHighH, 1.0f));
	XMFLOAT3 pos2 = pCam->CalNDC(XMFLOAT3(fHighW, fLowH, 1.0f));
	XMFLOAT3 pos3 = pCam->CalNDC(XMFLOAT3(fHighW, fHighH, 1.0f));
	pCam1 = pCam1;
#endif
}

SkyBox::~SkyBox()
{
}

void SkyBox::SetRefCamera(TCameraPtr pCam)
{
	mRefCam = pCam;
}

int SkyBox::GenRenderOperation(RenderOperationQueue& opList)
{
	RenderOperation op = {};
	op.mMaterial = mMaterial;
	op.mIndexBuffer = mIndexBuffer;
	op.mVertexBuffer = mVertexBuffer;
	op.mTextures.push_back(mCubeSRV);
	op.mWorldTransform = XMMatrixIdentity();// mRefCam->mWorld;
	opList.AddOP(op);
	return 1;
}

void SkyBox::Draw()
{
	mRenderSys->Draw(this);
}

}