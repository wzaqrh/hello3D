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
	virtual Eigen::Vector2i GetSize() const = 0;
	virtual size_t GetAttachColorCount() const = 0;
	virtual IFrameBufferAttachmentPtr GetAttachColor(size_t index) const = 0;
	virtual IFrameBufferAttachmentPtr GetAttachZStencil() const = 0;
public:
	ITexturePtr GetAttachColorTexture(size_t index) const { 
		auto texture = GetAttachColor(index);
		return texture ? texture->AsTexture() : nullptr; 
	}
	ITexturePtr GetAttachZStencilTexture() const { 
		auto texture = GetAttachZStencil();
		return texture ? texture->AsTexture() : nullptr;
	}
};

}