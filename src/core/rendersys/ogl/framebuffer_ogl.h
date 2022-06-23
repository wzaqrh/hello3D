#pragma once
#include <windows.h>
#include <glad/glad.h>
#include "core/base/math.h"
#include "core/rendersys/framebuffer.h"
#include "core/rendersys/ogl/predeclare.h"
#include "core/rendersys/ogl/texture_ogl.h"

namespace mir {

class FrameBufferOGLAttach : public IFrameBufferAttachment
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	FrameBufferOGLAttach(TextureOGLPtr texture) :mTexture(texture) {}
	ITexturePtr AsTexture() const override { return mTexture; }
private:
	TextureOGLPtr mTexture;
};
typedef std::shared_ptr<FrameBufferOGLAttach> FrameBufferOGLAttachPtr;

class FrameBufferAttachOGLFactory
{
public:
	static FrameBufferOGLAttachPtr CreateColorAttachment(const Eigen::Vector3i& size, ResourceFormat format);
	static FrameBufferOGLAttachPtr CreateZStencilAttachment(const Eigen::Vector2i& size, ResourceFormat format);
};

class FrameBufferOGL : public ImplementResource<IFrameBuffer>
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	~FrameBufferOGL();
	void Dispose();
	void Init(Eigen::Vector2i size);
	void SetAttachColor(size_t slot, IFrameBufferAttachmentPtr attach) override;
	void SetAttachZStencil(IFrameBufferAttachmentPtr attach) override;
public:
	GLuint GetId() const { return mId; }
	Eigen::Vector2i GetSize() const override { return mSize; }
	size_t GetAttachColorCount() const override { return mAttachColors.size(); }
	IFrameBufferAttachmentPtr GetAttachColor(size_t index) const override { return !mAttachColors.empty() ? mAttachColors[index] : nullptr; }
	IFrameBufferAttachmentPtr GetAttachZStencil() const override { return mAttachZStencil; }
private:
	GLuint mId = 0;
	Eigen::Vector2i mSize;
	std::vector<IFrameBufferAttachmentPtr> mAttachColors;
	IFrameBufferAttachmentPtr mAttachZStencil;
};

}