#pragma once
#include <windows.h>
#include <glad/glad.h>
#include <GL/GL.h>
#include "core/rendersys/hardware_buffer.h"

namespace mir {

class VertexArrayOGL : public ImplementResource<IVertexArray> {
public:
	VertexArrayOGL(GLuint id) :mId(id) {}
	GLuint GetId() const { return mId; }
private:
	GLuint mId = 0;
};

class VertexBufferOGL : public ImplementResource<IVertexBuffer>
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	VertexBufferOGL() :mBufferSize(0), mUsage(kHWUsageDefault), mId(0), mStride(0), mOffset(0) {}
	void Init(VertexArrayOGLPtr vao, GLuint id, int bufferSize, HWMemoryUsage usage, int stride, int offset) {
		mVao = vao;
		mId = id;
		mBufferSize = bufferSize;
		mUsage = usage;
		mStride = stride;
		mOffset = offset;
	}
public:
	HWMemoryUsage GetUsage() const override { return mUsage; }
	int GetBufferSize() const override { return mBufferSize; }
	HardwareBufferType GetType() const override { return kHWBufferVertex; }

	int GetStride() const override { return mStride; }
	int GetOffset() const override { return mOffset; }

	GLuint& GetId() { return mId; }
public:
	VertexArrayOGLPtr mVao;
	GLuint mId;
	size_t mBufferSize;
	HWMemoryUsage mUsage;
	unsigned int mStride, mOffset;
};

class IndexBufferOGL : public ImplementResource<IIndexBuffer>
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	IndexBufferOGL() :mBufferSize(0), mUsage(kHWUsageDefault), mId(0), mFormat(kFormatUnknown) {}
	void Init(VertexArrayOGLPtr vao, GLuint id, int bufferSize, ResourceFormat format, HWMemoryUsage usage) {
		mVao = vao;
		mId = id;
		mBufferSize = bufferSize;
		mUsage = usage;
		mFormat = format;
	}
public:
	HWMemoryUsage GetUsage() const override { return mUsage; }
	int GetBufferSize() const override { return mBufferSize; }
	HardwareBufferType GetType() const override { return kHWBufferIndex; }

	int GetWidth() const override;
	ResourceFormat GetFormat() const override { return mFormat; }
	GLuint& GetId() { return mId; }
public:
	VertexArrayOGLPtr mVao;
	GLuint mId;
	size_t mBufferSize;
	HWMemoryUsage mUsage;
	ResourceFormat mFormat;
};

class ContantBufferOGL : public ImplementResource<IContantBuffer>
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	ContantBufferOGL() {}
	void Init(GLuint id, ConstBufferDeclPtr decl, HWMemoryUsage usage) {
		mId = id;
		mBufferSize = decl->BufferSize;
		mUsage = usage;
	}
public:
	HWMemoryUsage GetUsage() const override { return mUsage; }
	int GetBufferSize() const override { return mBufferSize; }
	HardwareBufferType GetType() const override { return kHWBufferConstant; }

	ConstBufferDeclPtr GetDecl() const override { return mDecl; }
	GLuint& GetId() { return mId; }
public:
	size_t mBufferSize;
	HWMemoryUsage mUsage;
	GLuint mId;
	ConstBufferDeclPtr mDecl;
};

}