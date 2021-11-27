#include "core/rendersys/d3d9/framebuffer9.h"
#include "core/base/debug.h"

namespace mir {

/********** FrameBufferAttachColor9 **********/
FrameBufferAttachColor9::FrameBufferAttachColor9(Texture9Ptr colorTexture)
{
	mColorTexture = colorTexture;
	AsRes(mColorTexture)->SetLoaded();
	mColorBuffer = nullptr;
}

IDirect3DSurface9*& FrameBufferAttachColor9::GetColorBuffer9()
{
	if (CheckHR(mColorTexture->GetSRV9()->GetSurfaceLevel(0, &mColorBuffer))) {
		mColorBuffer = nullptr;
		return mColorBuffer;
	}
	return mColorBuffer;
}

/********** FrameBufferAttachZStencil9 **********/
FrameBufferAttachZStencil9::FrameBufferAttachZStencil9(IDirect3DSurface9* depthStencilBuffer)
{
	mDepthStencilBuffer = depthStencilBuffer;
}

/********** FrameBuffer9 **********/
FrameBuffer9::FrameBuffer9()
{

}
FrameBuffer9::FrameBuffer9(Texture9Ptr colorTexture, IDirect3DSurface9* depthStencilBuffer)
{
	mAttachColors.push_back(std::make_shared<FrameBufferAttachColor9>(colorTexture));
	mAttachZStencil = std::make_shared<FrameBufferAttachZStencil9>(depthStencilBuffer);
}

}