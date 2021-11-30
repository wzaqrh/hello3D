#include <boost/assert.hpp>
#include "core/base/d3d.h"

namespace mir {
namespace d3d {

/******* convert *******/
D3DBLEND convert11To9(D3D11_BLEND blend)
{
	return static_cast<D3DBLEND>(blend);
}

D3DCMPFUNC convert11To9(D3D11_COMPARISON_FUNC cmp)
{
	return static_cast<D3DCMPFUNC>(cmp);
}

D3DFORMAT convert11To9(DXGI_FORMAT fmt)
{
	D3DFORMAT ret = D3DFMT_UNKNOWN;
	switch (fmt) {
	case DXGI_FORMAT_UNKNOWN:
		return D3DFMT_UNKNOWN;
	case DXGI_FORMAT_R32G32B32A32_TYPELESS:
		return D3DFMT_A32B32G32R32F; //break;
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
		return D3DFMT_A32B32G32R32F;
	case DXGI_FORMAT_R32G32B32A32_UINT:
		break;
	case DXGI_FORMAT_R32G32B32A32_SINT:
		break;
	case DXGI_FORMAT_R32G32B32_TYPELESS:
		break;
	case DXGI_FORMAT_R32G32B32_FLOAT:
		break;// return D3DDECLTYPE_FLOAT3;
	case DXGI_FORMAT_R32G32B32_UINT:
		break;
	case DXGI_FORMAT_R32G32B32_SINT:
		break;
	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
		break;
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
		return D3DFMT_A16B16G16R16F;
	case DXGI_FORMAT_R16G16B16A16_UNORM:
		return D3DFMT_A16B16G16R16;
	case DXGI_FORMAT_R16G16B16A16_UINT:
		break;
	case DXGI_FORMAT_R16G16B16A16_SNORM:
		return D3DFMT_Q16W16V16U16;
	case DXGI_FORMAT_R16G16B16A16_SINT:
		break;
	case DXGI_FORMAT_R32G32_TYPELESS:
		break;
	case DXGI_FORMAT_R32G32_FLOAT:
		return D3DFMT_G32R32F;
	case DXGI_FORMAT_R32G32_UINT:
		break;
	case DXGI_FORMAT_R32G32_SINT:
		break;
	case DXGI_FORMAT_R32G8X24_TYPELESS:
		break;
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		break;
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		break;
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		break;
	case DXGI_FORMAT_R10G10B10A2_TYPELESS:
	case DXGI_FORMAT_R10G10B10A2_UNORM:
	case DXGI_FORMAT_R10G10B10A2_UINT:
		return D3DFMT_A2B10G10R10;
	case DXGI_FORMAT_R11G11B10_FLOAT:
		break;
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		break;
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		return D3DFMT_A8B8G8R8;
	case DXGI_FORMAT_R8G8B8A8_UINT:
		break;// return D3DDECLTYPE_UBYTE4;
	case DXGI_FORMAT_R8G8B8A8_SNORM:
		return D3DFMT_Q8W8V8U8;
	case DXGI_FORMAT_R8G8B8A8_SINT:
		break;
	case DXGI_FORMAT_R16G16_TYPELESS:
		break;
	case DXGI_FORMAT_R16G16_FLOAT:
		return D3DFMT_G16R16F;
	case DXGI_FORMAT_R16G16_UNORM:
		return D3DFMT_G16R16;
	case DXGI_FORMAT_R16G16_UINT:
		break;
	case DXGI_FORMAT_R16G16_SNORM:
		return D3DFMT_V16U16;
	case DXGI_FORMAT_R16G16_SINT:
		break;// return D3DDECLTYPE_SHORT2;
	case DXGI_FORMAT_R32_TYPELESS:
		break;
	case DXGI_FORMAT_D32_FLOAT:
		return D3DFMT_D32F_LOCKABLE;
	case DXGI_FORMAT_R32_FLOAT:
		return D3DFMT_R32F;
	case DXGI_FORMAT_R32_UINT:
		return D3DFMT_INDEX32;
	case DXGI_FORMAT_R32_SINT:
		break;
	case DXGI_FORMAT_R24G8_TYPELESS:
		break;
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
		return D3DFMT_D24S8;
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		break;
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		break;
	case DXGI_FORMAT_R8G8_TYPELESS:
		break;
	case DXGI_FORMAT_R8G8_UNORM:
		return D3DFMT_A8L8;
	case DXGI_FORMAT_R8G8_UINT:
		break;
	case DXGI_FORMAT_R8G8_SNORM:
		return D3DFMT_V8U8;
	case DXGI_FORMAT_R8G8_SINT:
		break;
	case DXGI_FORMAT_R16_TYPELESS:
		break;
	case DXGI_FORMAT_R16_FLOAT:
		return D3DFMT_R16F;
	case DXGI_FORMAT_D16_UNORM:
		return D3DFMT_D16;
	case DXGI_FORMAT_R16_UNORM:
		return D3DFMT_L16;
	case DXGI_FORMAT_R16_UINT:
		return D3DFMT_INDEX16;
	case DXGI_FORMAT_R16_SNORM:
		break;
	case DXGI_FORMAT_R16_SINT:
		break;
	case DXGI_FORMAT_R8_TYPELESS:
		break;
	case DXGI_FORMAT_R8_UNORM:
		return D3DFMT_L8;
	case DXGI_FORMAT_R8_UINT:
		break;
	case DXGI_FORMAT_R8_SNORM:
		break;
	case DXGI_FORMAT_R8_SINT:
		break;
	case DXGI_FORMAT_A8_UNORM:
		return D3DFMT_A8;
	case DXGI_FORMAT_R1_UNORM:
		break;
	case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
		break;
	case DXGI_FORMAT_R8G8_B8G8_UNORM:
		return D3DFMT_G8R8_G8B8;
	case DXGI_FORMAT_G8R8_G8B8_UNORM:
		return D3DFMT_R8G8_B8G8;
	case DXGI_FORMAT_BC1_TYPELESS:
		break;
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
		return D3DFMT_DXT1;
	case DXGI_FORMAT_BC2_TYPELESS:
		break;
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
		return D3DFMT_DXT3;
	case DXGI_FORMAT_BC3_TYPELESS:
		break;
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
		return D3DFMT_DXT5;
	case DXGI_FORMAT_BC4_TYPELESS:
		break;
	case DXGI_FORMAT_BC4_UNORM:
		break;
	case DXGI_FORMAT_BC4_SNORM:
		break;
	case DXGI_FORMAT_BC5_TYPELESS:
		break;
	case DXGI_FORMAT_BC5_UNORM:
		break;
	case DXGI_FORMAT_BC5_SNORM:
		break;
	case DXGI_FORMAT_B5G6R5_UNORM:
		return D3DFMT_R5G6B5;
	case DXGI_FORMAT_B5G5R5A1_UNORM:
		return D3DFMT_A1R5G5B5;
	case DXGI_FORMAT_B8G8R8A8_UNORM:
		return D3DFMT_A8R8G8B8;
	case DXGI_FORMAT_B8G8R8X8_UNORM:
		return D3DFMT_X8R8G8B8;
	case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
		break;
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:
		break;
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		return D3DFMT_A8R8G8B8;
	case DXGI_FORMAT_B8G8R8X8_TYPELESS:
		break;
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		return D3DFMT_X8R8G8B8;
	case DXGI_FORMAT_BC6H_TYPELESS:
		break;
	case DXGI_FORMAT_BC6H_UF16:
		break;
	case DXGI_FORMAT_BC6H_SF16:
		break;
	case DXGI_FORMAT_BC7_TYPELESS:
		break;
	case DXGI_FORMAT_BC7_UNORM:
		break;
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		break;
	case DXGI_FORMAT_FORCE_UINT:
		break;
	default:
		break;
	}
	assert(false);
	return ret;
}

D3DPRIMITIVETYPE convert11To9(D3D11_PRIMITIVE_TOPOLOGY topo)
{
	D3DPRIMITIVETYPE ret = D3DPT_FORCE_DWORD;
	switch (topo) {
	case D3D_PRIMITIVE_TOPOLOGY_POINTLIST:
		return D3DPT_POINTLIST;
	case D3D_PRIMITIVE_TOPOLOGY_LINELIST:
		return D3DPT_LINELIST;
	case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP:
		return D3DPT_LINESTRIP;
	case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
		return D3DPT_TRIANGLELIST;
	case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
		return D3DPT_TRIANGLESTRIP;
	case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ:
		return D3DPT_TRIANGLEFAN;
	default:
		assert(false);
		break;
	}
	return ret;
}

static D3DDECLUSAGE __convert11To9(const std::string& semantic) {
	D3DDECLUSAGE ret;
	if (semantic.find("POSITION") != std::string::npos) {
		ret = D3DDECLUSAGE_POSITION;
	}
	else if (semantic.find("BLENDWEIGHT") != std::string::npos) {
		ret = D3DDECLUSAGE_BLENDWEIGHT;
	}
	else if (semantic.find("BLENDINDICES") != std::string::npos) {
		ret = D3DDECLUSAGE_BLENDINDICES;
	}
	else if (semantic.find("NORMAL") != std::string::npos) {
		ret = D3DDECLUSAGE_NORMAL;
	}
	else if (semantic.find("PSIZE") != std::string::npos) {
		ret = D3DDECLUSAGE_PSIZE;
	}
	else if (semantic.find("TEXCOORD") != std::string::npos) {
		ret = D3DDECLUSAGE_TEXCOORD;
	}
	else if (semantic.find("TANGENT") != std::string::npos) {
		ret = D3DDECLUSAGE_TANGENT;
	}
	else if (semantic.find("BINORMAL") != std::string::npos) {
		ret = D3DDECLUSAGE_BINORMAL;
	}
	else if (semantic.find("TESSFACTOR") != std::string::npos) {
		ret = D3DDECLUSAGE_TESSFACTOR;
	}
	else if (semantic.find("POSITIONT") != std::string::npos) {
		ret = D3DDECLUSAGE_POSITIONT;
	}
	else if (semantic.find("COLOR") != std::string::npos) {
		ret = D3DDECLUSAGE_COLOR;
	}
	else if (semantic.find("FOG") != std::string::npos) {
		ret = D3DDECLUSAGE_FOG;
	}
	else if (semantic.find("DEPTH") != std::string::npos) {
		ret = D3DDECLUSAGE_DEPTH;
	}
	else {
		assert(false);
	}
	return ret;
}

static D3DDECLTYPE __convert11To9(DXGI_FORMAT fmt) {
	D3DDECLTYPE ret = D3DDECLTYPE_UNUSED;
	switch (fmt) {
	case DXGI_FORMAT_R32_FLOAT:
		return D3DDECLTYPE_FLOAT1;
	case DXGI_FORMAT_R32G32_FLOAT:
		return D3DDECLTYPE_FLOAT2;
	case DXGI_FORMAT_R32G32B32_FLOAT:
		return D3DDECLTYPE_FLOAT3;
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
		return D3DDECLTYPE_FLOAT4;
	case DXGI_FORMAT_R32G32B32A32_UINT:
	case DXGI_FORMAT_R32G32B32A32_SINT:
		return D3DDECLTYPE_UBYTE4;
	case DXGI_FORMAT_R16G16_SINT:
		return D3DDECLTYPE_SHORT2;
	case DXGI_FORMAT_R16G16B16A16_SINT:
		return D3DDECLTYPE_SHORT4;

	case DXGI_FORMAT_R8G8B8A8_UINT:
		return D3DDECLTYPE_UBYTE4;
	case DXGI_FORMAT_R8G8B8A8_UNORM:
		return D3DDECLTYPE_UBYTE4N;

	case DXGI_FORMAT_R16G16_SNORM:
		return D3DDECLTYPE_SHORT2N;
	case DXGI_FORMAT_R16G16B16A16_SNORM:
		return D3DDECLTYPE_SHORT4N;
	case DXGI_FORMAT_R16G16_UNORM:
		return D3DDECLTYPE_USHORT2N;
	case DXGI_FORMAT_R16G16B16A16_UNORM:
		return D3DDECLTYPE_USHORT4N;

	case DXGI_FORMAT_R16G16_FLOAT:
		return D3DDECLTYPE_FLOAT16_2;
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
		return D3DDECLTYPE_FLOAT16_4;
	default:
		assert(false);
		break;
	}
	return ret;
}

D3DVERTEXELEMENT9 convert11To9(const LayoutInputElement& desc)
{
	D3DVERTEXELEMENT9 ret;
	ret.Stream = desc.InputSlot;
	ret.Offset = desc.AlignedByteOffset;
	ret.Type = (D3DDECLTYPE)__convert11To9(static_cast<DXGI_FORMAT>(desc.Format));
	ret.Method = D3DDECLMETHOD_DEFAULT;
	ret.Usage = __convert11To9(desc.SemanticName);
	ret.UsageIndex = desc.SemanticIndex;
	return ret;
}

D3DTEXTUREADDRESS convert11To9(D3D11_TEXTURE_ADDRESS_MODE addrMode)
{
	return static_cast<D3DTEXTUREADDRESS>(addrMode);
}

std::map<D3DSAMPLERSTATETYPE, D3DTEXTUREFILTERTYPE> convert11To9(D3D11_FILTER filter)
{
	std::map<D3DSAMPLERSTATETYPE, D3DTEXTUREFILTERTYPE> ret;
	switch (filter) {
	case D3D11_FILTER_MIN_MAG_MIP_POINT:
		ret[D3DSAMP_MINFILTER] = D3DTEXF_POINT;
		ret[D3DSAMP_MAGFILTER] = D3DTEXF_POINT;
		ret[D3DSAMP_MIPFILTER] = D3DTEXF_POINT;
		break;
	case D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR:
		ret[D3DSAMP_MINFILTER] = D3DTEXF_POINT;
		ret[D3DSAMP_MAGFILTER] = D3DTEXF_POINT;
		ret[D3DSAMP_MIPFILTER] = D3DTEXF_LINEAR;
		break;
	case D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT:
		ret[D3DSAMP_MINFILTER] = D3DTEXF_POINT;
		ret[D3DSAMP_MAGFILTER] = D3DTEXF_LINEAR;
		ret[D3DSAMP_MIPFILTER] = D3DTEXF_POINT;
		break;
	case D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR:
		ret[D3DSAMP_MINFILTER] = D3DTEXF_POINT;
		ret[D3DSAMP_MAGFILTER] = D3DTEXF_LINEAR;
		ret[D3DSAMP_MIPFILTER] = D3DTEXF_LINEAR;
		break;
	case D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT:
		ret[D3DSAMP_MINFILTER] = D3DTEXF_LINEAR;
		ret[D3DSAMP_MAGFILTER] = D3DTEXF_POINT;
		ret[D3DSAMP_MIPFILTER] = D3DTEXF_POINT;
		break;
	case D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
		ret[D3DSAMP_MINFILTER] = D3DTEXF_LINEAR;
		ret[D3DSAMP_MAGFILTER] = D3DTEXF_POINT;
		ret[D3DSAMP_MIPFILTER] = D3DTEXF_LINEAR;
		break;
	case D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT:
		ret[D3DSAMP_MINFILTER] = D3DTEXF_LINEAR;
		ret[D3DSAMP_MAGFILTER] = D3DTEXF_LINEAR;
		ret[D3DSAMP_MIPFILTER] = D3DTEXF_POINT;
		break;
	case D3D11_FILTER_MIN_MAG_MIP_LINEAR:
		ret[D3DSAMP_MINFILTER] = D3DTEXF_LINEAR;
		ret[D3DSAMP_MAGFILTER] = D3DTEXF_LINEAR;
		ret[D3DSAMP_MIPFILTER] = D3DTEXF_LINEAR;
		break;
	case D3D11_FILTER_ANISOTROPIC:
		ret[D3DSAMP_MINFILTER] = D3DTEXF_ANISOTROPIC;
		ret[D3DSAMP_MAGFILTER] = D3DTEXF_ANISOTROPIC;
		ret[D3DSAMP_MIPFILTER] = D3DTEXF_ANISOTROPIC;
		break;
	default:
		break;
	}
	return ret;
}

DXGI_FORMAT convert9To11(D3DFORMAT fmt)
{
	DXGI_FORMAT ret = DXGI_FORMAT_UNKNOWN;
	switch (fmt) {
	case D3DFMT_A8R8G8B8:
		return DXGI_FORMAT_B8G8R8A8_UNORM;
	case D3DFMT_R5G6B5:
		return DXGI_FORMAT_B5G6R5_UNORM;
	case D3DFMT_A1R5G5B5:
		return DXGI_FORMAT_B5G5R5A1_UNORM;
	case D3DFMT_A8:
		return DXGI_FORMAT_A8_UNORM;
	case D3DFMT_A8B8G8R8:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case D3DFMT_D24S8:
		return DXGI_FORMAT_D24_UNORM_S8_UINT;
	default:
		break;
	}
	return ret;
}

/******* bpp *******/
size_t BitsPerPixel(DXGI_FORMAT fmt)
{
	switch (static_cast<int>(fmt)) {
	case DXGI_FORMAT_R32G32B32A32_TYPELESS:
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
	case DXGI_FORMAT_R32G32B32A32_UINT:
	case DXGI_FORMAT_R32G32B32A32_SINT:
		return 128;

	case DXGI_FORMAT_R32G32B32_TYPELESS:
	case DXGI_FORMAT_R32G32B32_FLOAT:
	case DXGI_FORMAT_R32G32B32_UINT:
	case DXGI_FORMAT_R32G32B32_SINT:
		return 96;

	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
	case DXGI_FORMAT_R16G16B16A16_UNORM:
	case DXGI_FORMAT_R16G16B16A16_UINT:
	case DXGI_FORMAT_R16G16B16A16_SNORM:
	case DXGI_FORMAT_R16G16B16A16_SINT:
	case DXGI_FORMAT_R32G32_TYPELESS:
	case DXGI_FORMAT_R32G32_FLOAT:
	case DXGI_FORMAT_R32G32_UINT:
	case DXGI_FORMAT_R32G32_SINT:
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
	case DXGI_FORMAT_Y416:
	case DXGI_FORMAT_Y210:
	case DXGI_FORMAT_Y216:
		return 64;

	case DXGI_FORMAT_R10G10B10A2_TYPELESS:
	case DXGI_FORMAT_R10G10B10A2_UNORM:
	case DXGI_FORMAT_R10G10B10A2_UINT:
	case DXGI_FORMAT_R11G11B10_FLOAT:
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SNORM:
	case DXGI_FORMAT_R8G8B8A8_SINT:
	case DXGI_FORMAT_R16G16_TYPELESS:
	case DXGI_FORMAT_R16G16_FLOAT:
	case DXGI_FORMAT_R16G16_UNORM:
	case DXGI_FORMAT_R16G16_UINT:
	case DXGI_FORMAT_R16G16_SNORM:
	case DXGI_FORMAT_R16G16_SINT:
	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R32_FLOAT:
	case DXGI_FORMAT_R32_UINT:
	case DXGI_FORMAT_R32_SINT:
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
	case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
	case DXGI_FORMAT_R8G8_B8G8_UNORM:
	case DXGI_FORMAT_G8R8_G8B8_UNORM:
	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_B8G8R8X8_UNORM:
	case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
	case DXGI_FORMAT_B8G8R8X8_TYPELESS:
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
	case DXGI_FORMAT_AYUV:
	case DXGI_FORMAT_Y410:
	case DXGI_FORMAT_YUY2:
	/*case XBOX_DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT:
	case XBOX_DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT:
	case XBOX_DXGI_FORMAT_R10G10B10_SNORM_A2_UNORM:*/
		return 32;

	case DXGI_FORMAT_P010:
	case DXGI_FORMAT_P016:
	//case XBOX_DXGI_FORMAT_D16_UNORM_S8_UINT:
	//case XBOX_DXGI_FORMAT_R16_UNORM_X8_TYPELESS:
	//case XBOX_DXGI_FORMAT_X16_TYPELESS_G8_UINT:
	//case WIN10_DXGI_FORMAT_V408:
		return 24;

	case DXGI_FORMAT_R8G8_TYPELESS:
	case DXGI_FORMAT_R8G8_UNORM:
	case DXGI_FORMAT_R8G8_UINT:
	case DXGI_FORMAT_R8G8_SNORM:
	case DXGI_FORMAT_R8G8_SINT:
	case DXGI_FORMAT_R16_TYPELESS:
	case DXGI_FORMAT_R16_FLOAT:
	case DXGI_FORMAT_D16_UNORM:
	case DXGI_FORMAT_R16_UNORM:
	case DXGI_FORMAT_R16_UINT:
	case DXGI_FORMAT_R16_SNORM:
	case DXGI_FORMAT_R16_SINT:
	case DXGI_FORMAT_B5G6R5_UNORM:
	case DXGI_FORMAT_B5G5R5A1_UNORM:
	case DXGI_FORMAT_A8P8:
	case DXGI_FORMAT_B4G4R4A4_UNORM:
	//case WIN10_DXGI_FORMAT_P208:
	//case WIN10_DXGI_FORMAT_V208:
		return 16;

	case DXGI_FORMAT_NV12:
	case DXGI_FORMAT_420_OPAQUE:
	case DXGI_FORMAT_NV11:
		return 12;

	case DXGI_FORMAT_R8_TYPELESS:
	case DXGI_FORMAT_R8_UNORM:
	case DXGI_FORMAT_R8_UINT:
	case DXGI_FORMAT_R8_SNORM:
	case DXGI_FORMAT_R8_SINT:
	case DXGI_FORMAT_A8_UNORM:
	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM:
	case DXGI_FORMAT_BC6H_TYPELESS:
	case DXGI_FORMAT_BC6H_UF16:
	case DXGI_FORMAT_BC6H_SF16:
	case DXGI_FORMAT_BC7_TYPELESS:
	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
	case DXGI_FORMAT_AI44:
	case DXGI_FORMAT_IA44:
	case DXGI_FORMAT_P8:
	//case XBOX_DXGI_FORMAT_R4G4_UNORM:
		return 8;

	case DXGI_FORMAT_R1_UNORM:
		return 1;

	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM:
		return 4;

	default:
		return 0;
	}
}

size_t BytePerPixel(DXGI_FORMAT fmt)
{
	return (BitsPerPixel(fmt) + 7) / 8;
}

bool ComputePitch(DXGI_FORMAT fmt, size_t width, size_t height,
	size_t& rowPitch, size_t& slicePitch, int flags)
{
	uint64_t pitch = 0;
	uint64_t slice = 0;

	switch (static_cast<int>(fmt)) {
	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM:
		//BOOST_ASSERT(IsCompressed(fmt));
		{
			if (flags & CP_FLAGS_BAD_DXTN_TAILS) {
				size_t nbw = width >> 2;
				size_t nbh = height >> 2;
				pitch = std::max<uint64_t>(1u, uint64_t(nbw) * 8u);
				slice = std::max<uint64_t>(1u, pitch * uint64_t(nbh));
			}
			else {
				uint64_t nbw = std::max<uint64_t>(1u, (uint64_t(width) + 3u) / 4u);
				uint64_t nbh = std::max<uint64_t>(1u, (uint64_t(height) + 3u) / 4u);
				pitch = nbw * 8u;
				slice = pitch * nbh;
			}
		}
		break;

	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM:
	case DXGI_FORMAT_BC6H_TYPELESS:
	case DXGI_FORMAT_BC6H_UF16:
	case DXGI_FORMAT_BC6H_SF16:
	case DXGI_FORMAT_BC7_TYPELESS:
	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		//assert(IsCompressed(fmt));
		{
			if (flags & CP_FLAGS_BAD_DXTN_TAILS) {
				size_t nbw = width >> 2;
				size_t nbh = height >> 2;
				pitch = std::max<uint64_t>(1u, uint64_t(nbw) * 16u);
				slice = std::max<uint64_t>(1u, pitch * uint64_t(nbh));
			}
			else {
				uint64_t nbw = std::max<uint64_t>(1u, (uint64_t(width) + 3u) / 4u);
				uint64_t nbh = std::max<uint64_t>(1u, (uint64_t(height) + 3u) / 4u);
				pitch = nbw * 16u;
				slice = pitch * nbh;
			}
		}
		break;

	case DXGI_FORMAT_R8G8_B8G8_UNORM:
	case DXGI_FORMAT_G8R8_G8B8_UNORM:
	case DXGI_FORMAT_YUY2:
		//assert(IsPacked(fmt));
		pitch = ((uint64_t(width) + 1u) >> 1) * 4u;
		slice = pitch * uint64_t(height);
		break;

	case DXGI_FORMAT_Y210:
	case DXGI_FORMAT_Y216:
		//assert(IsPacked(fmt));
		pitch = ((uint64_t(width) + 1u) >> 1) * 8u;
		slice = pitch * uint64_t(height);
		break;

	case DXGI_FORMAT_NV12:
	case DXGI_FORMAT_420_OPAQUE:
		//assert(IsPlanar(fmt));
		pitch = ((uint64_t(width) + 1u) >> 1) * 2u;
		slice = pitch * (uint64_t(height) + ((uint64_t(height) + 1u) >> 1));
		break;

	case DXGI_FORMAT_P010:
	case DXGI_FORMAT_P016:
	/*case XBOX_DXGI_FORMAT_D16_UNORM_S8_UINT:
	case XBOX_DXGI_FORMAT_R16_UNORM_X8_TYPELESS:
	case XBOX_DXGI_FORMAT_X16_TYPELESS_G8_UINT:*/
		//assert(IsPlanar(fmt));
		pitch = ((uint64_t(width) + 1u) >> 1) * 4u;
		slice = pitch * (uint64_t(height) + ((uint64_t(height) + 1u) >> 1));
		break;

	case DXGI_FORMAT_NV11:
		//assert(IsPlanar(fmt));
		pitch = ((uint64_t(width) + 3u) >> 2) * 4u;
		slice = pitch * uint64_t(height) * 2u;
		break;

	/*case WIN10_DXGI_FORMAT_P208:
		assert(IsPlanar(fmt));
		pitch = ((uint64_t(width) + 1u) >> 1) * 2u;
		slice = pitch * uint64_t(height) * 2u;
		break;

	case WIN10_DXGI_FORMAT_V208:
		assert(IsPlanar(fmt));
		pitch = uint64_t(width);
		slice = pitch * (uint64_t(height) + (((uint64_t(height) + 1u) >> 1) * 2u));
		break;

	case WIN10_DXGI_FORMAT_V408:
		assert(IsPlanar(fmt));
		pitch = uint64_t(width);
		slice = pitch * (uint64_t(height) + (uint64_t(height >> 1) * 4u));
		break;*/

	default:
		//assert(!IsCompressed(fmt) && !IsPacked(fmt) && !IsPlanar(fmt));
		{
			size_t bpp;

			if (flags & CP_FLAGS_24BPP)
				bpp = 24;
			else if (flags & CP_FLAGS_16BPP)
				bpp = 16;
			else if (flags & CP_FLAGS_8BPP)
				bpp = 8;
			else
				bpp = BitsPerPixel(fmt);

			if (!bpp)
				return false;// E_INVALIDARG;

			if (flags & (CP_FLAGS_LEGACY_DWORD | CP_FLAGS_PARAGRAPH | CP_FLAGS_YMM | CP_FLAGS_ZMM | CP_FLAGS_PAGE4K)) {
				if (flags & CP_FLAGS_PAGE4K) {
					pitch = ((uint64_t(width) * bpp + 32767u) / 32768u) * 4096u;
					slice = pitch * uint64_t(height);
				}
				else if (flags & CP_FLAGS_ZMM) {
					pitch = ((uint64_t(width) * bpp + 511u) / 512u) * 64u;
					slice = pitch * uint64_t(height);
				}
				else if (flags & CP_FLAGS_YMM) {
					pitch = ((uint64_t(width) * bpp + 255u) / 256u) * 32u;
					slice = pitch * uint64_t(height);
				}
				else if (flags & CP_FLAGS_PARAGRAPH) {
					pitch = ((uint64_t(width) * bpp + 127u) / 128u) * 16u;
					slice = pitch * uint64_t(height);
				}
				else // DWORD alignment
				{
					// Special computation for some incorrectly created DDS files based on
					// legacy DirectDraw assumptions about pitch alignment
					pitch = ((uint64_t(width) * bpp + 31u) / 32u) * sizeof(uint32_t);
					slice = pitch * uint64_t(height);
				}
			}
			else {
				// Default byte alignment
				pitch = (uint64_t(width) * bpp + 7u) / 8u;
				slice = pitch * uint64_t(height);
			}
		}
		break;
	}

#if defined(_M_IX86) || defined(_M_ARM) || defined(_M_HYBRID_X86_ARM64)
	static_assert(sizeof(size_t) == 4, "Not a 32-bit platform!");
	if (pitch > UINT32_MAX || slice > UINT32_MAX) {
		rowPitch = slicePitch = 0;
		return false;
	}
#else
	static_assert(sizeof(size_t) == 8, "Not a 64-bit platform!");
#endif

	rowPitch = static_cast<size_t>(pitch);
	slicePitch = static_cast<size_t>(slice);

	return true;
}

/******* typeless *******/
bool IsDepthStencil(DXGI_FORMAT fmt)
{
	switch (static_cast<int>(fmt)) {
	case DXGI_FORMAT_R32G8X24_TYPELESS://typeless
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS://typeless
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT://typeless
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		return true;
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_D16_UNORM:
		return true;
	case DXGI_FORMAT_R24G8_TYPELESS://typeless
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS://typeless
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT://typeless
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	//case XBOX_DXGI_FORMAT_D16_UNORM_S8_UINT:
	//case XBOX_DXGI_FORMAT_R16_UNORM_X8_TYPELESS:
	//case XBOX_DXGI_FORMAT_X16_TYPELESS_G8_UINT:
		return true;
	default:
		return false;
	}
}

/******* typeless *******/
DXGI_FORMAT MakeTypeless(DXGI_FORMAT fmt)
{
	switch (static_cast<int>(fmt)) {
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
	case DXGI_FORMAT_R32G32B32A32_UINT:
	case DXGI_FORMAT_R32G32B32A32_SINT:
		return DXGI_FORMAT_R32G32B32A32_TYPELESS;

	case DXGI_FORMAT_R32G32B32_FLOAT:
	case DXGI_FORMAT_R32G32B32_UINT:
	case DXGI_FORMAT_R32G32B32_SINT:
		return DXGI_FORMAT_R32G32B32_TYPELESS;

	case DXGI_FORMAT_R16G16B16A16_FLOAT:
	case DXGI_FORMAT_R16G16B16A16_UNORM:
	case DXGI_FORMAT_R16G16B16A16_UINT:
	case DXGI_FORMAT_R16G16B16A16_SNORM:
	case DXGI_FORMAT_R16G16B16A16_SINT:
		return DXGI_FORMAT_R16G16B16A16_TYPELESS;

	case DXGI_FORMAT_R32G32_FLOAT:
	case DXGI_FORMAT_R32G32_UINT:
	case DXGI_FORMAT_R32G32_SINT:
		return DXGI_FORMAT_R32G32_TYPELESS;

	case DXGI_FORMAT_R10G10B10A2_UNORM:
	case DXGI_FORMAT_R10G10B10A2_UINT:
	//case XBOX_DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT:
	//case XBOX_DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT:
	//case XBOX_DXGI_FORMAT_R10G10B10_SNORM_A2_UNORM:
		return DXGI_FORMAT_R10G10B10A2_TYPELESS;

	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SNORM:
	case DXGI_FORMAT_R8G8B8A8_SINT:
		return DXGI_FORMAT_R8G8B8A8_TYPELESS;

	case DXGI_FORMAT_R16G16_FLOAT:
	case DXGI_FORMAT_R16G16_UNORM:
	case DXGI_FORMAT_R16G16_UINT:
	case DXGI_FORMAT_R16G16_SNORM:
	case DXGI_FORMAT_R16G16_SINT:
		return DXGI_FORMAT_R16G16_TYPELESS;

	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R32_FLOAT:
	case DXGI_FORMAT_R32_UINT:
	case DXGI_FORMAT_R32_SINT:
		return DXGI_FORMAT_R32_TYPELESS;

	case DXGI_FORMAT_D24_UNORM_S8_UINT:
		return DXGI_FORMAT_R24G8_TYPELESS;

	case DXGI_FORMAT_R8G8_UNORM:
	case DXGI_FORMAT_R8G8_UINT:
	case DXGI_FORMAT_R8G8_SNORM:
	case DXGI_FORMAT_R8G8_SINT:
		return DXGI_FORMAT_R8G8_TYPELESS;

	case DXGI_FORMAT_R16_FLOAT:
	case DXGI_FORMAT_D16_UNORM:
	case DXGI_FORMAT_R16_UNORM:
	case DXGI_FORMAT_R16_UINT:
	case DXGI_FORMAT_R16_SNORM:
	case DXGI_FORMAT_R16_SINT:
		return DXGI_FORMAT_R16_TYPELESS;

	case DXGI_FORMAT_R8_UNORM:
	case DXGI_FORMAT_R8_UINT:
	case DXGI_FORMAT_R8_SNORM:
	case DXGI_FORMAT_R8_SINT:
	//case XBOX_DXGI_FORMAT_R4G4_UNORM:
		return DXGI_FORMAT_R8_TYPELESS;

	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
		return DXGI_FORMAT_BC1_TYPELESS;

	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
		return DXGI_FORMAT_BC2_TYPELESS;

	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
		return DXGI_FORMAT_BC3_TYPELESS;

	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM:
		return DXGI_FORMAT_BC4_TYPELESS;

	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM:
		return DXGI_FORMAT_BC5_TYPELESS;

	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		return DXGI_FORMAT_B8G8R8A8_TYPELESS;

	case DXGI_FORMAT_B8G8R8X8_UNORM:
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		return DXGI_FORMAT_B8G8R8X8_TYPELESS;

	case DXGI_FORMAT_BC6H_UF16:
	case DXGI_FORMAT_BC6H_SF16:
		return DXGI_FORMAT_BC6H_TYPELESS;

	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		return DXGI_FORMAT_BC7_TYPELESS;

	default:
		return fmt;
	}
}
}
}