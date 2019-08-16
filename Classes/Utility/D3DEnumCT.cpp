#include "D3DEnumCT.h"

D3DBLEND D3DEnumCT::d3d11To9(D3D11_BLEND blend)
{
	switch (blend)
	{
	case D3D11_BLEND_ZERO: return D3DBLEND_ZERO;
	case D3D11_BLEND_ONE: return D3DBLEND_ONE;
	case D3D11_BLEND_SRC_COLOR: return D3DBLEND_SRCCOLOR;
	case D3D11_BLEND_INV_SRC_COLOR: return D3DBLEND_INVSRCCOLOR;
	case D3D11_BLEND_SRC_ALPHA: return D3DBLEND_SRCALPHA;
	case D3D11_BLEND_INV_SRC_ALPHA: return D3DBLEND_INVSRCALPHA;
	case D3D11_BLEND_DEST_ALPHA: return D3DBLEND_DESTALPHA;
	case D3D11_BLEND_INV_DEST_ALPHA: return D3DBLEND_INVDESTALPHA;
	case D3D11_BLEND_DEST_COLOR: return D3DBLEND_DESTCOLOR;
	case D3D11_BLEND_INV_DEST_COLOR: return D3DBLEND_INVDESTCOLOR;
	case D3D11_BLEND_SRC_ALPHA_SAT: return D3DBLEND_SRCALPHASAT;
	case D3D11_BLEND_BLEND_FACTOR: return D3DBLEND_BOTHSRCALPHA;
	case D3D11_BLEND_INV_BLEND_FACTOR: return D3DBLEND_BOTHINVSRCALPHA;
	case D3D11_BLEND_SRC1_COLOR:
	case D3D11_BLEND_INV_SRC1_COLOR:
	case D3D11_BLEND_SRC1_ALPHA:
	case D3D11_BLEND_INV_SRC1_ALPHA:
	default:
		return D3DBLEND_FORCE_DWORD;
	}
}

D3DCMPFUNC D3DEnumCT::d3d11To9(D3D11_COMPARISON_FUNC cmp)
{
	return static_cast<_D3DCMPFUNC>(cmp);
}

D3DFORMAT D3DEnumCT::d3d11To9(DXGI_FORMAT fmt)
{
	D3DFORMAT ret = D3DFMT_UNKNOWN;
	switch (fmt)
	{
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

D3DPRIMITIVETYPE D3DEnumCT::d3d11To9(D3D11_PRIMITIVE_TOPOLOGY topo)
{
	D3DPRIMITIVETYPE ret = D3DPT_FORCE_DWORD;
	switch (topo)
	{
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

static D3DDECLUSAGE __d3d11To9(const std::string& semantic) {
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

static D3DDECLTYPE __d3d11To9(DXGI_FORMAT fmt) {
	D3DDECLTYPE ret = D3DDECLTYPE_UNUSED;
	switch (fmt)
	{
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

D3DVERTEXELEMENT9 D3DEnumCT::d3d11To9(const D3D11_INPUT_ELEMENT_DESC& desc)
{
	D3DVERTEXELEMENT9 ret;
	ret.Stream = desc.InputSlot;
	ret.Offset = desc.AlignedByteOffset;     
	ret.Type = (D3DDECLTYPE)__d3d11To9(desc.Format);
	ret.Method = D3DDECLMETHOD_DEFAULT;
	ret.Usage = __d3d11To9(desc.SemanticName);
	ret.UsageIndex = desc.SemanticIndex;
	return ret;
}

D3DTEXTUREADDRESS D3DEnumCT::d3d11To9(D3D11_TEXTURE_ADDRESS_MODE addrMode)
{
	return static_cast<D3DTEXTUREADDRESS>(addrMode);
}

std::map<D3DSAMPLERSTATETYPE, D3DTEXTUREFILTERTYPE> D3DEnumCT::d3d11To9(D3D11_FILTER filter)
{
	std::map<D3DSAMPLERSTATETYPE, D3DTEXTUREFILTERTYPE> ret;
	switch (filter)
	{
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

DXGI_FORMAT D3DEnumCT::d3d9To11(D3DFORMAT fmt)
{
	DXGI_FORMAT ret = DXGI_FORMAT_UNKNOWN;
	switch (fmt)
	{
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

int D3DEnumCT::GetWidth(DXGI_FORMAT format)
{
	int width = 4;
	switch (format)
	{
	case DXGI_FORMAT_R32_UINT:
		width = 4;
		break;
	case DXGI_FORMAT_R32_SINT:
		width = 4;
		break;
	case DXGI_FORMAT_R16_UINT:
		width = 2;
		break;
	case DXGI_FORMAT_R16_SINT:
		width = 2;
		break;
	case DXGI_FORMAT_R8_UINT:
		width = 1;
		break;
	case DXGI_FORMAT_R8_SINT:
		width = 1;
		break;
	default:
		break;
	}
	return width;
}