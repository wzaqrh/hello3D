#include "TBaseTypes.h"

/********** TCamera **********/
TCamera::TCamera(int width, int height)
{
	// Initialize the view matrix
	XMVECTOR Eye = XMVectorSet(0.0f, 0.0f, -10.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	mView = XMMatrixLookAtLH(Eye, At, Up);

	/*XMVECTOR res = XMVector4Transform(Eye, mView);
	float x = XMVectorGetX(res);
	float y = XMVectorGetY(res);
	float z = XMVectorGetZ(res);*/

	// Initialize the projection matrix
	mProjection = XMMatrixPerspectiveFovLH(XM_PIDIV4, width / (FLOAT)height, 0.01f, 100.0f);
}

/********** TDirectLight **********/
TDirectLight::TDirectLight()
{
	SetDirection(0, 0, 1);
	SetDiffuseColor(1, 1, 1, 1);
	SetSpecularColor(1, 1, 1, 1);
	SetSpecularPower(32);
}

void TDirectLight::SetDirection(float x, float y, float z)
{
	mPosition = XMFLOAT4(x, y, z, 0);
}

void TDirectLight::SetDiffuseColor(float r, float g, float b, float a)
{
	mDiffuseColor = XMFLOAT4(r, g, b, a);
}

void TDirectLight::SetSpecularColor(float r, float g, float b, float a)
{
	mSpecularColorPower = XMFLOAT4(r, g, b, mSpecularColorPower.w);
}

void TDirectLight::SetSpecularPower(float power)
{
	mSpecularColorPower.w = power;
}

/********** TLight **********/
TPointLight::TPointLight()
{
	SetPosition(0, 0, 0);
	SetAttenuation(1.0, 0.01, 0.0);
}

void TPointLight::SetPosition(float x, float y, float z)
{
	mPosition = XMFLOAT4(x, y, z, 1);
}

void TPointLight::SetAttenuation(float a, float b, float c)
{
	mAttenuation = XMFLOAT4(a, b, c, 0);
}

/********** TSpotLight **********/
TSpotLight::TSpotLight()
{
	SetDirection(0, 0, 1);
	SetAngle(3.14 * 30 / 180);
}

void TSpotLight::SetDirection(float x, float y, float z)
{
	mDirCutOff = XMFLOAT4(x, y, z, mDirCutOff.w);
}

void TSpotLight::SetCutOff(float cutoff)
{
	mDirCutOff.w = cutoff;
}

void TSpotLight::SetAngle(float radian)
{
	SetCutOff(cos(radian));
}

/********** cbGlobalParam **********/
cbGlobalParam::cbGlobalParam()
{
	auto Ident = XMMatrixIdentity();
	mWorld = Ident;
	mView = Ident;
	mProjection = Ident;
}

/********** TMaterial **********/
ID3D11Buffer* TMaterial::AddConstBuffer(ID3D11Buffer* buffer)
{
	mConstantBuffers.push_back(buffer);
	return buffer;
}

/********** TRenderTexture **********/
TRenderTexture::TRenderTexture(ID3D11Device* pDevice, int width, int height)
{
	InitTexture(pDevice, width, height);
	InitRenderTargetView(pDevice);
}

const DXGI_FORMAT CTargetFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
bool TRenderTexture::InitTexture(ID3D11Device* pDevice, int width, int height)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = CTargetFormat;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	HRESULT result = pDevice->CreateTexture2D(&textureDesc, NULL, &mRenderTargetTexture);
	if (FAILED(result)) {
		return false;
	}
	return true;
}

bool TRenderTexture::InitRenderTargetView(ID3D11Device* pDevice)
{
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	ZeroMemory(&renderTargetViewDesc, sizeof(renderTargetViewDesc));
	renderTargetViewDesc.Format = CTargetFormat;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	// Create the render target view.
	HRESULT result = pDevice->CreateRenderTargetView(mRenderTargetTexture, &renderTargetViewDesc, &mRenderTargetView);
	if (FAILED(result)) {
		return false;
	}
	return true;
}

bool TRenderTexture::InitResourceView(ID3D11Device* pDevice)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.Format = CTargetFormat;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	HRESULT result = pDevice->CreateShaderResourceView(mRenderTargetTexture, &shaderResourceViewDesc, &mShaderResourceView);
	if (FAILED(result)) {
		return false;
	}
	return true;
}
