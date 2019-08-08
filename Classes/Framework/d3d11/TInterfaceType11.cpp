#include "TInterfaceType11.h"
#include "IRenderSystem.h"
#include "Utility.h"

template<class T>
static IUnknown*& MakeDeviceObjectRef(T*& ref) {
	IUnknown** ppDeviceObj = (IUnknown**)&ref;
	return *ppDeviceObj;
}

/********** TBlobDataD3d11 **********/
TBlobDataD3d11::TBlobDataD3d11(ID3DBlob* pBlob)
	:mBlob(pBlob)
{
}

void* TBlobDataD3d11::GetBufferPointer()
{
	return mBlob->GetBufferPointer();
}

size_t TBlobDataD3d11::GetBufferSize()
{
	return mBlob->GetBufferSize();
}

/********** TInputLayout11 **********/
ID3D11InputLayout*& TInputLayout11::GetLayout11()
{
	return mLayout;
}

/********** TVertexShader11 **********/
TVertexShader11::TVertexShader11(IBlobDataPtr pBlob)
	:mBlob(pBlob)
{
}
IUnknown*& TVertexShader11::GetDeviceObject()
{
	return MakeDeviceObjectRef(mErrBlob);
}

IBlobDataPtr TVertexShader11::GetBlob()
{
	return mBlob;
}

ID3D11VertexShader*& TVertexShader11::GetShader11()
{
	return mShader;
}

/********** TPixelShader11 **********/
TPixelShader11::TPixelShader11(IBlobDataPtr pBlob)
	: mBlob(pBlob)
{
}
IUnknown*& TPixelShader11::GetDeviceObject()
{
	return MakeDeviceObjectRef(mErrBlob);
}

IBlobDataPtr TPixelShader11::GetBlob()
{
	return mBlob;
}

ID3D11PixelShader*& TPixelShader11::GetShader11()
{
	return mShader;
}

/********** TTexture11 **********/
TTexture11::TTexture11(ID3D11ShaderResourceView* __texture, std::string __path)
{
	path = __path;
	texture = __texture;
}

void TTexture11::SetSRV11(ID3D11ShaderResourceView* __texture)
{
	texture = __texture;
}

IUnknown*& TTexture11::GetDeviceObject()
{
	return MakeDeviceObjectRef(texture);
}

const std::string& TTexture11::GetPath() const
{
	return path;
}

ID3D11ShaderResourceView*& TTexture11::GetSRV11()
{
	return texture;
}

D3D11_TEXTURE2D_DESC TTexture11::GetDesc()
{
	ID3D11Texture2D* pTexture;
	texture->GetResource((ID3D11Resource **)&pTexture);

	D3D11_TEXTURE2D_DESC desc;
	pTexture->GetDesc(&desc);

	return desc;
}

int TTexture11::GetWidth()
{
	return GetDesc().Width;
}

int TTexture11::GetHeight()
{
	return GetDesc().Height;
}

DXGI_FORMAT TTexture11::GetFormat()
{
	return GetDesc().Format;
}

/********** TVertex11Buffer **********/
int TVertexBuffer11::GetCount()
{
	return bufferSize / stride;
}

ID3D11Buffer*& TVertexBuffer11::GetBuffer11()
{
	return buffer;
}

unsigned int TVertexBuffer11::GetBufferSize()
{
	return bufferSize;
}

unsigned int TVertexBuffer11::GetStride()
{
	return stride;
}

unsigned int TVertexBuffer11::GetOffset()
{
	return offset;
}

/********** TIndexBuffer **********/
ID3D11Buffer*& TIndexBuffer11::GetBuffer11()
{
	return buffer;
}

unsigned int TIndexBuffer11::GetBufferSize()
{
	return bufferSize;
}

int TIndexBuffer11::GetWidth()
{
	return D3DEnumCT::GetWidth(format);
}

DXGI_FORMAT TIndexBuffer11::GetFormat()
{
	return format;
}

/********** TContantBuffer11 **********/
ID3D11Buffer*& TContantBuffer11::GetBuffer11()
{
	return buffer;
}

unsigned int TContantBuffer11::GetBufferSize()
{
	return bufferSize;
}

/********** TRenderTexture11 **********/
TRenderTexture11::TRenderTexture11(ID3D11Device* pDevice, int width, int height, DXGI_FORMAT format)
{
	mFormat = format;
	InitRenderTexture(pDevice, width, height);
	InitRenderTargetView(pDevice);
	InitRenderTextureView(pDevice);

	InitDepthStencilTexture(pDevice, width, height);
	InitDepthStencilView(pDevice);
}

//const DXGI_FORMAT CTargetFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
bool TRenderTexture11::InitRenderTexture(ID3D11Device* pDevice, int width, int height)
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

bool TRenderTexture11::InitRenderTargetView(ID3D11Device* pDevice)
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

bool TRenderTexture11::InitRenderTextureView(ID3D11Device* pDevice)
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
	mRenderTargetPtr = std::make_shared<TTexture11>(mRenderTargetSRV, "RenderTexture");
	return true;
}

bool TRenderTexture11::InitDepthStencilTexture(ID3D11Device* pDevice, int width, int height)
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

bool TRenderTexture11::InitDepthStencilView(ID3D11Device* pDevice)
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

ITexturePtr TRenderTexture11::GetColorTexture()
{
	return mRenderTargetPtr;
}

ID3D11RenderTargetView*& TRenderTexture11::GetColorBuffer11()
{
	return mRenderTargetView;
}

ID3D11DepthStencilView*& TRenderTexture11::GetDepthStencilBuffer11()
{
	return mDepthStencilView;
}

/********** TSamplerState11 **********/
ID3D11SamplerState*& TSamplerState11::GetSampler11()
{
	return mSampler;
}