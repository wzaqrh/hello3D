#pragma once
#include <windows.h>
#include <d3d11.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;
#include "core/rendersys/input_layout.h"

namespace mir {

class InputLayout11 : public ImplementResource<IInputLayout>
{
public:
	InputLayout11() {}
	ComPtr<ID3D11InputLayout>& GetLayout11() { return mLayout; }
public:
	std::vector<D3D11_INPUT_ELEMENT_DESC> mInputDescs;
	ComPtr<ID3D11InputLayout> mLayout = nullptr;
};

}