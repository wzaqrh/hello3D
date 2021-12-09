#pragma once
#define IL_STATIC_LIB
#include <IL/il.h>
#include <boost/assert.hpp>
#include "core/base/base_type.h"

namespace mir {

namespace il_helper {
static const ILenum CSupportILTypes[] = {
	IL_BMP, IL_PNG, IL_JPG, IL_DDS, IL_TGA, IL_HDR
};
static ILenum DetectType(FILE* fd) {
	for (int i = 0; i < sizeof(CSupportILTypes) / sizeof(CSupportILTypes[0]); ++i) {
		if (ilIsValidF(CSupportILTypes[i], fd))
			return CSupportILTypes[i];
	}
	return IL_TYPE_UNKNOWN;
}
static ILenum Convert3ChannelImageFormatTo4Channel(ILenum imageFormat) {
	switch (imageFormat) {
	case IL_RGB:
		imageFormat = IL_RGBA;
		break;
	case IL_BGR:
		imageFormat = IL_BGRA;
		break;
	default:
		break;
	}
	return imageFormat;
}
static ResourceFormat ConvertImageFormatTypeToResourceFormat(ILenum imageFormat, ILenum imageType) {
	ResourceFormat resFmt = kFormatUnknown;
	switch (imageFormat) {
	case IL_ALPHA: {
		switch (imageType) {
		case IL_UNSIGNED_BYTE:
			return kFormatA8UNorm;
		case IL_BYTE:
		case IL_SHORT:
		case IL_UNSIGNED_SHORT:
		case IL_INT:
		case IL_UNSIGNED_INT:
		case IL_FLOAT:
		case IL_DOUBLE:
		case IL_HALF:
		default:
			return kFormatUnknown;
		}
	}break;
	case IL_RGBA: {
		switch (imageType) {
		case IL_BYTE:
			return kFormatR8G8B8A8SNorm;
		case IL_UNSIGNED_BYTE:
			return kFormatR8G8B8A8UNorm;
		case IL_SHORT:
			return kFormatR16G16B16A16SNorm;
		case IL_UNSIGNED_SHORT:
			return kFormatR16G16B16A16UNorm;
		case IL_INT:
			return kFormatR32G32B32A32SInt;
		case IL_UNSIGNED_INT:
			return kFormatR32G32B32A32UInt;
		case IL_FLOAT:
			return kFormatR32G32B32A32Float;
		case IL_HALF:
			return kFormatR16G16B16A16Float;
		case IL_DOUBLE:
		default:
			return kFormatUnknown;
		}
	}break;
	case IL_RGB: {
		switch (imageType) {
		case IL_INT:
			return kFormatR32G32B32SInt;
		case IL_UNSIGNED_INT:
			return kFormatR32G32B32UInt;
		case IL_FLOAT:
			return kFormatR32G32B32Float;
		case IL_BYTE:
		case IL_UNSIGNED_BYTE:
		case IL_SHORT:
		case IL_UNSIGNED_SHORT:
		case IL_DOUBLE:
		case IL_HALF:
		default:
			return kFormatUnknown;
		}
	}break;
	case IL_BGRA: {
		switch (imageType) {
		case IL_UNSIGNED_BYTE:
			return kFormatB8G8R8A8UNorm;
		case IL_BYTE:
		case IL_SHORT:
		case IL_UNSIGNED_SHORT:
		case IL_INT:
		case IL_UNSIGNED_INT:
		case IL_FLOAT:
		case IL_DOUBLE:
		case IL_HALF:
		default:
			return kFormatUnknown;
		}
	}break;
	case IL_BGR: {
		switch (imageType) {
		case IL_UNSIGNED_BYTE:
			return kFormatB8G8R8X8UNorm;
		case IL_BYTE:
		case IL_SHORT:
		case IL_UNSIGNED_SHORT:
		case IL_INT:
		case IL_UNSIGNED_INT:
		case IL_FLOAT:
		case IL_DOUBLE:
		case IL_HALF:
		default:
			return kFormatUnknown;
		}
	}break;
	default:
		break;
	}
	return kFormatUnknown;
}
#define RETURN_PAIR(ImgFmt, TypeFmt) return std::make_tuple(ImgFmt, TypeFmt)
static std::tuple<ILenum, ILenum> ConvertResourceFormatToILImageFormatType(ResourceFormat format) {
	switch (format) {
	case kFormatR32G32B32A32Float: RETURN_PAIR(IL_RGBA, IL_FLOAT);
	case kFormatR32G32B32A32UInt: RETURN_PAIR(IL_RGBA, IL_UNSIGNED_INT);
	case kFormatR32G32B32A32SInt: RETURN_PAIR(IL_RGBA, IL_INT);

	case kFormatR32G32B32Float: RETURN_PAIR(IL_RGB, IL_FLOAT);
	case kFormatR32G32B32UInt: RETURN_PAIR(IL_RGB, IL_UNSIGNED_INT);
	case kFormatR32G32B32SInt: RETURN_PAIR(IL_RGB, IL_INT);

	case kFormatR16G16B16A16Float: RETURN_PAIR(IL_RGBA, IL_HALF);
	case kFormatR16G16B16A16UNorm: RETURN_PAIR(IL_RGBA, IL_UNSIGNED_SHORT);
	case kFormatR16G16B16A16UInt: RETURN_PAIR(IL_RGBA, IL_UNSIGNED_SHORT);
	case kFormatR16G16B16A16SNorm: RETURN_PAIR(IL_RGBA, IL_SHORT);
	case kFormatR16G16B16A16SInt: RETURN_PAIR(IL_RGBA, IL_SHORT);

	case kFormatR32G32Float: RETURN_PAIR(0, 0);
	case kFormatR32G32UInt: RETURN_PAIR(0, 0);
	case kFormatR32G32SInt: RETURN_PAIR(0, 0);

	case kFormatR8G8B8A8UNorm: RETURN_PAIR(IL_RGBA, IL_UNSIGNED_BYTE);
	case kFormatR8G8B8A8UNormSRgb: RETURN_PAIR(IL_RGBA, IL_UNSIGNED_BYTE);
	case kFormatR8G8B8A8UInt: RETURN_PAIR(IL_RGBA, IL_UNSIGNED_BYTE);
	case kFormatR8G8B8A8SNorm: RETURN_PAIR(IL_RGBA, IL_BYTE);
	case kFormatR8G8B8A8SInt: RETURN_PAIR(IL_RGBA, IL_BYTE);

	case kFormatR16G16Float: RETURN_PAIR(0, 0);
	case kFormatR16G16UNorm: RETURN_PAIR(0, 0);
	case kFormatR16G16UInt: RETURN_PAIR(0, 0);
	case kFormatR16G16SNorm: RETURN_PAIR(0, 0);
	case kFormatR16G16SInt: RETURN_PAIR(0, 0);

	case kFormatD32Float: RETURN_PAIR(0, 0);
	case kFormatR32Float: RETURN_PAIR(0, 0);
	case kFormatR32UInt: RETURN_PAIR(0, 0);
	case kFormatR32SInt: RETURN_PAIR(0, 0);

	case kFormatR8G8UNorm: RETURN_PAIR(0, 0);
	case kFormatR8G8UInt: RETURN_PAIR(0, 0);
	case kFormatR8G8SNorm: RETURN_PAIR(0, 0);
	case kFormatR8G8SInt: RETURN_PAIR(0, 0);

	case kFormatR16Float: RETURN_PAIR(0, 0);
	case kFormatD16UNorm: RETURN_PAIR(0, 0);
	case kFormatR16UNorm: RETURN_PAIR(0, 0);
	case kFormatR16UInt: RETURN_PAIR(0, 0);
	case kFormatR16SNorm: RETURN_PAIR(0, 0);
	case kFormatR16SInt: RETURN_PAIR(0, 0);

	case kFormatR8UNorm: RETURN_PAIR(0, 0);
	case kFormatR8UInt: RETURN_PAIR(0, 0);
	case kFormatR8SNorm: RETURN_PAIR(0, 0);
	case kFormatR8SInt: RETURN_PAIR(0, 0);

	case kFormatA8UNorm: RETURN_PAIR(IL_ALPHA, IL_UNSIGNED_BYTE);

	case kFormatB8G8R8A8UNorm: RETURN_PAIR(IL_BGRA, IL_UNSIGNED_BYTE);
	case kFormatB8G8R8X8UNorm: RETURN_PAIR(IL_BGR, IL_UNSIGNED_BYTE);
	case kFormatB8G8R8A8UNormSRgb: RETURN_PAIR(IL_BGRA, IL_UNSIGNED_BYTE);
	case kFormatB8G8R8X8UNormSRgb: RETURN_PAIR(IL_BGR, IL_UNSIGNED_BYTE);
	default:
		break;
	}
	RETURN_PAIR(0, 0);
}

static ResourceFormat ConvertILFormatToResourceFormat(ILenum compressILFormat) {
	switch (compressILFormat) {
	case IL_DXT1: return kFormatBC1UNorm;
	case IL_DXT2: return kFormatBC2UNorm;
	case IL_DXT3: return kFormatBC3UNorm;
	case IL_DXT4: return kFormatBC4UNorm;
	case IL_DXT5: return kFormatBC5UNorm;
	case IL_3DC: return kFormatBC4UNorm;
	case IL_ATI1N: return kFormatBC1UNorm;
	case IL_DXT1A: return kFormatBC1UNorm;
	default:
		break;
	}
	return kFormatUnknown;
}

static bool CheckLastError() {
	ILenum LastError = ilGetError();
	BOOST_ASSERT(IL_NO_ERROR == LastError);
	return (IL_NO_ERROR == LastError);
}
};

}