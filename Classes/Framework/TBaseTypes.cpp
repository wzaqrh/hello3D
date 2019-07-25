#include "TBaseTypes.h"


XMFLOAT3 TCameraBase::CalNDC(XMFLOAT3 pos)
{
	XMFLOAT3 ret = XMFLOAT3(0,0,0);
	XMMATRIX vp = mView * mProjection;
	XMVECTOR vec = XMVector3Transform(XMVectorSet(pos.x, pos.y, pos.z, 1), vp);
	auto w = XMVectorGetW(vec);
	if (w != 0) {
		ret = XMFLOAT3(XMVectorGetX(vec) / w, XMVectorGetY(vec) / w, XMVectorGetZ(vec) / w);
	}
	return ret;
}

/********** TCamera **********/
TCamera::TCamera(const TCamera& other)
{
	mEye = other.mEye;
	mAt = other.mAt;
	mUp = other.mUp;
	mWidth = other.mWidth;
	mHeight = other.mHeight;
	mFOV = other.mFOV;
	mFar = other.mFar;
	mView = other.mView;
	mProjection = other.mProjection;
}

TCamera::TCamera(int width, int height, double fov, int eyeDistance, double far1)
{
	mEyeDistance = eyeDistance;
	SetLookAt(XMFLOAT3(0.0f, 0.0f, -mEyeDistance), XMFLOAT3(0,0,0));
	SetProjection(width, height, fov, far1);
}

void TCamera::SetLookAt(XMFLOAT3 eye, XMFLOAT3 at)
{
	mEye = eye;
	mAt = at;
	XMVECTOR Eye = XMVectorSet(mEye.x, mEye.y, mEye.z, 0.0f);
	XMVECTOR At = XMVectorSet(mAt.x, mAt.y, mAt.z, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	mView = XMMatrixLookAtLH(Eye, At, Up);
}

void TCamera::SetProjection(int width, int height, double fov, double far1)
{
	mWidth = width;
	mHeight = height;
	mFOV = fov / 180.0 * XM_PI;
	mFar = far1;
	mProjection = XMMatrixPerspectiveFovLH(mFOV, mWidth / mHeight, 0.01f, mFar);
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

TCameraBase TDirectLight::GetLightCamera(TCamera& otherCam)
{
	TCamera ret(otherCam);
	ret.SetLookAt(XMFLOAT3(ret.mAt.x-mPosition.x, ret.mAt.y-mPosition.y, ret.mAt.z-mPosition.z), ret.mAt);
	return ret;
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

TCameraBase TPointLight::GetLightCamera(TCamera& otherCam)
{
	TCamera ret(otherCam);
	ret.SetLookAt(XMFLOAT3(mPosition.x, mPosition.y, mPosition.z), ret.mAt);
	return ret;
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
TContantBufferPtr TMaterial::AddConstBuffer(TContantBufferPtr buffer)
{
	mConstantBuffers.push_back(buffer);
	mConstBuffers.push_back(buffer->buffer);
	return buffer;
}

/********** TRenderTexture **********/
TRenderTexture::TRenderTexture(ID3D11Device* pDevice, int width, int height, DXGI_FORMAT format)
{
	mFormat = format;
	InitRenderTexture(pDevice, width, height);
	InitRenderTargetView(pDevice);
	InitRenderTextureView(pDevice);

	InitDepthStencilTexture(pDevice, width, height);
	InitDepthStencilView(pDevice);
}

//const DXGI_FORMAT CTargetFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
bool TRenderTexture::InitRenderTexture(ID3D11Device* pDevice, int width, int height)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = mFormat;
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
	renderTargetViewDesc.Format = mFormat;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	// Create the render target view.
	HRESULT result = pDevice->CreateRenderTargetView(mRenderTargetTexture, &renderTargetViewDesc, &mRenderTargetView);
	if (FAILED(result)) {
		return false;
	}
	return true;
}

bool TRenderTexture::InitRenderTextureView(ID3D11Device* pDevice)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.Format = mFormat;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	HRESULT result = pDevice->CreateShaderResourceView(mRenderTargetTexture, &shaderResourceViewDesc, &mRenderTargetSRV);
	if (FAILED(result)) {
		return false;
	}
	return true;
}

bool TRenderTexture::InitDepthStencilTexture(ID3D11Device* pDevice, int width, int height)
{
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = width;
	depthBufferDesc.Height = height;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	HRESULT result = pDevice->CreateTexture2D(&depthBufferDesc, NULL, &mDepthStencilTexture);
	if (FAILED(result)) {
		return false;
	}
	return true;
}

bool TRenderTexture::InitDepthStencilView(ID3D11Device* pDevice)
{
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	HRESULT result = pDevice->CreateDepthStencilView(mDepthStencilTexture, &depthStencilViewDesc, &mDepthStencilView);
	if (FAILED(result)) {
		return false;
	}
	return true;
}

/********** TTexture **********/
TTexture::TTexture(std::string __path, ID3D11ShaderResourceView* __texture)
{
	path = __path;
	texture = __texture;
}

TTexture::TTexture()
	:texture(nullptr)
{

}

/********** TTextureBySlot **********/
void TTextureBySlot::clear()
{
	textures.clear();
}

void TTextureBySlot::push_back(const TTexture& texture)
{
	textures.push_back(texture);
}

bool TTextureBySlot::empty() const
{
	return textures.empty();
}

size_t TTextureBySlot::size() const
{
	return textures.size();
}

void TTextureBySlot::swap(TTextureBySlot& other)
{
	textures.swap(other.textures);
}

void TTextureBySlot::resize(size_t size)
{
	textures.resize(size);
}

const TTexture& TTextureBySlot::At(size_t pos)  const {
	return textures[pos];
}
TTexture& TTextureBySlot::At(size_t pos) {
	if (pos >= textures.size()) textures.resize(pos + 1);
	return textures[pos];
}
const TTexture& TTextureBySlot::operator[](size_t pos)  const {
	return At(pos);
}
TTexture& TTextureBySlot::operator[](size_t pos) {
	return At(pos);
}

std::vector<ID3D11ShaderResourceView*> TTextureBySlot::GetTextureViews() const
{
	std::vector<ID3D11ShaderResourceView*> views(textures.size());
	for (int i = 0; i < views.size(); ++i)
		views[i] = textures[i].texture;
	return views;
}

/********** TIndexBuffer **********/
int TIndexBuffer::GetWidth()
{
	int width = 4;
	switch (format)
	{
	case DXGI_FORMAT_R32_UINT:
		width = 4;
		break;
	case DXGI_FORMAT_R32_SINT:
		width = 4;
		break;
	case DXGI_FORMAT_R16_UINT:
		width = 2;
		break;
	case DXGI_FORMAT_R16_SINT:
		width = 2;
		break;
	case DXGI_FORMAT_R8_UINT:
		width = 1;
		break;
	case DXGI_FORMAT_R8_SINT:
		width = 1;
		break;
	default:
		break;
	}
	return width;
}
