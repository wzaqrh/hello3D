#include "core/rendersys/d3d11/blob11.h"

namespace mir {

BlobData11::BlobData11(ComPtr<ID3DBlob>&& pBlob)
:mBlob(std::move(pBlob))
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