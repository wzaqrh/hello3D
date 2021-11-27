#include "core/rendersys/d3d11/blob11.h"

namespace mir {

BlobData11::BlobData11(ID3DBlob* pBlob)
	:mBlob(pBlob)
{}

const char* BlobData11::GetBytes() const
{
	return (char*)mBlob->GetBufferPointer();
}

size_t BlobData11::GetSize() const
{
	return mBlob->GetBufferSize();
}

}