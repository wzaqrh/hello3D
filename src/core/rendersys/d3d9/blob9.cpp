#include "core/rendersys/d3d9/blob9.h"

namespace mir {

BlobData9::BlobData9(ID3DXBuffer* pBlob)
	:mBlob(pBlob)
{}

const char* BlobData9::GetBytes() const
{
	return (char*)mBlob->GetBufferPointer();
}

size_t BlobData9::GetSize() const
{
	return mBlob->GetBufferSize();
}

}