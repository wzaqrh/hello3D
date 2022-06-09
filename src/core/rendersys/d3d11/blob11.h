#pragma once
#include <windows.h>
#include <d3d11.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;
#include "core/rendersys/blob.h"

namespace mir {

class BlobData11 : public IBlobData {
public:
	BlobData11(ComPtr<ID3DBlob>&& pBlob);
	const char* GetBytes() const override;
	size_t GetSize() const override;
public:
	ComPtr<ID3DBlob> mBlob = nullptr;
};

}