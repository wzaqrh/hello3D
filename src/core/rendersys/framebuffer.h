#pragma once
#include "core/rendersys/predeclare.h"
#include "core/resource/resource.h"

namespace mir {

interface IFrameBufferAttachment : boost::noncopyable
{
	virtual ITexturePtr AsTexture() const = 0;
};

interface IFrameBuffer : public IResource
{
	virtual size_t GetAttachColorCount() const = 0;
	virtual IFrameBufferAttachmentPtr GetAttachColor(size_t index) const = 0;
	virtual IFrameBufferAttachmentPtr GetAttachZStencil() const = 0;

	ITexturePtr GetAttachColorTexture(size_t index) const { return GetAttachColor(index)->AsTexture(); }
	ITexturePtr GetAttachZStencilTexture() const { return GetAttachZStencil()->AsTexture(); }
};

}