#include "TInterfaceType.h"
#include "TRenderSystem.h"

template<class T>
IUnknown*& MakeDeviceObjectRef(T*& ref) {
	IUnknown** ppDeviceObj = (IUnknown**)&ref;
	return *ppDeviceObj;
}

/********** TProgram **********/
IUnknown*& TVertexShader::GetDeviceObject()
{
	return MakeDeviceObjectRef(mBlob);
}

IUnknown*& TPixelShader::GetDeviceObject()
{
	return MakeDeviceObjectRef(mBlob);
}

void TProgram::SetVertex(TVertexShaderPtr pVertex)
{
	mVertex = pVertex;
	AddDependency(pVertex);
}

void TProgram::SetPixel(TPixelShaderPtr pPixel)
{
	mPixel = pPixel;
	AddDependency(pPixel);
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
	mRenderTargetPtr = std::make_shared<TTexture>(mRenderTargetSRV, "RenderTexture");
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

TTexturePtr TRenderTexture::GetRenderTargetSRV()
{
	return mRenderTargetPtr;
}

/********** TTexture **********/
#if 0
TTexture::TTexture()
	:texture(nullptr)
{
}
#endif

TTexture::TTexture(ID3D11ShaderResourceView* __texture, std::string __path)
{
	path = __path;
	texture = __texture;
}

void TTexture::SetSRV(ID3D11ShaderResourceView* __texture)
{
	texture = __texture;
}

IUnknown*& TTexture::GetDeviceObject()
{
	return MakeDeviceObjectRef(texture);
}

const std::string& TTexture::GetPath() const
{
	return path;
}

ID3D11ShaderResourceView*& TTexture::GetSRV()
{
	return texture;
}

D3D11_TEXTURE2D_DESC TTexture::GetDesc()
{
	ID3D11Texture2D* pTexture;
	texture->GetResource((ID3D11Resource **)&pTexture);
	
	D3D11_TEXTURE2D_DESC desc;
	pTexture->GetDesc(&desc);

	return desc;
}

int TTexture::GetWidth()
{
	return GetDesc().Width;
}

int TTexture::GetHeight()
{
	return GetDesc().Height;
}

DXGI_FORMAT TTexture::GetFormat()
{
	return GetDesc().Format;
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

/********** TVertexBuffer **********/
int TVertexBuffer::GetCount()
{
	return bufferSize / stride;
}