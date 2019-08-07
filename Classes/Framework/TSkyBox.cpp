#include "TSkyBox.h"
#include "IRenderSystem.h"
#include "TMaterial.h"

TSkyBox::TSkyBox(IRenderSystem* pRenderSys, TCameraPtr pCam, const std::string& imgName)
{
	mRenderSys = pRenderSys;
	mRefCam = pCam;

	mMaterial = mRenderSys->CreateMaterial(E_MAT_SKYBOX, nullptr);
	mIndexBuffer = nullptr;

	SKYBOX_VERTEX Vertexs[4];
	float fHighW = -1.0f - (1.0f / (float)pRenderSys->mScreenWidth);
	float fHighH = -1.0f - (1.0f / (float)pRenderSys->mScreenHeight);
	float fLowW = 1.0f + (1.0f / (float)pRenderSys->mScreenWidth);
	float fLowH = 1.0f + (1.0f / (float)pRenderSys->mScreenHeight);
	Vertexs[0].pos = XMFLOAT4(fLowW, fLowH, 1.0f, 1.0f);
	Vertexs[1].pos = XMFLOAT4(fLowW, fHighH, 1.0f, 1.0f);
	Vertexs[2].pos = XMFLOAT4(fHighW, fLowH, 1.0f, 1.0f);
	Vertexs[3].pos = XMFLOAT4(fHighW, fHighH, 1.0f, 1.0f);
	mVertexBuffer = mRenderSys->CreateVertexBuffer(sizeof(SKYBOX_VERTEX) * 4, sizeof(SKYBOX_VERTEX), 0, Vertexs);

	mCubeSRV = mRenderSys->GetTexByPath(imgName, DXGI_FORMAT_R32G32B32A32_FLOAT);
#if 0
	auto pCam1 = mRenderSys->mDefCamera;
	XMFLOAT3 pos0 = pCam->CalNDC(XMFLOAT3(fLowW, fLowH, 1.0f));
	XMFLOAT3 pos1 = pCam->CalNDC(XMFLOAT3(fLowW, fHighH, 1.0f));
	XMFLOAT3 pos2 = pCam->CalNDC(XMFLOAT3(fHighW, fLowH, 1.0f));
	XMFLOAT3 pos3 = pCam->CalNDC(XMFLOAT3(fHighW, fHighH, 1.0f));
	pCam1 = pCam1;
#endif
}

TSkyBox::~TSkyBox()
{
}

void TSkyBox::SetRefCamera(TCameraPtr pCam)
{
	mRefCam = pCam;
}

int TSkyBox::GenRenderOperation(TRenderOperationQueue& opList)
{
	TRenderOperation op = {};
	op.mMaterial = mMaterial;
	op.mIndexBuffer = mIndexBuffer;
	op.mVertexBuffer = mVertexBuffer;
	op.mTextures.push_back(mCubeSRV);
	op.mWorldTransform = XMMatrixIdentity();// mRefCam->mWorld;
	opList.push_back(op);
	return 1;
}

void TSkyBox::Draw()
{
	mRenderSys->Draw(this);
}
