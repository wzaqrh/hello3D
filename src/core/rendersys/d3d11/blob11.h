#pragma once
#include <windows.h>
#include <d3d11.h>
#include "core/rendersys/blob.h"

namespace mir {

class BlobData11 : public IBlobData {
public:
	BlobData11(ID3DBlob* pBlob);
	const char* GetBytes() const override;
	size_t GetSize() const override;
public:
	ID3DBlob* mBlob = nullptr;
};

}