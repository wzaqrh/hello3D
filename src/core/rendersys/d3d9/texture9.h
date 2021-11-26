#pragma once
#include <windows.h>
#include <d3d9.h>
#include "core/rendersys/texture.h"

namespace mir {

class SamplerState9 : public ImplementResource<ISamplerState>
{
public:
	std::map<D3DSAMPLERSTATETYPE, DWORD>& GetSampler9() { return mStates; }
public:
	std::map<D3DSAMPLERSTATETYPE, DWORD> mStates;
};

class Texture9 : public ImplementResource<ITexture>
{
public:
	Texture9(int width, int height, ResourceFormat format, int mipmap);
	Texture9(IDirect3DTexture9* texture);

	bool IsCube() const { return mTextureCube != nullptr; }
	bool HasSRV() const override { return mTexture != nullptr || mTextureCube != nullptr; }
	void SetSRV9(IDirect3DTexture9* texture);
	IDirect3DTexture9*& GetSRV9() { return mTexture; }
	IDirect3DCubeTexture9*& GetSRVCube9() { return mTextureCube; }

	int GetWidth() const override { return mWidth; }
	int GetHeight() const override { return mHeight; }
	ResourceFormat GetFormat() const override { return mFormat; }
	HWMemoryUsage GetUsage() const override { return kHWUsageDefault; }
	int GetMipmapCount() const override { return mMipCount; }
	int GetFaceCount() const override { return 1; }
	bool IsAutoGenMipmap() const override { return false; }
private:
	D3DSURFACE_DESC GetDesc();
private:
	int mWidth, mHeight, mMipCount;
	ResourceFormat mFormat;
	IDirect3DTexture9 *mTexture;
	IDirect3DCubeTexture9* mTextureCube;
};

}