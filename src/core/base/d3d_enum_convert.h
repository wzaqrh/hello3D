#pragma once
//INCLUDE_PREDEFINE_H
#include "core/rendersys/base_type.h"

class D3DEnumCT {
public:
	static D3DBLEND d3d11To9(D3D11_BLEND blend);
	static D3DCMPFUNC d3d11To9(D3D11_COMPARISON_FUNC cmp);
	static DXGI_FORMAT d3d9To11(D3DFORMAT fmt);
	static D3DFORMAT d3d11To9(DXGI_FORMAT fmt);
	static int GetWidth(DXGI_FORMAT format);
	static D3DPRIMITIVETYPE d3d11To9(D3D11_PRIMITIVE_TOPOLOGY topo);
	static D3DVERTEXELEMENT9 d3d11To9(const D3D11_INPUT_ELEMENT_DESC& desc);

	static D3DTEXTUREADDRESS d3d11To9(D3D11_TEXTURE_ADDRESS_MODE addrMode);
	static std::map<D3DSAMPLERSTATETYPE, D3DTEXTUREFILTERTYPE> d3d11To9(D3D11_FILTER filter);
};
