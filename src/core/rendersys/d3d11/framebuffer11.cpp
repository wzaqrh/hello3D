#include "core/rendersys/d3d11/framebuffer11.h"
#include "core/base/debug.h"

namespace mir {

#define MakePtr std::make_shared
#define PtrRaw(T) T.get()

FrameBuffer11::FrameBuffer11()
{
	mRenderTargetTexture = nullptr;
	mRenderTargetSRV = nullptr;
	mRenderTargetView = nullptr;

	mDepthStencilTexture = nullptr;
	mDepthStencilView = nullptr;
}

void FrameBuffer11::Init(ID3D11Device* pDevice, const Eigen::Vector2i& size, ResourceFormat format)
{
	mFormat = format;
	mSize = size;

	InitRenderTexture(pDevice);
	InitRenderTargetView(pDevice);
	InitRenderTextureView(pDevice);

	InitDepthStencilTexture(pDevice);
	InitDepthStencilView(pDevice);
}

bool FrameBuffer11::InitRenderTexture(ID3D11Device* pDevice)
{
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = mSize.x();
	textureDesc.Height = mSize.y();
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = static_cast<DXGI_FORMAT>(mFormat);
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	if (FAILED(pDevice->CreateTexture2D(&textureDesc, NULL, &mRenderTargetTexture)))
		return false;
	return true;
}

bool FrameBuffer11::InitRenderTargetView(ID3D11Device* pDevice)
{
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
	renderTargetViewDesc.Format = static_cast<DXGI_FORMAT>(mFormat);
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	if (CheckHR(pDevice->CreateRenderTargetView(mRenderTargetTexture, &renderTargetViewDesc, &mRenderTargetView)))
		return false;
	return true;
}

bool FrameBuffer11::InitRenderTextureView(ID3D11Device* pDevice)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
	shaderResourceViewDesc.Format = static_cast<DXGI_FORMAT>(mFormat);
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	if (CheckHR(pDevice->CreateShaderResourceView(mRenderTargetTexture, &shaderResourceViewDesc, &mRenderTargetSRV)))
		return false;

	mRenderTargetPtr = MakePtr<Texture11>();
	mRenderTargetPtr->Init(mFormat, kHWUsageDefault, mSize.x(), mSize.y(), 1, 1);
	mRenderTargetPtr->SetSRV11(mRenderTargetSRV);
	AsRes(mRenderTargetPtr)->SetLoaded();
	return true;
}

bool FrameBuffer11::InitDepthStencilTexture(ID3D11Device* pDevice)
{
	D3D11_TEXTURE2D_DESC depthBufferDesc = {};
	depthBufferDesc.Width = mSize.x();
	depthBufferDesc.Height = mSize.y();
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	if (CheckHR(pDevice->CreateTexture2D(&depthBufferDesc, NULL, &mDepthStencilTexture)))
		return false;
	return true;
}

bool FrameBuffer11::InitDepthStencilView(ID3D11Device* pDevice)
{
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	if (CheckHR(pDevice->CreateDepthStencilView(mDepthStencilTexture, &depthStencilViewDesc, &mDepthStencilView)))
		return false;
	return true;
}

}