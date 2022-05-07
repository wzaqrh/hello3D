#include "core/rendersys/d3d11/texture11.h"

namespace mir {

/********** Texture11 **********/
Texture11::Texture11()
{
	mAutoGenMipmap = false;
	mFaceCount = mMipCount = 0;
	mSize = mRealSize = Eigen::Vector2i(0, 0);
	mFormat = kFormatUnknown;
	mUsage = kHWUsageDefault;

	mTex2D = nullptr;
	mSRV = nullptr;
	mRTV = nullptr;
	mDSV = nullptr;
}

void Texture11::Init(ResourceFormat format, HWMemoryUsage usage, int width, int height, int faceCount, int mipmap)
{
	mSize = Eigen::Vector2i(width, height);
	mRealSize = Eigen::Vector2i(0, 0);
	mFaceCount = std::max<int>(faceCount, 1);
	mMipCount = std::max<int>(mipmap, 1);
	mAutoGenMipmap = mipmap < 0;
	mFormat = format;
	mUsage = usage;

	mSRV = nullptr;
	mRTV = nullptr;
	mDSV = nullptr;
}

void Texture11::OnLoaded()
{
	if (mSRV != nullptr) 
	{
		ID3D11Texture2D* pTexture;
		mSRV->GetResource((ID3D11Resource**)&pTexture);

		D3D11_TEXTURE2D_DESC desc;
		pTexture->GetDesc(&desc);
		mRealSize = Eigen::Vector2i(desc.Width, desc.Height);
	}
}

}