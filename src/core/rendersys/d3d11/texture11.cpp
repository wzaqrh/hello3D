#include "core/rendersys/d3d11/texture11.h"

namespace mir {

/********** Texture11 **********/
Texture11::Texture11()
{
	mAutoGenMipmap = false;
	mWidth = mHeight = mFaceCount = mMipCount = 0;
	mFormat = kFormatUnknown;
	mUsage = kHWUsageDefault;
	mSRV = nullptr;
	mRTV = nullptr;
	mDSV = nullptr;
}

void Texture11::Init(ResourceFormat format, HWMemoryUsage usage, int width, int height, int faceCount, int mipmap)
{
	mWidth = width;
	mHeight = height;
	mFaceCount = std::max<int>(faceCount, 1);
	mMipCount = std::max<int>(mipmap, 1);
	mAutoGenMipmap = mipmap < 0;
	mFormat = format;
	mUsage = usage;

	mSRV = nullptr;
}

D3D11_TEXTURE2D_DESC Texture11::GetDesc()
{
	if (mSRV != nullptr) {
		ID3D11Texture2D* pTexture;
		mSRV->GetResource((ID3D11Resource **)&pTexture);

		D3D11_TEXTURE2D_DESC desc;
		pTexture->GetDesc(&desc);
		return desc;
	}
	else {
		D3D11_TEXTURE2D_DESC desc = { 0 };
		return desc;
	}
}

}