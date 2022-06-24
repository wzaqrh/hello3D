#include "core/rendersys/ogl/texture_ogl.h"
#include "core/rendersys/ogl/ogl_utils.h"
#include "core/base/debug.h"
#include "core/base/macros.h"

namespace mir {

TextureOGL::~TextureOGL()
{
	Dispose();
}
void TextureOGL::Dispose()
{
	if (mId) {
		glDeleteTextures(1, &mId);
		mId = 0;
	}
}

TextureOGL::TextureOGL()
{
	mId = mTarget = 0;
	mAutoGenMipmap = false;
	mFaceCount = mMipCount = 0;
	mSize = mRealSize = Eigen::Vector2i(0, 0);
	mFormat = kFormatUnknown;
	mUsage = kHWUsageDefault;
}
void TextureOGL::Init(ResourceFormat format, HWMemoryUsage usage, int width, int height, int faceCount, int mipmap)
{
	mSize = Eigen::Vector2i(width, height);
	mRealSize = Eigen::Vector2i(0, 0);
	mFaceCount = std::max<int>(faceCount, 1); BOOST_ASSERT(mFaceCount == 1 || mFaceCount == 6);
	mMipCount = std::max<int>(mipmap, 1);
	mAutoGenMipmap = mipmap < 0;
	mFormat = format;
	mUsage = usage;
}
void TextureOGL::InitTex(const Data2 datas[])
{
	CheckHR(glGenTextures(1, &mId));

	mTarget = IF_AND_OR(mFaceCount > 1, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_2D);
	CheckHR(glBindTexture(mTarget, mId));

	//internal sampler parameter
	{
		CheckHR(glTexParameteri(mTarget, GL_TEXTURE_BASE_LEVEL, 0));
		CheckHR(glTexParameteri(mTarget, GL_TEXTURE_MAX_LEVEL, mMipCount - 1));

		CheckHR(glTexParameteri(mTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		CheckHR(glTexParameteri(mTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));

		CheckHR(glTexParameteri(mTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		CheckHR(glTexParameteri(mTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		CheckHR(glTexParameteri(mTarget, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));

		//CheckHR(glPixelStorei(GL_UNPACK_ROW_LENGTH, 0));
	}

	auto glFmt = ogl::GetGlFormatInfo(mFormat);
	unsigned texWidth = mSize.x(), texHeight = mSize.y();
	if (!mAutoGenMipmap) {
		CheckHR(glTexStorage2D(mTarget, mMipCount, glFmt.InternalFormat, texWidth, texHeight));
	}

	if (datas) {
		for (size_t mip = 0; mip < mMipCount; ++mip) {
			for (size_t face = 0; face < mFaceCount; ++face) {
				size_t index = face * mMipCount + mip;
				const Data2& data = datas[index];

				GLenum target = IF_AND_OR(mFaceCount > 1, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, GL_TEXTURE_2D);
				if (!mAutoGenMipmap) {
					if (glFmt.IsCompressed) {
						CheckHR(glCompressedTexSubImage2D(target, mip, 0, 0, texWidth >> mip, texHeight >> mip, glFmt.InternalFormat, data.Size, data.Bytes));
					}
					else {
						CheckHR(glTexSubImage2D(target, mip, 0, 0, texWidth >> mip, texHeight >> mip, glFmt.ExternalFormat, glFmt.InternalType, data.Bytes));
					}
				}
				else {
					CheckHR(glTexImage2D(target, mip, glFmt.InternalFormat, texWidth >> mip, texHeight >> mip, 0, glFmt.ExternalFormat, glFmt.InternalType, data.Bytes));
				}
			}
		}
	}
	else {
		if (mAutoGenMipmap) {
			CheckHR(glTexImage2D(mTarget, 0, glFmt.InternalFormat, texWidth, texHeight, 0, glFmt.ExternalFormat, glFmt.InternalType, nullptr));
		}
	}

	if (mAutoGenMipmap) {
		mMipCount = 1 + ceil(log2(std::max(mSize.x(), mSize.y())));
		CheckHR(glTexParameteri(mTarget, GL_TEXTURE_BASE_LEVEL, 0));
		CheckHR(glTexParameteri(mTarget, GL_TEXTURE_MAX_LEVEL, mMipCount - 1));
	}

	CheckHR(glBindTexture(mTarget, 0));
}

void TextureOGL::AutoGenMipmap()
{
	if (mAutoGenMipmap) {
		CheckHR(glGenerateTextureMipmap(mId));
	}
}

void TextureOGL::OnLoaded()
{}

}