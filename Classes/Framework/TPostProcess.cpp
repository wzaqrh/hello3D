#include "TPostProcess.h"

/********** POSTPROCESS_VERTEX_QUAD **********/
POSTPROCESS_VERTEX_QUAD::POSTPROCESS_VERTEX_QUAD(float x, float y, float w, float h)
{
	SetRect(x, y, w, h);
	SetFlipY(true);
	SetZ(1);
}

void POSTPROCESS_VERTEX_QUAD::SetRect(float x, float y, float w, float h)
{
	lb.Pos = XMFLOAT4(x, y, 0, 1);
	lt.Pos = XMFLOAT4(x, y + h, 0, 1);
	rt.Pos = XMFLOAT4(x + w, y + h, 0, 1);
	rb.Pos = XMFLOAT4(x + w, y, 0, 1);
}

void POSTPROCESS_VERTEX_QUAD::SetFlipY(bool flipY)
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

void POSTPROCESS_VERTEX_QUAD::SetZ(float z)
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
TPostProcess::TPostProcess(TRenderSystem* RenderSys, TRenderTexturePtr mainTex)
{
	mRenderSys = RenderSys;
	mMainTex = mainTex;

	mIndexBuffer = mRenderSys->CreateIndexBuffer(sizeof(indices), DXGI_FORMAT_R32_UINT, (void*)&indices[0]);
	POSTPROCESS_VERTEX_QUAD quad(-1, -1, 2, 2);
	mVertexBuffer = mRenderSys->CreateVertexBuffer(sizeof(POSTPROCESS_VERTEX_QUAD), sizeof(POSTPROCESS_VERTEX), 0, &quad);
}

TPostProcess::~TPostProcess()
{
}

void TPostProcess::Draw()
{
	mRenderSys->Draw(this);
}

int TPostProcess::GenRenderOperation(TRenderOperationQueue& opList)
{
	TRenderOperation op = {};
	op.mMaterial = mMaterial;
	op.mIndexBuffer = mIndexBuffer;
	op.mVertexBuffer = mVertexBuffer;
	op.mTextures.push_back(mMainTex->GetRenderTargetSRV());
	op.mWorldTransform = XMMatrixIdentity();
	op.mVertBufferByPass = mVertBufferByPass;
	opList.push_back(op);
	return 1;
}

/********** TBloom **********/
TVertexBufferPtr GetVertBufByRT(TRenderSystem* RenderSys, TRenderTexturePtr target) {
	auto srv = target->GetRenderTargetSRV();
	float sx = srv->GetWidth() * 1.0 / RenderSys->mScreenWidth;
	float sy = srv->GetHeight() * 1.0 / RenderSys->mScreenHeight;
	assert(sx <= 1 && sy <= 1);
	POSTPROCESS_VERTEX_QUAD quad(-1, 1.0 - 2 * sy, 2 * sx, 2 * sy);
	TVertexBufferPtr vertBuf = RenderSys->CreateVertexBuffer(sizeof(POSTPROCESS_VERTEX_QUAD), sizeof(POSTPROCESS_VERTEX), 0, &quad);
	return vertBuf;
}

TBloom::TBloom(TRenderSystem* RenderSys, TRenderTexturePtr mainTex)
	:TPostProcess(RenderSys, mainTex)
{
	mMaterial = mRenderSys->CreateMaterial(E_MAT_POSTPROC_BLOOM, nullptr);

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

cbBloom cbBloom::CreateDownScale2x2Offsets(int dwWidth, int dwHeight)
{
	cbBloom bloom = {};
	float tU = 1.0f / dwWidth;// / 2.0f;
	float tV = 1.0f / dwHeight;// / 2.0f;
	// Sample from 4 surrounding points. 
	int index = 0;
	for (int y = 0; y < 2; y++)
	{
		for (int x = 0; x < 2; x++)
		{
			bloom.SampleOffsets[index].x = (x - 0.5f) * tU;
			bloom.SampleOffsets[index].y = (y - 0.5f) * tV;
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
	for (int y = -1; y <= 1; y++)
	{
		for (int x = -1; x <= 1; x++)
		{
			bloom.SampleOffsets[index].x = x * tU;
			bloom.SampleOffsets[index].y = y * tV;
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
	bloom.SampleOffsets[0] = XMFLOAT2(0.0f, 0);
	bloom.SampleWeights[0] = XMFLOAT4(weight, weight, weight, 1.0f);

	// Fill the right side
	for (i = 1; i < 8; i++)
	{
		weight = fMultiplier * GaussianDistribution((float)i, 0, fDeviation);
		bloom.SampleOffsets[i] = XMFLOAT2(i * tu,0);
		bloom.SampleWeights[i] = XMFLOAT4(weight, weight, weight, 1.0f);
	}

	// Copy to the left side
	for (i = 8; i < 15; i++)
	{
		bloom.SampleOffsets[i] = XMFLOAT2(-bloom.SampleOffsets[i - 7].x, 0);
		bloom.SampleWeights[i] = bloom.SampleWeights[i - 7];
	}
	return bloom;
}
