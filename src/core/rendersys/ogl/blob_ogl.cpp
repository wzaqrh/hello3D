#include "core/rendersys/ogl/blob_ogl.h"

namespace mir {

BlobDataOGL::BlobDataOGL(GLuint id)
	:mId(id)
{}

const char* BlobDataOGL::GetBytes() const
{
	return (char*)mBlob.Bytes;
}

size_t BlobDataOGL::GetSize() const
{
	return mBlob.Size;
}

}