#include "core/rendersys/base/res_format.h"
#include <gli/gli.hpp>
#include <Windows.h>
#include <d3d11.h>

namespace mir {

struct ResourceFormatInfo 
{
	ResourceFormat Format;
	ResourceBaseFormat BaseFormat;
	ResourceDataType DataType;
	int BytePerChannel;
	bool IsSRGB;
};

class ResourceFormatInfoQuery
{
public:
	ResourceFormatInfoQuery() {
		//rgba 32
		AddEntry(kFormatR32G32B32A32Typeless, kRBF_RGBA, kRDT_Typeless, 32);
		AddEntry(kFormatR32G32B32A32Float, kRBF_RGBA, kRDT_Float, 32);
		AddEntry(kFormatR32G32B32A32UInt, kRBF_RGBA, kRDT_UInt, 32);
		AddEntry(kFormatR32G32B32A32SInt, kRBF_RGBA, kRDT_Int, 32);

		//rgba 16
		AddEntry(kFormatR16G16B16A16Typeless, kRBF_RGBA, kRDT_Typeless, 16);
		AddEntry(kFormatR16G16B16A16Float, kRBF_RGBA, kRDT_Float, 16);
		AddEntry(kFormatR16G16B16A16UNorm, kRBF_RGBA, kRDT_UNorm, 16);
		AddEntry(kFormatR16G16B16A16UInt, kRBF_RGBA, kRDT_UInt, 16);
		AddEntry(kFormatR16G16B16A16SNorm, kRBF_RGBA, kRDT_SNorm, 16);
		AddEntry(kFormatR16G16B16A16SInt, kRBF_RGBA, kRDT_Int, 16);

		//rgba 8
		AddEntry(kFormatR8G8B8A8Typeless, kRBF_RGBA, kRDT_Typeless, 8);
		AddEntry(kFormatR8G8B8A8UNorm, kRBF_RGBA, kRDT_UNorm, 8);
		AddEntry(kFormatR8G8B8A8UNormSRgb, kRBF_RGBA, kRDT_UNorm, 8, true);
		AddEntry(kFormatR8G8B8A8UInt, kRBF_RGBA, kRDT_UInt, 8);
		AddEntry(kFormatR8G8B8A8SNorm, kRBF_RGBA, kRDT_SNorm, 8);
		AddEntry(kFormatR8G8B8A8SInt, kRBF_RGBA, kRDT_Int, 8);

		///rgb 32
		AddEntry(kFormatR32G32B32Typeless, kRBF_RGB, kRDT_Typeless, 32);
		AddEntry(kFormatR32G32B32Float, kRBF_RGB, kRDT_Float, 32);
		AddEntry(kFormatR32G32B32UInt, kRBF_RGB, kRDT_UInt, 32);
		AddEntry(kFormatR32G32B32SInt, kRBF_RGB, kRDT_Int, 32);

		///rg 32
		AddEntry(kFormatR32G32Typeless, kRBF_RG, kRDT_Typeless, 32);
		AddEntry(kFormatR32G32Float, kRBF_RG, kRDT_Float, 32);
		AddEntry(kFormatR32G32UInt, kRBF_RG, kRDT_UInt, 32);
		AddEntry(kFormatR32G32SInt, kRBF_RG, kRDT_Int, 32);

		//rg 16
		AddEntry(kFormatR16G16Typeless, kRBF_RG, kRDT_Typeless, 16);
		AddEntry(kFormatR16G16Float, kRBF_RG, kRDT_Float, 16);
		AddEntry(kFormatR16G16UNorm, kRBF_RG, kRDT_UNorm, 16);
		AddEntry(kFormatR16G16UInt, kRBF_RG, kRDT_UInt, 16);
		AddEntry(kFormatR16G16SNorm, kRBF_RG, kRDT_SNorm, 16);
		AddEntry(kFormatR16G16SInt, kRBF_RG, kRDT_Int, 16);

		//rg 8
		AddEntry(kFormatR8G8Typeless, kRBF_RG, kRDT_Typeless, 8);
		AddEntry(kFormatR8G8UNorm, kRBF_RG, kRDT_UNorm, 8);
		AddEntry(kFormatR8G8UInt, kRBF_RG, kRDT_UInt, 8);
		AddEntry(kFormatR8G8SNorm, kRBF_RG, kRDT_SNorm, 8);
		AddEntry(kFormatR8G8SInt, kRBF_RG, kRDT_Int, 8);

		//AddEntry(kFormatR32G8X24Typeless);
		//AddEntry(kFormatD32FloatS8X24UInt);
		//AddEntry(kFormatR32FloatX8X24Typeless);
		//AddEntry(kFormatX32TypelessG8X24UInt);

		//AddEntry(kFormatR10G10B10A2Typeless);
		//AddEntry(kFormatR10G10B10A2UNorm);
		//AddEntry(kFormatR10G10B10A2UInt);
		//AddEntry(kFormatR11G11B10Float);

		///r 32
		AddEntry(kFormatR32Typeless, kRBF_R, kRDT_Typeless, 32);
		AddEntry(kFormatR32Float, kRBF_R, kRDT_Float, 32);
		AddEntry(kFormatR32UInt, kRBF_R, kRDT_UInt, 32);
		AddEntry(kFormatR32SInt, kRBF_R, kRDT_Int, 32);

		//r 16
		AddEntry(kFormatR16Typeless, kRBF_R, kRDT_Typeless, 16);
		AddEntry(kFormatR16Float, kRBF_R, kRDT_Float, 16);
		AddEntry(kFormatR16UNorm, kRBF_R, kRDT_UNorm, 16);
		AddEntry(kFormatR16UInt, kRBF_R, kRDT_UInt, 16);
		AddEntry(kFormatR16SNorm, kRBF_R, kRDT_SNorm, 16);
		AddEntry(kFormatR16SInt, kRBF_R, kRDT_Int, 16);

		//r 8
		AddEntry(kFormatR8Typeless, kRBF_R, kRDT_Typeless, 8);
		AddEntry(kFormatR8UNorm, kRBF_R, kRDT_UNorm, 8);
		AddEntry(kFormatR8UInt, kRBF_R, kRDT_UInt, 8);
		AddEntry(kFormatR8SNorm, kRBF_R, kRDT_SNorm, 8);
		AddEntry(kFormatR8SInt, kRBF_R, kRDT_Int, 8);

		/*AddEntry(kFormatR24G8Typeless);
		AddEntry(kFormatD24UNormS8UInt);
		AddEntry(kFormatR24UNormX8Typeless);
		AddEntry(kFormatX24Typeless_G8UInt);*/
		
		AddEntry(kFormatD32Float, kRBF_D, kRDT_Float, 32);
		AddEntry(kFormatD16UNorm, kRBF_R, kRDT_UNorm, 16);
		AddEntry(kFormatA8UNorm, kRBF_A, kRDT_UNorm, 8);

		/*AddEntry(kFormatBC1Typeless);
		AddEntry(kFormatBC1UNorm);
		AddEntry(kFormatBC1UNormSRgb);
		AddEntry(kFormatBC2Typeless);
		AddEntry(kFormatBC2UNorm);
		AddEntry(kFormatBC2UNormSRgb);
		AddEntry(kFormatBC3Typeless);
		AddEntry(kFormatBC3UNorm);
		AddEntry(kFormatBC3UNormSRgb);
		AddEntry(kFormatBC4Typeless);
		AddEntry(kFormatBC4UNorm);
		AddEntry(kFormatBC4SNorm);
		AddEntry(kFormatBC5Typeless);
		AddEntry(kFormatBC5UNorm);
		AddEntry(kFormatBC5SNorm);
		AddEntry(kFormatB5G6R5UNorm);
		AddEntry(kFormatB5G5R5A1UNorm);
		AddEntry(kFormatB8G8R8A8UNorm);
		AddEntry(kFormatB8G8R8X8UNorm);
		AddEntry(kFormatR10G10B10XRBiasA2UNorm);
		AddEntry(kFormatB8G8R8A8Typeless);
		AddEntry(kFormatB8G8R8A8UNormSRgb);
		AddEntry(kFormatB8G8R8X8Typeless);
		AddEntry(kFormatB8G8R8X8UNormSRgb);
		AddEntry(kFormatBC6HTypeless);
		AddEntry(kFormatBC6HUF16);
		AddEntry(kFormatBC6HSF16);
		AddEntry(kFormatBC7Typeless);
		AddEntry(kFormatBC7UNorm);
		AddEntry(kFormatBC7UNormSRgb);*/
	}

	ResourceFormat FindResFormat(ResourceBaseFormat baseFormat, ResourceDataType dataType, int bitsPerChannel, bool isSRGB) const {
		auto find_iter = std::find_if(mEntries.begin(), mEntries.end(), [&](const ResourceFormatInfo& i)->bool {
			return i.BaseFormat == baseFormat
				&& i.DataType == dataType
				&& i.BytePerChannel == bitsPerChannel
				&& i.IsSRGB == isSRGB;
		});
		return (find_iter != mEntries.end()) ? find_iter->Format : kFormatUnknown;
	}
private:
	void AddEntry(ResourceFormat fmt, ResourceBaseFormat baseFmt, ResourceDataType dataType, int bpc, bool isSRGB = false) {
		mEntries.emplace_back(ResourceFormatInfo{fmt, baseFmt, dataType, bpc, isSRGB});
	}
private:
	std::vector<ResourceFormatInfo> mEntries;
};
static ResourceFormatInfoQuery gResFmtInfoQuery;

ResourceFormat MakeResFormat(ResourceBaseFormat baseFormat, ResourceDataType dataType, int bitsPerChannel, bool isSRGB)
{
	return gResFmtInfoQuery.FindResFormat(baseFormat, dataType, bitsPerChannel, isSRGB);
}

size_t BitsPerPixel(ResourceFormat fmt)
{
	gli::dx::dxgi_format_dds dxgiFmt = (gli::dx::dxgi_format_dds)fmt;
	static gli::dx dx1;
	gli::format gliFmt = dx1.find(gli::dx::D3DFMT_DX10, gli::dx::dxgiFormat(dxgiFmt));

	auto block_size = gli::block_size(gliFmt);
	auto block_ext = gli::block_extent(gliFmt);

	BOOST_ASSERT(block_size * 8 % (block_ext.x * block_ext.y) == 0);
	return block_size * 8 / (block_ext.x * block_ext.y);
}

size_t BytePerPixel(ResourceFormat fmt)
{
	return BitsPerPixel(fmt) / 8;
}

bool IsDepthStencil(ResourceFormat fmt)
{
	switch (static_cast<int>(fmt)) {
	case kFormatR32G8X24Typeless://typeless
	case kFormatR32FloatX8X24Typeless://typeless
	case kFormatX32TypelessG8X24UInt://typeless
	case kFormatD32FloatS8X24UInt:
		return true;
	case kFormatD32Float:
	case kFormatD16UNorm:
		return true;
	case kFormatR24G8Typeless://typeless
	case kFormatR24UNormX8Typeless://typeless
	case kFormatX24Typeless_G8UInt://typeless
	case kFormatD24UNormS8UInt:
		return true;
	default:
		return false;
	}
}

DXGI_FORMAT MakeTypeless1(DXGI_FORMAT fmt)
{
	switch (static_cast<int>(fmt)) {
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
		return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	default:
		return fmt;
	}
}
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

ResourceFormat MakeTypeless1(ResourceFormat fmt) {
	return (ResourceFormat)MakeTypeless1((DXGI_FORMAT)fmt);
}
ResourceFormat MakeTypeless(ResourceFormat fmt) {
	return (ResourceFormat)MakeTypeless((DXGI_FORMAT)fmt);
}
}