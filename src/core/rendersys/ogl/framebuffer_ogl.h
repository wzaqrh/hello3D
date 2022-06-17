#pragma once
#include <windows.h>
#include <glad/glad.h>
#include "core/base/math.h"
//#include "core/base/base_type.h"
#include "core/rendersys/framebuffer.h"
#include "core/rendersys/ogl/predeclare.h"
#include "core/rendersys/ogl/texture_ogl.h"

namespace mir {

class FrameBufferAttachOGL : public IFrameBufferAttachment
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	FrameBufferAttachOGL(TextureOGLPtr texture) :mTexture(texture) {}
	ITexturePtr AsTexture() const override { return mTexture; }
private:
	TextureOGLPtr mTexture;
};
typedef std::shared_ptr<FrameBufferAttachOGL> FrameBufferAttachOGLPtr;

class FrameBufferOGL : public ImplementResource<IFrameBuffer>
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	void Init(GLuint id, Eigen::Vector2i size) { 
		mId = id; 
		mSize = size;
	}
	void SetAttachColor(size_t slot, FrameBufferAttachOGLPtr attach);
	void SetAttachZStencil(FrameBufferAttachOGLPtr attach) {
		mAttachZStencil = attach;
	}
public:
	GLuint GetId() const { return mId; }
	Eigen::Vector2i GetSize() const override { return mSize; }
	size_t GetAttachColorCount() const override { return mAttachColors.size(); }
	IFrameBufferAttachmentPtr GetAttachColor(size_t index) const override { 
		return !mAttachColors.empty() ? mAttachColors[index] : nullptr; 
	}
	IFrameBufferAttachmentPtr GetAttachZStencil() const override { return mAttachZStencil; }
private:
	GLuint mId = 0;
	std::vector<FrameBufferAttachOGLPtr> mAttachColors;
	FrameBufferAttachOGLPtr mAttachZStencil;
	Eigen::Vector2i mSize;
};

}