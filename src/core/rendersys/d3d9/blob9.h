#pragma once
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "core/rendersys/blob.h"

namespace mir {

class BlobData9 : public IBlobData
{
public:
	BlobData9(ID3DXBuffer* pBlob);
	const char* GetBytes() const override;
	size_t GetSize() const override;
public:
	ID3DXBuffer* mBlob;
};

}