#include "core/rendersys/ogl/blob_ogl.h"

namespace mir {

BlobDataOGL::BlobDataOGL()
{}

const char* BlobDataOGL::GetBytes() const
{
	return !mBinary.empty() ? &mBinary[0] : nullptr;
}

size_t BlobDataOGL::GetSize() const
{
	return mBinary.size();
}

}