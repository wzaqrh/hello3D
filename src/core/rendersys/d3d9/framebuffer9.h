#pragma once
#include <windows.h>
#include <d3d9.h>
#include "core/base/math.h"
#include "core/rendersys/base_type.h"
#include "core/rendersys/framebuffer.h"
#include "core/rendersys/d3d9/predeclare.h"
#include "core/rendersys/d3d9/texture9.h"

namespace mir {

class FrameBuffer9 : public ImplementResource<IFrameBuffer>
{
public:
	FrameBuffer9();
	FrameBuffer9(Texture9Ptr colorTexture, IDirect3DSurface9* depthStencilBuffer);
	ITexturePtr GetColorTexture() const override { return mColorTexture; }
	IDirect3DSurface9*& GetColorBuffer9();
	IDirect3DSurface9*& GetDepthStencilBuffer9() { return mDepthStencilBuffer; }
public:
	IDirect3DSurface9* mDepthStencilBuffer;
	IDirect3DSurface9* mColorBuffer;
	Texture9Ptr mColorTexture;
};

}