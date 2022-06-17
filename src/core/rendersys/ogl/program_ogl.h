#pragma once
#include <windows.h>
#include <glad/glad.h>
#include "core/base/math.h"
#include "core/rendersys/ogl/predeclare.h"
#include "core/rendersys/program.h"

namespace mir {

class VertexShaderOGL : public ImplementResource<IVertexShader>
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	VertexShaderOGL(IBlobDataPtr pBlob, GLuint id) :mBlob(pBlob), mId(id) {}
	IBlobDataPtr GetBlob() const override { return mBlob; }

	GLuint GetId() { return mId; }
public:
	GLuint mId = 0;
	IBlobDataPtr mBlob;
};

class PixelShaderOGL : public ImplementResource<IPixelShader>
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	PixelShaderOGL(IBlobDataPtr pBlob, GLuint id) :mBlob(pBlob), mId(id) {}
	IBlobDataPtr GetBlob() const override { return mBlob; }

	GLuint GetId() { return mId; }
public:
	GLuint mId = 0;
	IBlobDataPtr mBlob;
};

class ProgramOGL : public ImplementResource<IProgram>
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	ProgramOGL() {}
	void Init(GLuint id) {
		mId = id;
	}
	void SetVertex(VertexShaderOGLPtr pVertex) {
		mVertex = pVertex;
	}
	void SetPixel(PixelShaderOGLPtr pPixel) {
		mPixel = pPixel;
	}
	IVertexShaderPtr GetVertex() const override { return mVertex; }
	IPixelShaderPtr GetPixel() const override { return mPixel; }
	GLuint GetId() const { return mId; }
public:
	GLuint mId = 0;
	VertexShaderOGLPtr mVertex;
	PixelShaderOGLPtr mPixel;
};

}