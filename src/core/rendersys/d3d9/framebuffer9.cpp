#include "core/rendersys/d3d9/framebuffer9.h"
#include "core/base/debug.h"

namespace mir {

FrameBuffer9::FrameBuffer9()
{
	mColorTexture = nullptr;
	mColorBuffer = nullptr;
	mDepthStencilBuffer = nullptr;
}
FrameBuffer9::FrameBuffer9(Texture9Ptr colorTexture, IDirect3DSurface9* depthStencilBuffer)
{
	mColorTexture = colorTexture;
	AsRes(mColorTexture)->SetLoaded();
	mColorBuffer = nullptr;
	mDepthStencilBuffer = depthStencilBuffer;
}

IDirect3DSurface9*& FrameBuffer9::GetColorBuffer9()
{
	if (CheckHR(mColorTexture->GetSRV9()->GetSurfaceLevel(0, &mColorBuffer))) {
		mColorBuffer = nullptr;
		return mColorBuffer;
	}
	return mColorBuffer;
}

}