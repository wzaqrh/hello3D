#include "interface_type11.h"
#include "core/rendersys/render_system.h"
#include "core/rendersys/resource.h"
#include "core/base/utility.h"

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

char* TBlobDataD3d11::GetBufferPointer()
{
	return (char*)mBlob->GetBufferPointer();
}

size_t TBlobDataD3d11::GetBufferSize()
{
	return mBlob->GetBufferSize();
}

/********** TInputLayout11 **********/
TInputLayout11::TInputLayout11()
{
	mRes = MakePtr<TResource>((IUnknown**)&mLayout);
}
IResourcePtr TInputLayout11::AsRes() {
	return mRes;
}

ID3D11InputLayout*& TInputLayout11::GetLayout11()
{
	return mLayout;
}


/********** TVertexShader11 **********/
TVertexShader11::TVertexShader11(IBlobDataPtr pBlob)
	:mBlob(pBlob)
{
	mRes = MakePtr<TResource>((IUnknown**)&mErrBlob);
}
IResourcePtr TVertexShader11::AsRes() {
	return mRes;
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
	mRes = MakePtr<TResource>((IUnknown**)&mErrBlob);
}
IResourcePtr TPixelShader11::AsRes() {
	return mRes;
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
TTexture11::TTexture11(ID3D11ShaderResourceView* __texture, const std::string& __path)
{
	mPath = __path;
	mTexture = __texture;

	mRes = MakePtr<TResource>((IUnknown**)&mTexture);
	mRes->AddOnLoadedListener([this](IResource* pRes) {
		D3D11_TEXTURE2D_DESC desc = GetDesc();
		mWidth = desc.Width;
		mHeight = desc.Height;
		mFormat = desc.Format;
		mMipCount = desc.MipLevels;
	});
}

TTexture11::TTexture11(int width, int height, DXGI_FORMAT format, int mipmap)
{
	mWidth = width;
	mHeight = height;
	mFormat = format;
	mMipCount = mipmap;
	mRes = MakePtr<TResource>((IUnknown**)&mTexture);
}

void TTexture11::SetSRV11(ID3D11ShaderResourceView* __texture) {
	mTexture = __texture;
}
ID3D11ShaderResourceView*& TTexture11::GetSRV11() {
	return mTexture;
}

const char* TTexture11::GetPath() { 
	return mPath.c_str(); 
}
int TTexture11::GetWidth() {
	return mWidth;
}
int TTexture11::GetHeight() {
	return mHeight;
}
DXGI_FORMAT TTexture11::GetFormat() {
	return mFormat;
}
int TTexture11::GetMipmapCount() {
	return mMipCount;
}
D3D11_TEXTURE2D_DESC TTexture11::GetDesc()
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
int TVertexBuffer11::GetCount()
{
	return hd.bufferSize / stride;
}

ID3D11Buffer*& TVertexBuffer11::GetBuffer11()
{
	return hd.buffer;
}

unsigned int TVertexBuffer11::GetBufferSize()
{
	return hd.bufferSize;
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
	return hd.buffer;
}

unsigned int TIndexBuffer11::GetBufferSize()
{
	return hd.bufferSize;
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
TContantBuffer11::TContantBuffer11(ID3D11Buffer* __buffer, TConstBufferDeclPtr decl)
	: hd(__buffer, decl->bufferSize)
	, mDecl(decl)
{
}

TConstBufferDeclPtr TContantBuffer11::GetDecl()
{
	return mDecl;
}

ID3D11Buffer*& TContantBuffer11::GetBuffer11()
{
	return hd.buffer;
}

unsigned int TContantBuffer11::GetBufferSize()
{
	return hd.bufferSize;
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
	mRenderTargetPtr = MakePtr<TTexture11>(mRenderTargetSRV, "RenderTexture");
	mRenderTargetPtr->AsRes()->SetLoaded();
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

/********** TProgram11 **********/
TProgram11::TProgram11()
{
	mRes = MakePtr<TResource>((IUnknown**)0);
}
IResourcePtr TProgram11::AsRes() {
	return mRes;
}

IVertexShaderPtr TProgram11::GetVertex() {
	return mVertex;
}
IPixelShaderPtr TProgram11::GetPixel() {
	return mPixel;
}

void TProgram11::SetVertex(TVertexShader11Ptr pVertex)
{
	mVertex = pVertex;
	mRes->AddDependency(pVertex->AsRes());
}

void TProgram11::SetPixel(TPixelShader11Ptr pPixel)
{
	mPixel = pPixel;
	mRes->AddDependency(pPixel->AsRes());
}
