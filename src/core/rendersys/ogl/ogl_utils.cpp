#include "core/rendersys/ogl/ogl_utils.h"
#include <d3d11.h>
#include <d3d9.h>
#include <map>
#include <boost/format.hpp>

namespace mir {
namespace ogl {

struct TextureFormat
{
	DXGI_FORMAT texFormat;
	DXGI_FORMAT srvFormat;
	DXGI_FORMAT rtvFormat;
	DXGI_FORMAT dsvFormat;
	DXGI_FORMAT renderFormat;
	//DXGI_FORMAT swizzleTexFormat;
	//DXGI_FORMAT swizzleSRVFormat;
	//DXGI_FORMAT swizzleRTVFormat;
	GLenum internalFormat;
	GLenum internalType;
	BOOL normalized;
	size_t channelCount;
};

typedef std::map<DXGI_FORMAT, TextureFormat> D3D11ES3FormatMap;
static void InsertD3D11FormatInfo(D3D11ES3FormatMap* map, GLenum internalFormat, DXGI_FORMAT texFormat, DXGI_FORMAT srvFormat, DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat)
{
	TextureFormat info;
	info.internalFormat = internalFormat;
	info.texFormat = texFormat;
	info.srvFormat = srvFormat;
	info.rtvFormat = rtvFormat;
	info.dsvFormat = dsvFormat;

	// Given a GL internal format, the renderFormat is the DSV format if it is depth- or stencil-renderable,
	// the RTV format if it is color-renderable, and the (nonrenderable) texture format otherwise.
	if (dsvFormat != DXGI_FORMAT_UNKNOWN)
	{
		info.renderFormat = dsvFormat;
	}
	else if (rtvFormat != DXGI_FORMAT_UNKNOWN)
	{
		info.renderFormat = rtvFormat;
	}
	else if (texFormat != DXGI_FORMAT_UNKNOWN)
	{
		info.renderFormat = texFormat;
	}
	else
	{
		info.renderFormat = DXGI_FORMAT_UNKNOWN;
	}
#if 0
	// Compute the swizzle formats
	const gl::InternalFormat& formatInfo = gl::GetInternalFormatInfo(internalFormat);
	if (internalFormat != GL_NONE && formatInfo.pixelBytes > 0)
	{
		if (formatInfo.componentCount != 4 || texFormat == DXGI_FORMAT_UNKNOWN ||
			srvFormat == DXGI_FORMAT_UNKNOWN || rtvFormat == DXGI_FORMAT_UNKNOWN)
		{
			// Get the maximum sized component
			unsigned int maxBits = 1;
			if (formatInfo.compressed)
			{
				unsigned int compressedBitsPerBlock = formatInfo.pixelBytes * 8;
				unsigned int blockSize = formatInfo.compressedBlockWidth * formatInfo.compressedBlockHeight;
				maxBits = __max(compressedBitsPerBlock / blockSize, maxBits);
			}
			else
			{
				maxBits = __max(maxBits, formatInfo.alphaBits);
				maxBits = __max(maxBits, formatInfo.redBits);
				maxBits = __max(maxBits, formatInfo.greenBits);
				maxBits = __max(maxBits, formatInfo.blueBits);
				maxBits = __max(maxBits, formatInfo.luminanceBits);
				maxBits = __max(maxBits, formatInfo.depthBits);
			}
			maxBits = roundUp(maxBits, 8U);
			static const SwizzleInfoMap swizzleMap = BuildSwizzleInfoMap();
			SwizzleInfoMap::const_iterator swizzleIter = swizzleMap.find(SwizzleSizeType(maxBits, formatInfo.componentType));
			ASSERT(swizzleIter != swizzleMap.end());
			const SwizzleFormatInfo& swizzleInfo = swizzleIter->second;
			info.swizzleTexFormat = swizzleInfo.mTexFormat;
			info.swizzleSRVFormat = swizzleInfo.mSRVFormat;
			info.swizzleRTVFormat = swizzleInfo.mRTVFormat;
		}
		else
		{
			// The original texture format is suitable for swizzle operations
			info.swizzleTexFormat = texFormat;
			info.swizzleSRVFormat = srvFormat;
			info.swizzleRTVFormat = rtvFormat;
		}
	}
	else
	{
		// Not possible to swizzle with this texture format since it is either unsized or GL_NONE
		info.swizzleTexFormat = DXGI_FORMAT_UNKNOWN;
		info.swizzleSRVFormat = DXGI_FORMAT_UNKNOWN;
		info.swizzleRTVFormat = DXGI_FORMAT_UNKNOWN;
	}

	// Check if there is an initialization function for this texture format
	static const InternalFormatInitializerMap initializerMap = BuildInternalFormatInitializerMap();
	InternalFormatInitializerMap::const_iterator initializerIter = initializerMap.find(internalFormat);
	info.dataInitializerFunction = (initializerIter != initializerMap.end()) ? initializerIter->second : NULL;
	// Gather all the load functions for this internal format
	static const D3D11LoadFunctionMap loadFunctions = BuildD3D11LoadFunctionMap();
	D3D11LoadFunctionMap::const_iterator loadFunctionIter = loadFunctions.find(internalFormat);
	if (loadFunctionIter != loadFunctions.end())
	{
		const std::vector<TypeLoadFunctionPair>& loadFunctionVector = loadFunctionIter->second;
		for (size_t i = 0; i < loadFunctionVector.size(); i++)
		{
			GLenum type = loadFunctionVector[i].first;
			LoadImageFunction function = loadFunctionVector[i].second;
			info.loadFunctions.insert(std::make_pair(type, function));
		}
	}
#endif
	map->insert(std::make_pair(texFormat, info));
}
static void BuildD3D11_FL9_3FormatOverrideMap(D3D11ES3FormatMap& map)
{
	// D3D11 Feature Level 9_3 doesn't support as many texture formats as Feature Level 10_0+.
	// In particular, it doesn't support:
	//      - *_TYPELESS formats
	//      - DXGI_FORMAT_D32_FLOAT_S8X24_UINT or DXGI_FORMAT_D32_FLOAT
	//                         | GL internal format   | D3D11 texture format            | D3D11 SRV format     | D3D11 RTV format      | D3D11 DSV format
	InsertD3D11FormatInfo(&map, GL_DEPTH_COMPONENT16, DXGI_FORMAT_D16_UNORM, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_D16_UNORM);
	InsertD3D11FormatInfo(&map, GL_DEPTH_COMPONENT24, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_D24_UNORM_S8_UINT);
	InsertD3D11FormatInfo(&map, GL_DEPTH_COMPONENT32F, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_DEPTH24_STENCIL8, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_D24_UNORM_S8_UINT);
	InsertD3D11FormatInfo(&map, GL_DEPTH32F_STENCIL8, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_STENCIL_INDEX8, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_D24_UNORM_S8_UINT);
}
static void BuildD3D11FormatMap(D3D11ES3FormatMap& map)
{
	//                         | GL internal format  | D3D11 texture format            | D3D11 SRV format               | D3D11 RTV format               | D3D11 DSV format   |
	InsertD3D11FormatInfo(&map, GL_NONE, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_R8, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_R8_SNORM, DXGI_FORMAT_R8_SNORM, DXGI_FORMAT_R8_SNORM, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RG8, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RG8_SNORM, DXGI_FORMAT_R8G8_SNORM, DXGI_FORMAT_R8G8_SNORM, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RGB8, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RGB8_SNORM, DXGI_FORMAT_R8G8B8A8_SNORM, DXGI_FORMAT_R8G8B8A8_SNORM, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RGB565, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RGBA4, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RGB5_A1, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RGBA8, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RGBA8_SNORM, DXGI_FORMAT_R8G8B8A8_SNORM, DXGI_FORMAT_R8G8B8A8_SNORM, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RGB10_A2, DXGI_FORMAT_R10G10B10A2_UNORM, DXGI_FORMAT_R10G10B10A2_UNORM, DXGI_FORMAT_R10G10B10A2_UNORM, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RGB10_A2UI, DXGI_FORMAT_R10G10B10A2_UINT, DXGI_FORMAT_R10G10B10A2_UINT, DXGI_FORMAT_R10G10B10A2_UINT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_SRGB8, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_SRGB8_ALPHA8, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_R16F, DXGI_FORMAT_R16_FLOAT, DXGI_FORMAT_R16_FLOAT, DXGI_FORMAT_R16_FLOAT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RG16F, DXGI_FORMAT_R16G16_FLOAT, DXGI_FORMAT_R16G16_FLOAT, DXGI_FORMAT_R16G16_FLOAT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RGB16F, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RGBA16F, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_R32F, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RG32F, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RGB32F, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RGBA32F, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_R11F_G11F_B10F, DXGI_FORMAT_R11G11B10_FLOAT, DXGI_FORMAT_R11G11B10_FLOAT, DXGI_FORMAT_R11G11B10_FLOAT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RGB9_E5, DXGI_FORMAT_R9G9B9E5_SHAREDEXP, DXGI_FORMAT_R9G9B9E5_SHAREDEXP, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_R8I, DXGI_FORMAT_R8_SINT, DXGI_FORMAT_R8_SINT, DXGI_FORMAT_R8_SINT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_R8UI, DXGI_FORMAT_R8_UINT, DXGI_FORMAT_R8_UINT, DXGI_FORMAT_R8_UINT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_R16I, DXGI_FORMAT_R16_SINT, DXGI_FORMAT_R16_SINT, DXGI_FORMAT_R16_SINT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_R16UI, DXGI_FORMAT_R16_UINT, DXGI_FORMAT_R16_UINT, DXGI_FORMAT_R16_UINT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_R32I, DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32_SINT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_R32UI, DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32_UINT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RG8I, DXGI_FORMAT_R8G8_SINT, DXGI_FORMAT_R8G8_SINT, DXGI_FORMAT_R8G8_SINT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RG8UI, DXGI_FORMAT_R8G8_UINT, DXGI_FORMAT_R8G8_UINT, DXGI_FORMAT_R8G8_UINT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RG16I, DXGI_FORMAT_R16G16_SINT, DXGI_FORMAT_R16G16_SINT, DXGI_FORMAT_R16G16_SINT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RG16UI, DXGI_FORMAT_R16G16_UINT, DXGI_FORMAT_R16G16_UINT, DXGI_FORMAT_R16G16_UINT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RG32I, DXGI_FORMAT_R32G32_SINT, DXGI_FORMAT_R32G32_SINT, DXGI_FORMAT_R32G32_SINT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RG32UI, DXGI_FORMAT_R32G32_UINT, DXGI_FORMAT_R32G32_UINT, DXGI_FORMAT_R32G32_UINT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RGB8I, DXGI_FORMAT_R8G8B8A8_SINT, DXGI_FORMAT_R8G8B8A8_SINT, DXGI_FORMAT_R8G8B8A8_SINT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RGB8UI, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RGB16I, DXGI_FORMAT_R16G16B16A16_SINT, DXGI_FORMAT_R16G16B16A16_SINT, DXGI_FORMAT_R16G16B16A16_SINT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RGB16UI, DXGI_FORMAT_R16G16B16A16_UINT, DXGI_FORMAT_R16G16B16A16_UINT, DXGI_FORMAT_R16G16B16A16_UINT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RGB32I, DXGI_FORMAT_R32G32B32A32_SINT, DXGI_FORMAT_R32G32B32A32_SINT, DXGI_FORMAT_R32G32B32A32_SINT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RGB32UI, DXGI_FORMAT_R32G32B32A32_UINT, DXGI_FORMAT_R32G32B32A32_UINT, DXGI_FORMAT_R32G32B32A32_UINT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RGBA8I, DXGI_FORMAT_R8G8B8A8_SINT, DXGI_FORMAT_R8G8B8A8_SINT, DXGI_FORMAT_R8G8B8A8_SINT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RGBA8UI, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RGBA16I, DXGI_FORMAT_R16G16B16A16_SINT, DXGI_FORMAT_R16G16B16A16_SINT, DXGI_FORMAT_R16G16B16A16_SINT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RGBA16UI, DXGI_FORMAT_R16G16B16A16_UINT, DXGI_FORMAT_R16G16B16A16_UINT, DXGI_FORMAT_R16G16B16A16_UINT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RGBA32I, DXGI_FORMAT_R32G32B32A32_SINT, DXGI_FORMAT_R32G32B32A32_SINT, DXGI_FORMAT_R32G32B32A32_SINT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RGBA32UI, DXGI_FORMAT_R32G32B32A32_UINT, DXGI_FORMAT_R32G32B32A32_UINT, DXGI_FORMAT_R32G32B32A32_UINT, DXGI_FORMAT_UNKNOWN);
	// Unsized formats, TODO: Are types of float and half float allowed for the unsized types? Would it change the DXGI format?
	InsertD3D11FormatInfo(&map, GL_ALPHA, DXGI_FORMAT_A8_UNORM, DXGI_FORMAT_A8_UNORM, DXGI_FORMAT_A8_UNORM, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_LUMINANCE, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_LUMINANCE_ALPHA, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RGB, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_RGBA, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_BGRA_EXT, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_UNKNOWN);
	// From GL_EXT_texture_storage
	//                           | GL internal format     | D3D11 texture format          | D3D11 SRV format                    | D3D11 RTV format              | D3D11 DSV format               |
	InsertD3D11FormatInfo(&map, GL_ALPHA8_EXT, DXGI_FORMAT_A8_UNORM, DXGI_FORMAT_A8_UNORM, DXGI_FORMAT_A8_UNORM, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_LUMINANCE8_EXT, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_ALPHA32F_EXT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_LUMINANCE32F_EXT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_ALPHA16F_EXT, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_LUMINANCE16F_EXT, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_LUMINANCE8_ALPHA8_EXT, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_LUMINANCE_ALPHA32F_EXT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_LUMINANCE_ALPHA16F_EXT, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_BGRA8_EXT, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_UNKNOWN);
#if 0
	InsertD3D11FormatInfo(&map, GL_BGRA4_ANGLEX, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_BGR5_A1_ANGLEX, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_UNKNOWN);
#endif
	// Depth stencil formats
	InsertD3D11FormatInfo(&map, GL_DEPTH_COMPONENT16, DXGI_FORMAT_R16_TYPELESS, DXGI_FORMAT_R16_UNORM, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_D16_UNORM);
	InsertD3D11FormatInfo(&map, GL_DEPTH_COMPONENT24, DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_R24_UNORM_X8_TYPELESS, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_D24_UNORM_S8_UINT);
	InsertD3D11FormatInfo(&map, GL_DEPTH_COMPONENT32F, DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_D32_FLOAT);
	InsertD3D11FormatInfo(&map, GL_DEPTH24_STENCIL8, DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_R24_UNORM_X8_TYPELESS, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_D24_UNORM_S8_UINT);
	InsertD3D11FormatInfo(&map, GL_DEPTH32F_STENCIL8, DXGI_FORMAT_R32G8X24_TYPELESS, DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_D32_FLOAT_S8X24_UINT);
	InsertD3D11FormatInfo(&map, GL_STENCIL_INDEX8, DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_X24_TYPELESS_G8_UINT, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_D24_UNORM_S8_UINT);
#if 0
	// From GL_ANGLE_depth_texture
	// Since D3D11 doesn't have a D32_UNORM format, use D24S8 which has comparable precision and matches the ES3 format.
	InsertD3D11FormatInfo(&map, GL_DEPTH_COMPONENT32_OES, DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_R24_UNORM_X8_TYPELESS, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_D24_UNORM_S8_UINT);
#endif
	// Compressed formats, From ES 3.0.1 spec, table 3.16
	//                           | GL internal format                        | D3D11 texture format | D3D11 SRV format     | D3D11 RTV format   | D3D11 DSV format  |
	InsertD3D11FormatInfo(&map, GL_COMPRESSED_R11_EAC, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_COMPRESSED_SIGNED_R11_EAC, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_COMPRESSED_RG11_EAC, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_COMPRESSED_SIGNED_RG11_EAC, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_COMPRESSED_RGB8_ETC2, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_COMPRESSED_SRGB8_ETC2, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_COMPRESSED_RGBA8_ETC2_EAC, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN);
	// From GL_EXT_texture_compression_dxt1
	InsertD3D11FormatInfo(&map, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN);
	InsertD3D11FormatInfo(&map, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN);
	// From GL_ANGLE_texture_compression_dxt3
	InsertD3D11FormatInfo(&map, GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN);
	// From GL_ANGLE_texture_compression_dxt5
	InsertD3D11FormatInfo(&map, GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN);
}

static void AddVertexFormatInfo(D3D11ES3FormatMap* map, GLenum type, BOOL normalized, size_t channelCount, DXGI_FORMAT texFormat)
{
	auto& elem = (*map)[texFormat];
	elem.internalType = type;
	elem.normalized = normalized;
	elem.channelCount = channelCount;
}
static void BuildD3D11_FL9_3VertexFormatInfoOverrideMap(D3D11ES3FormatMap& map)
{
	// D3D11 Feature Level 9_3 doesn't support as many formats for vertex buffer resource as Feature Level 10_0+.
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ff471324(v=vs.85).aspx
	// GL_BYTE -- unnormalized
	AddVertexFormatInfo(&map, GL_BYTE, GL_FALSE, 2, DXGI_FORMAT_R16G16_SINT);
	AddVertexFormatInfo(&map, GL_BYTE, GL_FALSE, 2, DXGI_FORMAT_R16G16_SINT);
	AddVertexFormatInfo(&map, GL_BYTE, GL_FALSE, 4, DXGI_FORMAT_R16G16B16A16_SINT);
	AddVertexFormatInfo(&map, GL_BYTE, GL_FALSE, 4, DXGI_FORMAT_R16G16B16A16_SINT);
	// GL_BYTE -- normalized
	AddVertexFormatInfo(&map, GL_BYTE, GL_TRUE, 2, DXGI_FORMAT_R16G16_SNORM);
	AddVertexFormatInfo(&map, GL_BYTE, GL_TRUE, 2, DXGI_FORMAT_R16G16_SNORM);
	AddVertexFormatInfo(&map, GL_BYTE, GL_TRUE, 4, DXGI_FORMAT_R16G16B16A16_SNORM);
	AddVertexFormatInfo(&map, GL_BYTE, GL_TRUE, 4, DXGI_FORMAT_R16G16B16A16_SNORM);
	// GL_UNSIGNED_BYTE -- unnormalized
	AddVertexFormatInfo(&map, GL_UNSIGNED_BYTE, GL_FALSE, 4, DXGI_FORMAT_R8G8B8A8_UINT);
	AddVertexFormatInfo(&map, GL_UNSIGNED_BYTE, GL_FALSE, 4, DXGI_FORMAT_R8G8B8A8_UINT);
	// NOTE: 3 and 4 component unnormalized GL_UNSIGNED_BYTE should use the default format table.
	// GL_UNSIGNED_BYTE -- normalized
	AddVertexFormatInfo(&map, GL_UNSIGNED_BYTE, GL_TRUE, 4, DXGI_FORMAT_R8G8B8A8_UNORM);
	AddVertexFormatInfo(&map, GL_UNSIGNED_BYTE, GL_TRUE, 4, DXGI_FORMAT_R8G8B8A8_UNORM);
	// NOTE: 3 and 4 component normalized GL_UNSIGNED_BYTE should use the default format table.
	// GL_SHORT -- unnormalized
	AddVertexFormatInfo(&map, GL_SHORT, GL_FALSE, 2, DXGI_FORMAT_R16G16_SINT);
	// NOTE: 2, 3 and 4 component unnormalized GL_SHORT should use the default format table.
	// GL_SHORT -- normalized
	AddVertexFormatInfo(&map, GL_SHORT, GL_TRUE, 2, DXGI_FORMAT_R16G16_SNORM);
	// NOTE: 2, 3 and 4 component normalized GL_SHORT should use the default format table.
	// GL_UNSIGNED_SHORT -- unnormalized
	AddVertexFormatInfo(&map, GL_UNSIGNED_SHORT, GL_FALSE, 2, DXGI_FORMAT_R32G32_FLOAT);
	AddVertexFormatInfo(&map, GL_UNSIGNED_SHORT, GL_FALSE, 2, DXGI_FORMAT_R32G32_FLOAT);
	AddVertexFormatInfo(&map, GL_UNSIGNED_SHORT, GL_FALSE, 3, DXGI_FORMAT_R32G32B32_FLOAT);
	AddVertexFormatInfo(&map, GL_UNSIGNED_SHORT, GL_FALSE, 4, DXGI_FORMAT_R32G32B32A32_FLOAT);
	// GL_UNSIGNED_SHORT -- normalized
	AddVertexFormatInfo(&map, GL_UNSIGNED_SHORT, GL_TRUE, 2, DXGI_FORMAT_R32G32_FLOAT);
	AddVertexFormatInfo(&map, GL_UNSIGNED_SHORT, GL_TRUE, 2, DXGI_FORMAT_R32G32_FLOAT);
	AddVertexFormatInfo(&map, GL_UNSIGNED_SHORT, GL_TRUE, 3, DXGI_FORMAT_R32G32B32_FLOAT);
	AddVertexFormatInfo(&map, GL_UNSIGNED_SHORT, GL_TRUE, 4, DXGI_FORMAT_R32G32B32A32_FLOAT);
	// GL_FIXED
	// TODO: Add test to verify that this works correctly.
	AddVertexFormatInfo(&map, GL_FIXED, GL_FALSE, 2, DXGI_FORMAT_R32G32_FLOAT);
	// NOTE: 2, 3 and 4 component GL_FIXED should use the default format table.
	// GL_FLOAT
	// TODO: Add test to verify that this works correctly.
	AddVertexFormatInfo(&map, GL_FLOAT, GL_FALSE, 2, DXGI_FORMAT_R32G32_FLOAT);
	// NOTE: 2, 3 and 4 component GL_FLOAT should use the default format table.
}
static void BuildD3D11VertexFormatInfoMap(D3D11ES3FormatMap& map)
{
	// TODO: column legend
	//
	// Float formats
	//
	// GL_BYTE -- un-normalized
	AddVertexFormatInfo(&map, GL_BYTE, GL_FALSE, 1, DXGI_FORMAT_R8_SINT);
	AddVertexFormatInfo(&map, GL_BYTE, GL_FALSE, 2, DXGI_FORMAT_R8G8_SINT);
	AddVertexFormatInfo(&map, GL_BYTE, GL_FALSE, 4, DXGI_FORMAT_R8G8B8A8_SINT);
	AddVertexFormatInfo(&map, GL_BYTE, GL_FALSE, 4, DXGI_FORMAT_R8G8B8A8_SINT);
	// GL_BYTE -- normalized
	AddVertexFormatInfo(&map, GL_BYTE, GL_TRUE, 1, DXGI_FORMAT_R8_SNORM);
	AddVertexFormatInfo(&map, GL_BYTE, GL_TRUE, 2, DXGI_FORMAT_R8G8_SNORM);
	AddVertexFormatInfo(&map, GL_BYTE, GL_TRUE, 4, DXGI_FORMAT_R8G8B8A8_SNORM);
	AddVertexFormatInfo(&map, GL_BYTE, GL_TRUE, 4, DXGI_FORMAT_R8G8B8A8_SNORM);
	// GL_UNSIGNED_BYTE -- un-normalized
	AddVertexFormatInfo(&map, GL_UNSIGNED_BYTE, GL_FALSE, 1, DXGI_FORMAT_R8_UINT);
	AddVertexFormatInfo(&map, GL_UNSIGNED_BYTE, GL_FALSE, 2, DXGI_FORMAT_R8G8_UINT);
	AddVertexFormatInfo(&map, GL_UNSIGNED_BYTE, GL_FALSE, 4, DXGI_FORMAT_R8G8B8A8_UINT);
	AddVertexFormatInfo(&map, GL_UNSIGNED_BYTE, GL_FALSE, 4, DXGI_FORMAT_R8G8B8A8_UINT);
	// GL_UNSIGNED_BYTE -- normalized
	AddVertexFormatInfo(&map, GL_UNSIGNED_BYTE, GL_TRUE, 1, DXGI_FORMAT_R8_UNORM);
	AddVertexFormatInfo(&map, GL_UNSIGNED_BYTE, GL_TRUE, 2, DXGI_FORMAT_R8G8_UNORM);
	AddVertexFormatInfo(&map, GL_UNSIGNED_BYTE, GL_TRUE, 4, DXGI_FORMAT_R8G8B8A8_UNORM);
	AddVertexFormatInfo(&map, GL_UNSIGNED_BYTE, GL_TRUE, 4, DXGI_FORMAT_R8G8B8A8_UNORM);
	// GL_SHORT -- un-normalized
	AddVertexFormatInfo(&map, GL_SHORT, GL_FALSE, 1, DXGI_FORMAT_R16_SINT);
	AddVertexFormatInfo(&map, GL_SHORT, GL_FALSE, 2, DXGI_FORMAT_R16G16_SINT);
	AddVertexFormatInfo(&map, GL_SHORT, GL_FALSE, 4, DXGI_FORMAT_R16G16B16A16_SINT);
	AddVertexFormatInfo(&map, GL_SHORT, GL_FALSE, 4, DXGI_FORMAT_R16G16B16A16_SINT);
	// GL_SHORT -- normalized
	AddVertexFormatInfo(&map, GL_SHORT, GL_TRUE, 1, DXGI_FORMAT_R16_SNORM);
	AddVertexFormatInfo(&map, GL_SHORT, GL_TRUE, 2, DXGI_FORMAT_R16G16_SNORM);
	AddVertexFormatInfo(&map, GL_SHORT, GL_TRUE, 4, DXGI_FORMAT_R16G16B16A16_SNORM);
	AddVertexFormatInfo(&map, GL_SHORT, GL_TRUE, 4, DXGI_FORMAT_R16G16B16A16_SNORM);
	// GL_UNSIGNED_SHORT -- un-normalized
	AddVertexFormatInfo(&map, GL_UNSIGNED_SHORT, GL_FALSE, 1, DXGI_FORMAT_R16_UINT);
	AddVertexFormatInfo(&map, GL_UNSIGNED_SHORT, GL_FALSE, 2, DXGI_FORMAT_R16G16_UINT);
	AddVertexFormatInfo(&map, GL_UNSIGNED_SHORT, GL_FALSE, 4, DXGI_FORMAT_R16G16B16A16_UINT);
	AddVertexFormatInfo(&map, GL_UNSIGNED_SHORT, GL_FALSE, 4, DXGI_FORMAT_R16G16B16A16_UINT);
	// GL_UNSIGNED_SHORT -- normalized
	AddVertexFormatInfo(&map, GL_UNSIGNED_SHORT, GL_TRUE, 1, DXGI_FORMAT_R16_UNORM);
	AddVertexFormatInfo(&map, GL_UNSIGNED_SHORT, GL_TRUE, 2, DXGI_FORMAT_R16G16_UNORM);
	AddVertexFormatInfo(&map, GL_UNSIGNED_SHORT, GL_TRUE, 4, DXGI_FORMAT_R16G16B16A16_UNORM);
	AddVertexFormatInfo(&map, GL_UNSIGNED_SHORT, GL_TRUE, 4, DXGI_FORMAT_R16G16B16A16_UNORM);
	// GL_INT -- un-normalized
	AddVertexFormatInfo(&map, GL_INT, GL_FALSE, 1, DXGI_FORMAT_R32_SINT);
	AddVertexFormatInfo(&map, GL_INT, GL_FALSE, 2, DXGI_FORMAT_R32G32_SINT);
	AddVertexFormatInfo(&map, GL_INT, GL_FALSE, 3, DXGI_FORMAT_R32G32B32_SINT);
	AddVertexFormatInfo(&map, GL_INT, GL_FALSE, 4, DXGI_FORMAT_R32G32B32A32_SINT);
	// GL_INT -- normalized
	AddVertexFormatInfo(&map, GL_INT, GL_TRUE, 1, DXGI_FORMAT_R32_FLOAT);
	AddVertexFormatInfo(&map, GL_INT, GL_TRUE, 2, DXGI_FORMAT_R32G32_FLOAT);
	AddVertexFormatInfo(&map, GL_INT, GL_TRUE, 3, DXGI_FORMAT_R32G32B32_FLOAT);
	AddVertexFormatInfo(&map, GL_INT, GL_TRUE, 4, DXGI_FORMAT_R32G32B32A32_FLOAT);
	// GL_UNSIGNED_INT -- un-normalized
	AddVertexFormatInfo(&map, GL_UNSIGNED_INT, GL_FALSE, 1, DXGI_FORMAT_R32_UINT);
	AddVertexFormatInfo(&map, GL_UNSIGNED_INT, GL_FALSE, 2, DXGI_FORMAT_R32G32_UINT);
	AddVertexFormatInfo(&map, GL_UNSIGNED_INT, GL_FALSE, 3, DXGI_FORMAT_R32G32B32_UINT);
	AddVertexFormatInfo(&map, GL_UNSIGNED_INT, GL_FALSE, 4, DXGI_FORMAT_R32G32B32A32_UINT);
	// GL_UNSIGNED_INT -- normalized
	AddVertexFormatInfo(&map, GL_UNSIGNED_INT, GL_TRUE, 1, DXGI_FORMAT_R32_FLOAT);
	AddVertexFormatInfo(&map, GL_UNSIGNED_INT, GL_TRUE, 2, DXGI_FORMAT_R32G32_FLOAT);
	AddVertexFormatInfo(&map, GL_UNSIGNED_INT, GL_TRUE, 3, DXGI_FORMAT_R32G32B32_FLOAT);
	AddVertexFormatInfo(&map, GL_UNSIGNED_INT, GL_TRUE, 4, DXGI_FORMAT_R32G32B32A32_FLOAT);
	// GL_FIXED
	AddVertexFormatInfo(&map, GL_FIXED, GL_FALSE, 1, DXGI_FORMAT_R32_FLOAT);
	AddVertexFormatInfo(&map, GL_FIXED, GL_FALSE, 2, DXGI_FORMAT_R32G32_FLOAT);
	AddVertexFormatInfo(&map, GL_FIXED, GL_FALSE, 3, DXGI_FORMAT_R32G32B32_FLOAT);
	AddVertexFormatInfo(&map, GL_FIXED, GL_FALSE, 4, DXGI_FORMAT_R32G32B32A32_FLOAT);
	// GL_HALF_FLOAT
	AddVertexFormatInfo(&map, GL_HALF_FLOAT, GL_FALSE, 1, DXGI_FORMAT_R16_FLOAT);
	AddVertexFormatInfo(&map, GL_HALF_FLOAT, GL_FALSE, 2, DXGI_FORMAT_R16G16_FLOAT);
	AddVertexFormatInfo(&map, GL_HALF_FLOAT, GL_FALSE, 3, DXGI_FORMAT_R16G16B16A16_FLOAT);
	AddVertexFormatInfo(&map, GL_HALF_FLOAT, GL_FALSE, 4, DXGI_FORMAT_R16G16B16A16_FLOAT);
	// GL_FLOAT
	AddVertexFormatInfo(&map, GL_FLOAT, GL_FALSE, 1, DXGI_FORMAT_R32_FLOAT);
	AddVertexFormatInfo(&map, GL_FLOAT, GL_FALSE, 2, DXGI_FORMAT_R32G32_FLOAT);
	AddVertexFormatInfo(&map, GL_FLOAT, GL_FALSE, 3, DXGI_FORMAT_R32G32B32_FLOAT);
	AddVertexFormatInfo(&map, GL_FLOAT, GL_FALSE, 4, DXGI_FORMAT_R32G32B32A32_FLOAT);
	// GL_INT_2_10_10_10_REV
	AddVertexFormatInfo(&map, GL_INT_2_10_10_10_REV, GL_FALSE, 4, DXGI_FORMAT_R32G32B32A32_FLOAT);
	AddVertexFormatInfo(&map, GL_INT_2_10_10_10_REV, GL_TRUE, 4, DXGI_FORMAT_R32G32B32A32_FLOAT);
	// GL_UNSIGNED_INT_2_10_10_10_REV
	AddVertexFormatInfo(&map, GL_UNSIGNED_INT_2_10_10_10_REV, GL_FALSE, 4, DXGI_FORMAT_R32G32B32A32_FLOAT);
	AddVertexFormatInfo(&map, GL_UNSIGNED_INT_2_10_10_10_REV, GL_TRUE, 4, DXGI_FORMAT_R10G10B10A2_UNORM);

	AddVertexFormatInfo(&map, GL_UNSIGNED_INT_2_10_10_10_REV, GL_FALSE, 4, DXGI_FORMAT_R10G10B10A2_UINT);
}

struct GetResourceFormatInfo {
	D3D11ES3FormatMap mFmtMap;
	GetResourceFormatInfo() {
		BuildD3D11_FL9_3FormatOverrideMap(mFmtMap);
		BuildD3D11FormatMap(mFmtMap);
		BuildD3D11_FL9_3VertexFormatInfoOverrideMap(mFmtMap);
		BuildD3D11VertexFormatInfoMap(mFmtMap);
	}
	TextureFormat& operator()(ResourceFormat fmt) {
		return mFmtMap[(DXGI_FORMAT)fmt];
	}
};
static GetResourceFormatInfo GetResFmtInfoFunc;

GLenum GetGLFormat(ResourceFormat fmt) {
	return GetResFmtInfoFunc(fmt).internalFormat;
}
GLenum GetGLType(ResourceFormat fmt) {
	return GetResFmtInfoFunc(fmt).internalType;
}
bool IsNormalized(ResourceFormat fmt) {
	return GetResFmtInfoFunc(fmt).normalized;
}

size_t GetChannelCount(ResourceFormat fmt)
{
	return GetResFmtInfoFunc(fmt).channelCount;
}

std::tuple<GLenum, GLenum, GLenum> GetGLSamplerFilterMode(SamplerFilterMode sfmode)
{
	GLenum minFilter = GL_NEAREST, magFilter = GL_NEAREST, compMode = GL_NONE;

	switch (sfmode)
	{
	case kSamplerFilterMinMagMipPoint:
		minFilter = GL_NEAREST_MIPMAP_NEAREST;
		magFilter = GL_NEAREST;
		break;
	case kSamplerFilterMinMagPointMipLinear:
		minFilter = GL_NEAREST_MIPMAP_LINEAR;
		magFilter = GL_NEAREST;
		break;
	case kSamplerFilterMinPointMagLinearMipPoint:
		minFilter = GL_NEAREST_MIPMAP_NEAREST;
		magFilter = GL_LINEAR;
		break;
	case kSamplerFilterMinPointMagMipLinear:
		minFilter = GL_NEAREST_MIPMAP_LINEAR;
		magFilter = GL_LINEAR;
		break;
	case kSamplerFilterMinLinearMagMipPoint:
		minFilter = GL_LINEAR_MIPMAP_NEAREST;
		magFilter = GL_NEAREST;
		break;
	case kSamplerFilterMinLinearMagPointMipLinear:
		minFilter = GL_LINEAR_MIPMAP_LINEAR;
		magFilter = GL_NEAREST;
		break;
	case kSamplerFilterMinMagLinearMipPoint:
		minFilter = GL_LINEAR_MIPMAP_NEAREST;
		magFilter = GL_LINEAR;
		break;
	case kSamplerFilterMinMagMipLinear:
		minFilter = GL_LINEAR_MIPMAP_LINEAR;
		magFilter = GL_LINEAR;
		break;
	case kSamplerFilterAnisotropic:
		minFilter = GL_LINEAR_MIPMAP_LINEAR;
		magFilter = GL_LINEAR;
		break;
	case kSamplerFilterCmpMinMagMipPoint:
		minFilter = GL_NEAREST_MIPMAP_NEAREST;
		magFilter = GL_NEAREST;
		compMode = GL_COMPARE_R_TO_TEXTURE;
		break;
	case kSamplerFilterCmpMinMagPointMipLinear:
		minFilter = GL_NEAREST_MIPMAP_LINEAR;
		magFilter = GL_NEAREST;
		compMode = GL_COMPARE_R_TO_TEXTURE;
		break;
	case kSamplerFilterCmpMinPointMagLinearMipPoint:
		minFilter = GL_NEAREST_MIPMAP_NEAREST;
		magFilter = GL_LINEAR;
		compMode = GL_COMPARE_R_TO_TEXTURE;
		break;
	case kSamplerFilterCmpMinPointMagMipLinear:
		minFilter = GL_NEAREST_MIPMAP_LINEAR;
		magFilter = GL_LINEAR;
		compMode = GL_COMPARE_R_TO_TEXTURE;
		break;
	case kSamplerFilterCmpMinLinearMagMipPoint:
		minFilter = GL_LINEAR_MIPMAP_NEAREST;
		magFilter = GL_NEAREST;
		compMode = GL_COMPARE_R_TO_TEXTURE;
		break;
	case kSamplerFilterCmpMinLinearMagPointMipLinear:
		minFilter = GL_LINEAR_MIPMAP_LINEAR;
		magFilter = GL_NEAREST;
		compMode = GL_COMPARE_R_TO_TEXTURE;
		break;
	case kSamplerFilterCmpMinMagLinearMipPoint:
		minFilter = GL_LINEAR_MIPMAP_NEAREST;
		magFilter = GL_LINEAR;
		compMode = GL_COMPARE_R_TO_TEXTURE;
		break;
	case kSamplerFilterCmpMinMagMipLinear:
		minFilter = GL_LINEAR_MIPMAP_LINEAR;
		magFilter = GL_LINEAR;
		compMode = GL_COMPARE_R_TO_TEXTURE;
		break;
	case kSamplerFilterCmpAnisotropic:
		minFilter = GL_LINEAR_MIPMAP_LINEAR;
		magFilter = GL_LINEAR;
		compMode = GL_COMPARE_R_TO_TEXTURE;
		break;
	default:
		break;
	}

	return std::make_tuple(minFilter, magFilter, compMode);
}

GLenum GetGlCompFunc(CompareFunc compFunc)
{
	GLenum glCompFunc = GL_NONE;
	switch (compFunc)
	{
	case kCompareNever:
		glCompFunc = GL_NEVER;
		break;
	case kCompareLess:
		glCompFunc = GL_LESS;
		break;
	case kCompareEqual:
		glCompFunc = GL_EQUAL;
		break;
	case kCompareLessEqual:
		glCompFunc = GL_LEQUAL;
		break;
	case kCompareGreater:
		glCompFunc = GL_GREATER;
		break;
	case kCompareNotEqual:
		glCompFunc = GL_NOTEQUAL;
		break;
	case kCompareGreaterEqual:
		glCompFunc = GL_GEQUAL;
		break;
	case kCompareAlways:
		glCompFunc = GL_ALWAYS;
		break;
	case kCompareUnkown:
	default:
		break;
	}
	return glCompFunc;
}

GLenum GetGlSamplerAddressMode(AddressMode addressMode)
{
	GLenum glAddrMode = GL_NONE;
	switch (addressMode)
	{
	case kAddressWrap:
		glAddrMode = GL_MIRRORED_REPEAT;
		break;
	case kAddressMirror:
		glAddrMode = GL_MIRRORED_REPEAT;
		break;
	case kAddressClamp:
		glAddrMode = GL_CLAMP_TO_EDGE;
		break;
	case kAddressBorder:
		glAddrMode = GL_CLAMP_TO_BORDER;
		break;
	case kAddressMirrorOnce:
		glAddrMode = GL_MIRROR_CLAMP_TO_EDGE;
		break;
	case kAddressUnkown:
	default:
		break;
	}
	return glAddrMode;
}

GLenum GetGlBlendFunc(BlendFunc func)
{
	GLenum glFunc = GL_NONE;
	switch (func)
	{
	case kBlendZero:
		glFunc = GL_ZERO;
		break;
	case kBlendOne:
		glFunc = GL_ONE;
		break;
	case kBlendSrcColor:
		glFunc = GL_SRC_COLOR;
		break;
	case kBlendInvSrcColor:
		glFunc = GL_ONE_MINUS_SRC_COLOR;
		break;
	case kBlendSrcAlpha:
		glFunc = GL_SRC_ALPHA;
		break;
	case kBlendInvSrcAlpha:
		glFunc = GL_ONE_MINUS_SRC_ALPHA;
		break;
	case kBlendDstAlpha:
		glFunc = GL_DST_ALPHA;
		break;
	case kBlendInvDstAlpha:
		glFunc = GL_ONE_MINUS_DST_ALPHA;
		break;
	case kBlendDstColor:
		glFunc = GL_DST_COLOR;
		break;
	case kBlendInvDstColor:
		glFunc = GL_ONE_MINUS_DST_COLOR;
		break;
	case kBlendSrcAlphaSat:
		glFunc = GL_SRC_ALPHA_SATURATE;
		break;
	case kBlendBlendFactor:
		glFunc = GL_CONSTANT_COLOR;
		break;
	case kBlendInvBlendFactor:
		glFunc = GL_ONE_MINUS_CONSTANT_COLOR;
		break;
	case kBlendSrc1Color:
		glFunc = GL_SRC1_COLOR;
		break;
	case kBlendInvSrc1Color:
		glFunc = GL_ONE_MINUS_SRC1_COLOR;
		break;
	case kBlendSrc1Alpha:
		glFunc = GL_SRC1_ALPHA;
		break;
	case kBlendInvSrc1Alpha:
		glFunc = GL_ONE_MINUS_SRC1_ALPHA;
		break;
	default:
		break;
	}
	return glFunc;
}

GLenum ogl::GetShaderType(int type)
{
	GLenum glType = GL_NONE;
	switch (type)
	{
	case kShaderVertex:
		glType = GL_VERTEX_SHADER;
		break;
	case kShaderPixel:
		glType = GL_FRAGMENT_SHADER;
		break;
	default:
		break;
	}
	return glType;
}

GLenum GetTopologyType(PrimitiveTopology topo)
{
	GLenum glTopo = GL_NONE;
	switch (topo)
	{
	case kPrimTopologyPointList:
		glTopo = GL_POINTS;
		break;
	case kPrimTopologyLineList:
		glTopo = GL_LINES;
		break;
	case kPrimTopologyLineStrip:
		glTopo = GL_LINE_STRIP;
		break;
	case kPrimTopologyTriangleList:
		glTopo = GL_TRIANGLES;
		break;
	case kPrimTopologyTriangleStrip:
		glTopo = GL_TRIANGLE_STRIP;
		break;
	case kPrimTopologyUnkown:
	default:
		break;
	}
	return glTopo;
}

GLenum GLCheckError(const char* file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		//std::cout << error << " | " << file << " (" << line << ")" << std::endl;
		DEBUG_LOG_ERROR((boost::format("%s: %s(%d)") % error % file % line).str().c_str());
	}
	return errorCode;
}

}
}