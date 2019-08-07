#include "TInterfaceType.h"
#include "IRenderSystem.h"
#include "TInterfaceType11.h"

template<class T>
static IUnknown*& MakeDeviceObjectRef(T*& ref) {
	IUnknown** ppDeviceObj = (IUnknown**)&ref;
	return *ppDeviceObj;
}
/********** TBlobDataD3d **********/
TBlobDataD3d::TBlobDataD3d(ID3DBlob* pBlob)
	:mBlob(pBlob)
{
}

void* TBlobDataD3d::GetBufferPointer()
{
	return mBlob->GetBufferPointer();
}

size_t TBlobDataD3d::GetBufferSize()
{
	return mBlob->GetBufferSize();
}
/********** TBlobDataStd **********/
TBlobDataStd::TBlobDataStd(const std::vector<char>& buffer)
	:mBuffer(buffer)
{
}

void* TBlobDataStd::GetBufferPointer()
{
	return mBuffer.empty() ? nullptr : &mBuffer[0];
}

size_t TBlobDataStd::GetBufferSize()
{
	return mBuffer.size();
}
/********** TVertexShader **********/
TVertexShader::TVertexShader(IBlobDataPtr pBlob)
	:mBlob(pBlob)
{
}
IUnknown*& TVertexShader::GetDeviceObject()
{
	return MakeDeviceObjectRef(mErrBlob);
}
/********** TPixelShader **********/
TPixelShader::TPixelShader(IBlobDataPtr pBlob)
	: mBlob(pBlob)
{
}
IUnknown*& TPixelShader::GetDeviceObject()
{
	return MakeDeviceObjectRef(mErrBlob);
}
/********** TProgram **********/
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
	mRenderTargetPtr = std::make_shared<TTexture11>(mRenderTargetSRV, "RenderTexture");
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

ITexturePtr TRenderTexture::GetRenderTargetSRV()
{
	return mRenderTargetPtr;
}

/********** ITexture **********/
ID3D11ShaderResourceView*& ITexture::GetSRV11()
{
	static ID3D11ShaderResourceView* srv;
	return srv;
}

IDirect3DTexture9*& ITexture::GetSRV9()
{
	static IDirect3DTexture9* srv;
	return srv;
}

/********** IHardwareBuffer **********/
ID3D11Buffer*& IHardwareBuffer::GetBuffer11()
{
	static ID3D11Buffer* buffer;
	return buffer;
}

unsigned int IHardwareBuffer::GetBufferSize()
{
	return 0;
}

/********** IVertexBuffer **********/
IDirect3DVertexBuffer9*& IVertexBuffer::GetBuffer9()
{
	static IDirect3DVertexBuffer9* buffer;
	return buffer;
}

unsigned int IVertexBuffer::GetStride()
{
	return 0;
}

unsigned int IVertexBuffer::GetOffset()
{
	return 0;
}

/********** IIndexBuffer **********/
IDirect3DIndexBuffer9*& IIndexBuffer::GetBuffer9()
{
	static IDirect3DIndexBuffer9* buffer;
	return buffer;
}