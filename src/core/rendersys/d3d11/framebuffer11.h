#pragma once
#include <windows.h>
#include <d3d11.h>
#include "core/base/math.h"
#include "core/rendersys/base_type.h"
#include "core/rendersys/framebuffer.h"
#include "core/rendersys/d3d11/predeclare.h"
#include "core/rendersys/d3d11/texture11.h"

namespace mir {

class FrameBufferAttachColor11 : public IFrameBufferAttachment
{
public:
	ITexturePtr AsTexture() const override { return mTexture; }
	ID3D11RenderTargetView* AsRTV() { return mTexture->AsRTV(); }
public:
	Texture11Ptr mTexture;
};
typedef std::shared_ptr<FrameBufferAttachColor11> FrameBufferAttachColor11Ptr;

class FrameBufferAttachZStencil11 : public IFrameBufferAttachment
{
public:
	ITexturePtr AsTexture() const override { return mTexture; }
	ID3D11DepthStencilView* AsDSV() { return mTexture->AsDSV(); }
public:
	Texture11Ptr mTexture;
};
typedef std::shared_ptr<FrameBufferAttachZStencil11> FrameBufferAttachZStencil11Ptr;

class FrameBuffer11 : public ImplementResource<IFrameBuffer>
{
public:
	void SetAttachColor(size_t slot, FrameBufferAttachColor11Ptr attach) {
		if (mAttachColors.size() < slot + 1)
			mAttachColors.resize(slot + 1);
		mAttachColors[slot] = attach;
	}
	void SetAttachZStencil(FrameBufferAttachZStencil11Ptr attach) {
		mAttachZStencil = attach;
	}

	size_t GetAttachColorCount() const override { return mAttachColors.size(); }
	IFrameBufferAttachmentPtr GetAttachColor(size_t index) const { return mAttachColors[index]; }
	IFrameBufferAttachmentPtr GetAttachZStencil() const { return mAttachZStencil; }
private:
	std::vector<FrameBufferAttachColor11Ptr> mAttachColors;
	FrameBufferAttachZStencil11Ptr mAttachZStencil;
};

}