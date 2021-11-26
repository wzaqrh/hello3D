#pragma once
#include <windows.h>
#include <d3d11.h>
#include "core/rendersys/texture.h"

namespace mir {

class SamplerState11 : public ImplementResource<ISamplerState> {
public:
	void Init(ID3D11SamplerState* sampler) { mSampler = sampler; }
	ID3D11SamplerState*& GetSampler11() { return mSampler; }
public:
	ID3D11SamplerState* mSampler = nullptr;
};

class Texture11 : public ImplementResource<ITexture>
{
public:
	void Init(ResourceFormat format, HWMemoryUsage usage, int width, int height, int faceCount, int mipmap);

	bool HasSRV() const override { return mTexture != nullptr; }
	ResourceFormat GetFormat() const override { return mFormat; }
	HWMemoryUsage GetUsage() const override { return mUsage; }
	int GetWidth() const override { return mWidth; }
	int GetHeight() const override { return mHeight; }
	int GetMipmapCount() const override { return mMipCount; }
	int GetFaceCount() const override { return mFaceCount; }
	bool IsAutoGenMipmap() const override { return mAutoGenMipmap; }

	void SetSRV11(ID3D11ShaderResourceView* texture);
	ID3D11ShaderResourceView*& GetSRV11() { return mTexture; }
private:
	D3D11_TEXTURE2D_DESC GetDesc();
private:
	bool mAutoGenMipmap;
	int mWidth, mHeight, mFaceCount, mMipCount;
	ResourceFormat mFormat;
	HWMemoryUsage mUsage;
	ID3D11ShaderResourceView* mTexture;
};

}