#include "core/rendersys/ogl/framebuffer_ogl.h"
#include "core/rendersys/ogl/ogl_bind.h"
#include "core/rendersys/ogl/ogl_utils.h"
#include "core/base/debug.h"

namespace mir {

FrameBufferOGL::~FrameBufferOGL()
{
	Dispose();
}
void FrameBufferOGL::Dispose()
{
	if (mId) {
		glDeleteFramebuffers(1, &mId);
		mId = 0;
	}
}

void FrameBufferOGL::Init(Eigen::Vector2i size)
{
	CheckHR(glGenFramebuffers(1, &mId));
	mSize = size;
}

void FrameBufferOGL::SetAttachColor(size_t slot, IFrameBufferAttachmentPtr attach) {
	if (attach) {
		if (mAttachColors.size() < slot + 1)
			mAttachColors.resize(slot + 1);
		mAttachColors[slot] = attach;
		CheckHR(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,  std::static_pointer_cast<TextureOGL>(attach->AsTexture())->GetId(), 0));
	}
	else {
		if (slot == mAttachColors.size() - 1) {
			while (!mAttachColors.empty() && mAttachColors.back() == nullptr)
				mAttachColors.pop_back();
		}
	}
}

void FrameBufferOGL::SetAttachZStencil(IFrameBufferAttachmentPtr attach) 
{
	mAttachZStencil = attach;
	CheckHR(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, std::static_pointer_cast<TextureOGL>(attach->AsTexture())->GetId(), 0));
}

/********** FrameBufferAttachOGLFactory **********/
static TextureOGLPtr _CreateColorAttachTexture(const Eigen::Vector3i& size, ResourceFormat format)
{
	constexpr bool autoGen = false;
	constexpr size_t mipCount = 1, faceCount = 1;

	TextureOGLPtr texture = CreateInstance<TextureOGL>();
	texture->Init(format, kHWUsageDefault, size.x(), size.y(), faceCount, mipCount);
	texture->InitTex(nullptr);
	texture->SetLoaded();
	return texture;
}
FrameBufferOGLAttachPtr FrameBufferAttachOGLFactory::CreateColorAttachment(const Eigen::Vector3i& size, ResourceFormat format)
{
	auto texture = _CreateColorAttachTexture(size, format);
	return format == kFormatUnknown ? nullptr : CreateInstance<FrameBufferOGLAttach>(texture);
}

static TextureOGLPtr _CreateZStencilAttachTexture(const Eigen::Vector2i& size, ResourceFormat format)
{
	BOOST_ASSERT(IsDepthStencil(format));
	BOOST_ASSERT(format == kFormatD24UNormS8UInt
		|| format == kFormatD32Float
		|| format == kFormatD16UNorm);

	constexpr bool autoGen = false;
	constexpr size_t mipCount = 1;

	TextureOGLPtr texture = CreateInstance<TextureOGL>();
	texture->Init(format, kHWUsageDefault, size.x(), size.y(), 1, mipCount);
	texture->InitTex(nullptr);
	texture->SetLoaded();
	return texture;
}
FrameBufferOGLAttachPtr FrameBufferAttachOGLFactory::CreateZStencilAttachment(const Eigen::Vector2i& size, ResourceFormat format)
{
	auto texture = _CreateZStencilAttachTexture(size, format);
	return format == kFormatUnknown ? nullptr : CreateInstance<FrameBufferOGLAttach>(texture);
}

}