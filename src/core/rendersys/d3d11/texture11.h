#pragma once
#include <windows.h>
#include <d3d11.h>
#include "core/mir_config.h"
#include "core/rendersys/sampler.h"
#include "core/rendersys/texture.h"

namespace mir {

class SamplerState11 : public ImplementResource<ISamplerState> 
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	void Init(ID3D11SamplerState* sampler) { mSampler = sampler; }
	ID3D11SamplerState*& GetSampler11() { return mSampler; }
public:
	ID3D11SamplerState* mSampler = nullptr;
#if defined MIR_RESOURCE_DEBUG
	SamplerDesc mDesc;
#endif
};

class Texture11 : public ImplementResource<ITexture>
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	Texture11();
	void Init(ResourceFormat format, HWMemoryUsage usage, int width, int height, int faceCount, int mipmap);
	void SetTex2D(ID3D11Texture2D* tex2d) { mTex2D = tex2d; }
	void SetSRV(ID3D11ShaderResourceView* srv) { mSRV = srv; }
	void SetRTV(ID3D11RenderTargetView* rtv) { mRTV = rtv; }
	void SetDSV(ID3D11DepthStencilView* dsv) { mDSV = dsv; }
public:
	ID3D11Texture2D* AsTex2D() const { return mTex2D; }
	ID3D11ShaderResourceView* AsSRV() const { return mSRV; }
	ID3D11RenderTargetView* AsRTV() const { return mRTV; }
	ID3D11DepthStencilView* AsDSV() const { return mDSV; }
public:
	ResourceFormat GetFormat() const override { return mFormat; }
	HWMemoryUsage GetUsage() const override { return mUsage; }
	Eigen::Vector2i GetSize() const override { return mSize; }
	Eigen::Vector2i GetRealSize() const override { return mRealSize; }
	int GetMipmapCount() const override { return mMipCount; }
	int GetFaceCount() const override { return mFaceCount; }
	bool IsAutoGenMipmap() const override { return mAutoGenMipmap; }

	void OnLoaded() override;
private:
	bool mAutoGenMipmap;
	int mFaceCount, mMipCount;
	Eigen::Vector2i mSize, mRealSize;
	ResourceFormat mFormat;
	HWMemoryUsage mUsage;
	ID3D11Texture2D* mTex2D;
	ID3D11ShaderResourceView* mSRV;
	ID3D11RenderTargetView* mRTV;
	ID3D11DepthStencilView* mDSV;
};

}