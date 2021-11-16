#pragma once
#include <windows.h>
#include <d3d11.h>
#include <d3d9.h>
#include "core/rendersys/base_type.h"

namespace mir {
namespace d3d {

int GetFormatWidthByte(DXGI_FORMAT format);

D3DBLEND convert11To9(D3D11_BLEND blend);
D3DCMPFUNC convert11To9(D3D11_COMPARISON_FUNC cmp);
DXGI_FORMAT convert9To11(D3DFORMAT fmt);
D3DFORMAT convert11To9(DXGI_FORMAT fmt);
D3DPRIMITIVETYPE convert11To9(D3D11_PRIMITIVE_TOPOLOGY topo);
D3DVERTEXELEMENT9 convert11To9(const LayoutInputElement& desc);
D3DTEXTUREADDRESS convert11To9(D3D11_TEXTURE_ADDRESS_MODE addrMode);
std::map<D3DSAMPLERSTATETYPE, D3DTEXTUREFILTERTYPE> convert11To9(D3D11_FILTER filter);

}
}