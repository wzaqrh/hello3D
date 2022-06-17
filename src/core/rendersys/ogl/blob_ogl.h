#pragma once
#include <windows.h>
#include <glad/glad.h>
#include <GL/GL.h>
#include "core/base/data.h"
#include "core/rendersys/blob.h"

namespace mir {

class BlobDataOGL : public IBlobData {
public:
	BlobDataOGL(GLuint id);
	const char* GetBytes() const override;
	size_t GetSize() const override;
	GLuint GetId() const { return mId; }
public:
	Data mBlob;
	GLuint mId = 0;
};

}