#include "core/rendersys/d3d11/texture11.h"
#include "core/base/debug.h"
#include "core/base/d3d.h"

namespace mir {

Texture11::Texture11()
{
	mAutoGenMipmap = false;
	mBindRTOrDS = false;
	mFaceCount = mMipCount = 0;
	mSize = mRealSize = Eigen::Vector2i(0, 0);
	mFormat = kFormatUnknown;
	mUsage = kHWUsageDefault;

	mTex2D = nullptr;
	mSRV = nullptr;
	mRTV = nullptr;
	mDSV = nullptr;
}
Texture11::~Texture11()
{
}

void Texture11::Init(ResourceFormat format, HWMemoryUsage usage, int width, int height, int faceCount, int mipmap, bool bindRTorDS)
{
	mSize = Eigen::Vector2i(width, height);
	mRealSize = Eigen::Vector2i(0, 0);
	mFaceCount = std::max<int>(faceCount, 1); BOOST_ASSERT(mFaceCount == 1 || mFaceCount == 6);
	mMipCount = std::max<int>(mipmap, 1);
	mAutoGenMipmap = mipmap < 0;
	mBindRTOrDS = bindRTorDS;
	mFormat = format;
	mUsage = usage;

	mTex2D = nullptr;
	mSRV = nullptr;
	mRTV = nullptr;
	mDSV = nullptr;
}

void Texture11::Init(ComPtr<ID3D11Texture2D>&& tex2d)
{
	D3D11_TEXTURE2D_DESC desc;
	tex2d->GetDesc(&desc);

	this->Init(static_cast<ResourceFormat>(desc.Format), static_cast<HWMemoryUsage>(desc.Usage), desc.Width, desc.Height, 1, desc.MipLevels, desc.BindFlags & D3D11_BIND_RENDER_TARGET);

	mTex2D = std::move(tex2d);
}

const ComPtr<ID3D11Texture2D>& Texture11::InitTex(const ComPtr<ID3D11Device>& device, const D3D11_SUBRESOURCE_DATA datas[])
{
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = mSize.x();
	desc.Height = mSize.y();
	desc.MipLevels = mAutoGenMipmap ? 0 : mMipCount;
	desc.ArraySize = mFaceCount;
	desc.Format = static_cast<DXGI_FORMAT>(mFormat);
	desc.Format = d3d::IsDepthStencil(desc.Format) ? d3d::MakeTypeless(desc.Format) : desc.Format;
	desc.SampleDesc.Count = 1;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	if (mUsage == kHWUsageDefault) {
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;
		if (mAutoGenMipmap) {
			desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
			desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}
		else if (mFaceCount > 1) {
			desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
		}
		else {
			desc.MiscFlags = 0;
		}
	}
	else {
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
	}
	if (mBindRTOrDS) {
		desc.BindFlags |= d3d::IsDepthStencil(desc.Format) ? D3D11_BIND_DEPTH_STENCIL : D3D11_BIND_RENDER_TARGET;
	}

	if (CheckHR(device->CreateTexture2D(&desc, datas, &mTex2D)))
		return nullptr;

	mTex2D->GetDesc(&desc);
	mMipCount = desc.MipLevels;

	return mTex2D;
}

const ComPtr<ID3D11ShaderResourceView>& Texture11::InitSRV(const ComPtr<ID3D11Device>& device)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = static_cast<DXGI_FORMAT>(mFormat);
	srvDesc.Format = d3d::IsDepthStencil(srvDesc.Format) ? d3d::MakeTypeless1(srvDesc.Format) : srvDesc.Format;
	srvDesc.ViewDimension = (mFaceCount > 1) ? D3D11_SRV_DIMENSION_TEXTURECUBE : D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = mAutoGenMipmap ? -1 : mMipCount;
	mSRV = nullptr;
	if (CheckHR(device->CreateShaderResourceView(mTex2D.Get(), &srvDesc, &mSRV))) 
		return nullptr;
	return mSRV;
}

const ComPtr<ID3D11RenderTargetView>& Texture11::InitRTV(const ComPtr<ID3D11Device>& device)
{
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = static_cast<DXGI_FORMAT>(mFormat);
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	
	if (CheckHR(device->CreateRenderTargetView(mTex2D.Get(), &rtvDesc, &mRTV))) 
		return nullptr;
	return mRTV;
}

const ComPtr<ID3D11DepthStencilView>& Texture11::InitDSV(const ComPtr<ID3D11Device>& device)
{
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = static_cast<DXGI_FORMAT>(mFormat);
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	
	if (CheckHR(device->CreateDepthStencilView(mTex2D.Get(), &dsvDesc, &mDSV))) 
		return nullptr;
	return mDSV;
}

void Texture11::OnLoaded()
{
	if (mSRV != nullptr) {
		D3D11_TEXTURE2D_DESC desc;
		mTex2D->GetDesc(&desc);
		mRealSize = Eigen::Vector2i(desc.Width, desc.Height);
	}
}

}