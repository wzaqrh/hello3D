#include "core/rendersys/interface_type.h"
#include "core/rendersys/render_system.h"
#include "core/rendersys/d3d11/interface_type11.h"

namespace mir {

/********** TBlobDataStd **********/
BlobDataStandard::BlobDataStandard(const std::vector<char>& buffer)
	:mBuffer(buffer)
{
}

char* BlobDataStandard::GetBufferPointer()
{
	return mBuffer.empty() ? nullptr : &mBuffer[0];
}

size_t BlobDataStandard::GetBufferSize()
{
	return mBuffer.size();
}

/********** IIndexBuffer **********/
int IIndexBuffer::GetCount()
{
	return GetBufferSize() / GetWidth();
}

}