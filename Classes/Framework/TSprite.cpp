#include "TSprite.h"

/********** Quad **********/
Quad::Quad(float x, float y, float w, float h)
{
	SetFlipY(false);
	SetRect(x, y, w, h);
	SetColor(1, 1, 1, 1);
}

void Quad::SetRect(float x, float y, float w, float h)
{
	lb.Pos = XMFLOAT3(x, y, 0);
	lt.Pos = XMFLOAT3(x, y + h, 0);
	rt.Pos = XMFLOAT3(x + w, y + h, 0);
	rb.Pos = XMFLOAT3(x + w, y, 0);
}

void Quad::SetColor(float r, float g, float b, float a)
{
	lb.Color = XMFLOAT4(r, g, b, a);
	lt.Color = XMFLOAT4(r, g, b, a);
	rt.Color = XMFLOAT4(r, g, b, a);
	rb.Color = XMFLOAT4(r, g, b, a);
}

void Quad::SetZ(float z)
{
	lb.Pos.z = z;
	lt.Pos.z = z;
	rt.Pos.z = z;
	rb.Pos.z = z;
}

void Quad::SetFlipY(bool flipY)
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

/********** TSprite **********/
const unsigned int indices[] = {
	0, 1, 2, 0, 2, 3 
};
TSprite::TSprite(TRenderSystem* RenderSys, const char* vsName, const char* psName)
:mQuad(0,0,0,0)
{
	mRenderSys = RenderSys;

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 3 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 7 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	mMaterial = mRenderSys->CreateMaterial(vsName, psName, layout, ARRAYSIZE(layout));
	mMaterial->AddConstBuffer(mRenderSys->CreateConstBuffer(sizeof(Pos3Color3Tex2)));

	mIndexBuffer = mRenderSys->CreateIndexBuffer(sizeof(indices), DXGI_FORMAT_R32_UINT, (void*)&indices[0]);
	mVertexBuffer = mRenderSys->CreateVertexBuffer(sizeof(Quad), sizeof(Pos3Color3Tex2), 0);
}

TSprite::~TSprite()
{
}

void TSprite::SetPosition(float x, float y, float z)
{
	mPosition = XMFLOAT2(x, y);
	mQuad.SetRect(mPosition.x, mPosition.y, mSize.x, mSize.y);
	mQuad.SetZ(z);

	mRenderSys->UpdateBuffer(mVertexBuffer.get(), &mQuad, sizeof(mQuad));
}

void TSprite::SetSize(float w, float h)
{
	mSize = XMFLOAT2(w,h);
	mQuad.SetRect(mPosition.x, mPosition.y, mSize.x, mSize.y);

	mRenderSys->UpdateBuffer(mVertexBuffer.get(), &mQuad, sizeof(mQuad));
}

void TSprite::SetTexture(ID3D11ShaderResourceView* Texture)
{
	mTexture = Texture;
}

void TSprite::SetFlipY(bool flipY)
{
	mQuad.SetFlipY(flipY);
}

void TSprite::Draw()
{
#ifdef USE_RENDER_OP
	mRenderSys->Draw(this);
#else
	mRenderSys->ApplyMaterial(mMaterial, XMMatrixIdentity());
	mRenderSys->SetVertexBuffer(mVertexBuffer);
	mRenderSys->SetIndexBuffer(mIndexBuffer);
	if (mTexture != nullptr) {
		mRenderSys->mDeviceContext->PSSetShaderResources(0, 1, &mTexture);
	}
	mRenderSys->DrawIndexed(mIndexBuffer);
#endif
}

int TSprite::GenRenderOperation(TRenderOperationList& opList)
{
	TRenderOperation op = {};
	op.mMaterial = mMaterial;
	op.mIndexBuffer = mIndexBuffer;
	op.mVertexBuffer = mVertexBuffer;
	op.mTextures.push_back(TTexture("", mTexture));
	opList.push_back(op);
	return 1;
}