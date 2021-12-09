#pragma once
#include <windows.h>
#include <d3d11.h>
#include <d3d9.h>
#include "core/base/base_type.h"

namespace mir {
namespace d3d {

size_t BitsPerPixel(DXGI_FORMAT fmt);
size_t BytePerPixel(DXGI_FORMAT fmt);

enum CP_FLAGS : unsigned long
{
	CP_FLAGS_NONE = 0x0,      // Normal operation
	CP_FLAGS_LEGACY_DWORD = 0x1,      // Assume pitch is DWORD aligned instead of BYTE aligned
	CP_FLAGS_PARAGRAPH = 0x2,      // Assume pitch is 16-byte aligned instead of BYTE aligned
	CP_FLAGS_YMM = 0x4,      // Assume pitch is 32-byte aligned instead of BYTE aligned
	CP_FLAGS_ZMM = 0x8,      // Assume pitch is 64-byte aligned instead of BYTE aligned
	CP_FLAGS_PAGE4K = 0x200,    // Assume pitch is 4096-byte aligned instead of BYTE aligned
	CP_FLAGS_BAD_DXTN_TAILS = 0x1000,   // BC formats with malformed mipchain blocks smaller than 4x4
	CP_FLAGS_24BPP = 0x10000,  // Override with a legacy 24 bits-per-pixel format size
	CP_FLAGS_16BPP = 0x20000,  // Override with a legacy 16 bits-per-pixel format size
	CP_FLAGS_8BPP = 0x40000,  // Override with a legacy 8 bits-per-pixel format size
};
bool ComputePitch(DXGI_FORMAT fmt, size_t width, size_t height,
	size_t& rowPitch, size_t& slicePitch, int flags);

DXGI_FORMAT MakeTypeless(DXGI_FORMAT fmt);

bool IsDepthStencil(DXGI_FORMAT fmt);

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