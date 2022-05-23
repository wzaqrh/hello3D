#pragma once
#include <windows.h>
#include <d3d9.h>
#include "core/base/math.h"
#include "core/rendersys/framebuffer.h"
#include "core/rendersys/d3d9/predeclare.h"
#include "core/rendersys/d3d9/texture9.h"

namespace mir {

class FrameBufferAttachColor9 : public IFrameBufferAttachment
{
public:
	FrameBufferAttachColor9(Texture9Ptr colorTexture);
	ITexturePtr AsTexture() const override { return mColorTexture; }

	IDirect3DSurface9*& GetColorBuffer9();
private:
	IDirect3DSurface9* mColorBuffer = nullptr;
	Texture9Ptr mColorTexture;
};
typedef std::shared_ptr<FrameBufferAttachColor9> FrameBufferAttachColor9Ptr;

class FrameBufferAttachZStencil9 : public IFrameBufferAttachment
{
public:
	FrameBufferAttachZStencil9(IDirect3DSurface9* depthStencilBuffer);
	ITexturePtr AsTexture() const override { return mTexture; }

	IDirect3DSurface9*& GetDepthStencilBuffer9() { return mDepthStencilBuffer; }
private:
	IDirect3DSurface9* mDepthStencilBuffer = nullptr;
	Texture9Ptr mTexture;
};
typedef std::shared_ptr<FrameBufferAttachZStencil9> FrameBufferAttachZStencil9Ptr;

class FrameBuffer9 : public ImplementResource<IFrameBuffer>
{
public:
	FrameBuffer9();
	FrameBuffer9(Texture9Ptr colorTexture, IDirect3DSurface9* depthStencilBuffer);
	
	Eigen::Vector2i GetSize() const override { return mSize; }
	size_t GetAttachColorCount() const override { return mAttachColors.size(); }
	IFrameBufferAttachmentPtr GetAttachColor(size_t index) const { return mAttachColors[index]; }
	IFrameBufferAttachmentPtr GetAttachZStencil() const { return mAttachZStencil; }
private:
	std::vector<FrameBufferAttachColor9Ptr> mAttachColors;
	FrameBufferAttachZStencil9Ptr mAttachZStencil;
	Eigen::Vector2i mSize;
};

}