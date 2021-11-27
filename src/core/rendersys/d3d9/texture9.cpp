#include "core/rendersys/d3d9/texture9.h"

namespace mir {

/********** Texture9 **********/
Texture9::Texture9(IDirect3DTexture9 *texture)
	: mTexture(texture)
	, mTextureCube(nullptr)
{
	mWidth = 0, mHeight = 0, mMipCount = 0;
	mFormat = kFormatUnknown;

	/*SetDeviceObject((IUnknown**)&mTexture);
	AsRes(this)->AddOnLoadedListener([this](IResource*) {
		D3DSURFACE_DESC desc = GetDesc();
		mWidth = desc.Width;
		mHeight = desc.Height;
		mFormat = static_cast<ResourceFormat>(d3d::convert9To11(desc.Format));
		mMipCount = mTexture->GetLevelCount();
	});*/
}

Texture9::Texture9(int width, int height, ResourceFormat format, int mipmap)
	: mTexture(nullptr)
	, mTextureCube(nullptr)
{
	mWidth = width;
	mHeight = height;
	mMipCount = mipmap;
	mFormat = format;

	//SetDeviceObject((IUnknown**)&mTexture);
}

void Texture9::SetSRV9(IDirect3DTexture9* texture)
{
	mTexture = texture;
}

D3DSURFACE_DESC Texture9::GetDesc()
{
	D3DSURFACE_DESC desc;
	mTexture->GetLevelDesc(0, &desc);
	return desc;
}

}