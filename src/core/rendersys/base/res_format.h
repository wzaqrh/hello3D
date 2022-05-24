#pragma once
#include "core/base/stl.h"
#include "core/base/math.h"

namespace mir {

enum ResourceBaseFormat
{
	kRBF_Unkown,
	kRBF_RGBA,
	kRBF_RGB,
	kRBF_RG,
	kRBF_R,
	kRBF_A,
	kRBF_D,
	kRBF_BGRA,
	kRBF_BGRX,
	kRBF_DS,
	kRBF_Max
};

enum ResourceDataType 
{
	kRDT_Unkown,
	kRDT_Typeless,
	kRDT_Float,
	kRDT_UInt,
	kRDT_Int,
	kRDT_UNorm,
	kRDT_SNorm,
	kRDT_Max
};

enum ResourceFormat 
{
	kFormatUnknown = 0,

	kFormatR32G32B32A32Typeless = 1,
	kFormatR32G32B32A32Float = 2,
	kFormatR32G32B32A32UInt = 3,
	kFormatR32G32B32A32SInt = 4,

	kFormatR32G32B32Typeless = 5,
	kFormatR32G32B32Float = 6,
	kFormatR32G32B32UInt = 7,
	kFormatR32G32B32SInt = 8,

	kFormatR16G16B16A16Typeless = 9,
	kFormatR16G16B16A16Float = 10,
	kFormatR16G16B16A16UNorm = 11,
	kFormatR16G16B16A16UInt = 12,
	kFormatR16G16B16A16SNorm = 13,
	kFormatR16G16B16A16SInt = 14,

	kFormatR32G32Typeless = 15,
	kFormatR32G32Float = 16,
	kFormatR32G32UInt = 17,
	kFormatR32G32SInt = 18,

	kFormatR32G8X24Typeless = 19,
	kFormatD32FloatS8X24UInt = 20,
	kFormatR32FloatX8X24Typeless = 21,
	kFormatX32TypelessG8X24UInt = 22,

	kFormatR10G10B10A2Typeless = 23,
	kFormatR10G10B10A2UNorm = 24,
	kFormatR10G10B10A2UInt = 25,
	kFormatR11G11B10Float = 26,

	kFormatR8G8B8A8Typeless = 27,
	kFormatR8G8B8A8UNorm = 28,
	kFormatR8G8B8A8UNormSRgb = 29,
	kFormatR8G8B8A8UInt = 30,
	kFormatR8G8B8A8SNorm = 31,
	kFormatR8G8B8A8SInt = 32,

	kFormatR16G16Typeless = 33,
	kFormatR16G16Float = 34,
	kFormatR16G16UNorm = 35,
	kFormatR16G16UInt = 36,
	kFormatR16G16SNorm = 37,
	kFormatR16G16SInt = 38,

	kFormatR32Typeless = 39,
	kFormatD32Float = 40,
	kFormatR32Float = 41,
	kFormatR32UInt = 42,
	kFormatR32SInt = 43,

	kFormatR24G8Typeless = 44,
	kFormatD24UNormS8UInt = 45,
	kFormatR24UNormX8Typeless = 46,
	kFormatX24Typeless_G8UInt = 47,

	kFormatR8G8Typeless = 48,
	kFormatR8G8UNorm = 49,
	kFormatR8G8UInt = 50,
	kFormatR8G8SNorm = 51,
	kFormatR8G8SInt = 52,

	kFormatR16Typeless = 53,
	kFormatR16Float = 54,
	kFormatD16UNorm = 55,
	kFormatR16UNorm = 56,
	kFormatR16UInt = 57,
	kFormatR16SNorm = 58,
	kFormatR16SInt = 59,

	kFormatR8Typeless = 60,
	kFormatR8UNorm = 61,
	kFormatR8UInt = 62,
	kFormatR8SNorm = 63,
	kFormatR8SInt = 64,

	kFormatA8UNorm = 65,

	kFormatBC1Typeless = 70,
	kFormatBC1UNorm = 71,
	kFormatBC1UNormSRgb = 72,
	kFormatBC2Typeless = 73,
	kFormatBC2UNorm = 74,
	kFormatBC2UNormSRgb = 75,
	kFormatBC3Typeless = 76,
	kFormatBC3UNorm = 77,
	kFormatBC3UNormSRgb = 78,
	kFormatBC4Typeless = 79,
	kFormatBC4UNorm = 80,
	kFormatBC4SNorm = 81,
	kFormatBC5Typeless = 82,
	kFormatBC5UNorm = 83,
	kFormatBC5SNorm = 84,
	kFormatB5G6R5UNorm = 85,
	kFormatB5G5R5A1UNorm = 86,
	kFormatB8G8R8A8UNorm = 87,
	kFormatB8G8R8X8UNorm = 88,
	kFormatR10G10B10XRBiasA2UNorm = 89,
	kFormatB8G8R8A8Typeless = 90,
	kFormatB8G8R8A8UNormSRgb = 91,
	kFormatB8G8R8X8Typeless = 92,
	kFormatB8G8R8X8UNormSRgb = 93,
	kFormatBC6HTypeless = 94,
	kFormatBC6HUF16 = 95,
	kFormatBC6HSF16 = 96,
	kFormatBC7Typeless = 97,
	kFormatBC7UNorm = 98,
	kFormatBC7UNormSRgb = 99,
};
#define MakeResFormats(...) std::vector<ResourceFormat>{ ##__VA_ARGS__ }

ResourceFormat MakeResFormat(ResourceBaseFormat baseFormat, ResourceDataType dataType, int bitsPerChannel, bool isSRGB);

}