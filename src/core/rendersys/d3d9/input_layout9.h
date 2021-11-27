#pragma once
#include <windows.h>
#include <d3d9.h>
#include "core/rendersys/input_layout.h"

namespace mir {

class InputLayout9 : public ImplementResource<IInputLayout>
{
public:
	InputLayout9() :mLayout(nullptr) {}
	IDirect3DVertexDeclaration9*& GetLayout9() { return mLayout; }
public:
	std::vector<D3DVERTEXELEMENT9> mInputDescs;
	IDirect3DVertexDeclaration9* mLayout;
};

}