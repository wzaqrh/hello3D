#pragma once
#include <windows.h>
#include <glad/glad.h>
#include <GL/GL.h>
#include "core/mir_config.h"
#include "core/base/data.h"
#include "core/rendersys/sampler.h"
#include "core/rendersys/texture.h"

namespace mir {

class SamplerStateOGL : public ImplementResource<ISamplerState>
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	void Init(GLuint id) { mId = id; }
	GLuint GetId() const { return mId; }
public:
	GLuint mId = 0;
};

class TextureOGL : public ImplementResource<ITexture>
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	TextureOGL();
	~TextureOGL();
	void Dispose();
	void Init(ResourceFormat format, HWMemoryUsage usage, int width, int height, int faceCount, int mipmap);
	void InitTex(const Data datas[]);
	void AutoGenMipmap();
	void OnLoaded() override;
public:
	GLuint GetId() const { return mId; }
	ResourceFormat GetFormat() const override { return mFormat; }
	HWMemoryUsage GetUsage() const override { return mUsage; }
	Eigen::Vector2i GetSize() const override { return mSize; }
	Eigen::Vector2i GetRealSize() const override { return mRealSize; }
	int GetMipmapCount() const override { return mMipCount; }
	int GetFaceCount() const override { return mFaceCount; }
	bool IsAutoGenMipmap() const override { return mAutoGenMipmap; }
private:
	GLuint mId, mTarget;
	bool mAutoGenMipmap;
	int mFaceCount, mMipCount;
	Eigen::Vector2i mSize, mRealSize;
	ResourceFormat mFormat;
	HWMemoryUsage mUsage;
};

}