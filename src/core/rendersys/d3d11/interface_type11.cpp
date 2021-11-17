#include "interface_type11.h"
#include "core/rendersys/render_system.h"
#include "core/rendersys/resource.h"
#include "core/base/d3d.h"

namespace mir {

#define MakePtr std::make_shared
#define PtrRaw(T) T.get()
template<class T> static IUnknown*& MakeDeviceObjectRef(T*& ref) {
	IUnknown** ppDeviceObj = (IUnknown**)&ref;
	return *ppDeviceObj;
}

/********** BlobData11 **********/
BlobData11::BlobData11(ID3DBlob* pBlob)
	:mBlob(pBlob)
{
}

char* BlobData11::GetBufferPointer()
{
	return (char*)mBlob->GetBufferPointer();
}

size_t BlobData11::GetBufferSize()
{
	return mBlob->GetBufferSize();
}

/********** TInputLayout11 **********/
InputLayout11::InputLayout11()
{
	SetDeviceObject((IUnknown**)&mLayout);
}

/********** VertexShader11 **********/
VertexShader11::VertexShader11(IBlobDataPtr pBlob)
	:mBlob(pBlob)
{
	SetDeviceObject((IUnknown**)&mErrBlob);
}

/********** PixelShader11 **********/
PixelShader11::PixelShader11(IBlobDataPtr pBlob)
	: mBlob(pBlob)
{
	SetDeviceObject((IUnknown**)&mErrBlob);
}

/********** Texture11 **********/
Texture11::Texture11(ID3D11ShaderResourceView* texture, const std::string& path)
{
	mWidth = 0, mHeight = 0, mMipCount = 0;
	mFormat = kFormatUnknown;

	mPath = path;
	mTexture = texture;

	SetDeviceObject((IUnknown**)&mTexture);
	AsRes(this)->AddOnLoadedListener([this](IResource* pRes) {
		D3D11_TEXTURE2D_DESC desc = GetDesc();
		mWidth = desc.Width;
		mHeight = desc.Height;
		mFormat = static_cast<ResourceFormat>(desc.Format);
		mMipCount = desc.MipLevels;
	});
}

Texture11::Texture11(int width, int height, ResourceFormat format, int mipmap)
{
	mTexture = nullptr;

	mWidth = width;
	mHeight = height;
	mMipCount = mipmap;
	mFormat = format;
	SetDeviceObject((IUnknown**)&mTexture);
}

void Texture11::SetSRV11(ID3D11ShaderResourceView* texture) 
{
	mTexture = texture;
}

D3D11_TEXTURE2D_DESC Texture11::GetDesc()
{
	if (mTexture != nullptr) {
		ID3D11Texture2D* pTexture;
		mTexture->GetResource((ID3D11Resource **)&pTexture);

		D3D11_TEXTURE2D_DESC desc;
		pTexture->GetDesc(&desc);
		return desc;
	}
	else {
		D3D11_TEXTURE2D_DESC desc = { 0 };
		return desc;
	}
}

/********** TVertex11Buffer **********/
int VertexBuffer11::GetCount()
{
	return hd.bufferSize / Stride;
}

/********** IndexBuffer11 **********/
int IndexBuffer11::GetWidth()
{
	return d3d::GetFormatWidthByte(static_cast<DXGI_FORMAT>(Format));
}

/********** ContantBuffer11 **********/
ContantBuffer11::ContantBuffer11(ID3D11Buffer* buffer, ConstBufferDeclPtr decl)
	: hd(buffer, decl->BufferSize)
	, mDecl(decl)
{
}

/********** RenderTexture11 **********/
RenderTexture11::RenderTexture11()
{
	mRenderTargetTexture = nullptr;
	mRenderTargetSRV = nullptr;
	mRenderTargetView = nullptr;

	mDepthStencilTexture = nullptr;
	mDepthStencilView = nullptr;
}

RenderTexture11::RenderTexture11(ID3D11Device* pDevice, int width, int height, ResourceFormat format)
	:RenderTexture11()
{
	mFormat = format;
	InitRenderTexture(pDevice, width, height);
	InitRenderTargetView(pDevice);
	InitRenderTextureView(pDevice);

	InitDepthStencilTexture(pDevice, width, height);
	InitDepthStencilView(pDevice);
}

//const DXGI_FORMAT CTargetFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
bool RenderTexture11::InitRenderTexture(ID3D11Device* pDevice, int width, int height)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = static_cast<DXGI_FORMAT>(mFormat);
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

bool RenderTexture11::InitRenderTargetView(ID3D11Device* pDevice)
{
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	ZeroMemory(&renderTargetViewDesc, sizeof(renderTargetViewDesc));
	renderTargetViewDesc.Format = static_cast<DXGI_FORMAT>(mFormat);
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	// Create the render target view.
	HRESULT result = pDevice->CreateRenderTargetView(mRenderTargetTexture, &renderTargetViewDesc, &mRenderTargetView);
	if (FAILED(result)) {
		return false;
	}
	return true;
}

bool RenderTexture11::InitRenderTextureView(ID3D11Device* pDevice)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.Format = static_cast<DXGI_FORMAT>(mFormat);
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	HRESULT result = pDevice->CreateShaderResourceView(mRenderTargetTexture, &shaderResourceViewDesc, &mRenderTargetSRV);
	if (FAILED(result)) {
		return false;
	}
	mRenderTargetPtr = MakePtr<Texture11>(mRenderTargetSRV, "RenderTexture");
	AsRes(mRenderTargetPtr)->SetLoaded();
	return true;
}

bool RenderTexture11::InitDepthStencilTexture(ID3D11Device* pDevice, int width, int height)
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

bool RenderTexture11::InitDepthStencilView(ID3D11Device* pDevice)
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

/********** TProgram11 **********/
Program11::Program11()
{
	//SetDeviceObject((IUnknown**)0);
}

void Program11::SetVertex(VertexShader11Ptr pVertex)
{
	mVertex = pVertex;
	AsRes(this)->AddDependency(AsRes(pVertex));
}

void Program11::SetPixel(PixelShader11Ptr pPixel)
{
	mPixel = pPixel;
	AsRes(this)->AddDependency(AsRes(pPixel));
}

}