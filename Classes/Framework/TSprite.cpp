#include "TSprite.h"

/********** Quad **********/
Quad::Quad(float x, float y, float w, float h)
{
	lb.Tex = XMFLOAT2(0, 0);
	lt.Tex = XMFLOAT2(0, 1);
	rt.Tex = XMFLOAT2(1, 1);
	rb.Tex = XMFLOAT2(1, 0);

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
	mMaterial->mConstantBuffers.push_back(mRenderSys->CreateConstBuffer(sizeof(Pos3Color3Tex2)));

	mIndexBuffer = mRenderSys->CreateIndexBuffer(sizeof(indices), (void*)&indices[0]);
	mVertexBuffer = mRenderSys->CreateVertexBuffer(sizeof(Quad));
}

TSprite::~TSprite()
{
}

void TSprite::SetPosition(float x, float y)
{
	mPosition = XMFLOAT2(x, y);
	mQuad.SetRect(mPosition.x, mPosition.y, mSize.x, mSize.y);

	mRenderSys->UpdateBuffer(mVertexBuffer, &mQuad, sizeof(mQuad));
}

void TSprite::SetSize(float w, float h)
{
	mSize = XMFLOAT2(w,h);
	mQuad.SetRect(mPosition.x, mPosition.y, mSize.x, mSize.y);

	mRenderSys->UpdateBuffer(mVertexBuffer, &mQuad, sizeof(mQuad));
}

void TSprite::SetTexture(ID3D11ShaderResourceView* Texture)
{
	mTexture = Texture;
}

void TSprite::Draw()
{
	mRenderSys->ApplyMaterial(mMaterial, XMMatrixIdentity());

	UINT stride = sizeof(Pos3Color3Tex2);
	UINT offset = 0;
	mRenderSys->mDeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
	mRenderSys->mDeviceContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	if (mTexture != nullptr) {
		mRenderSys->mDeviceContext->PSSetShaderResources(0, 1, &mTexture);
	}
	mRenderSys->mDeviceContext->DrawIndexed(ARRAYSIZE(indices), 0, 0);
}

