#include <boost/assert.hpp>
#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <gli/gli.hpp>
#include <FreeImage/FreeImagePlus.h>
#include "core/base/macros.h"
#include "core/base/debug.h"
#include "core/resource/texture_factory.h"
#include "core/resource/resource_manager.h"

namespace mir {
namespace res {

TextureFactory::TextureFactory(ResourceManager& resMng)
: mResMng(resMng)
, mRenderSys(resMng.RenderSys())
{
}

template <class T> void INPLACESWAP(T& a, T& b) { a ^= b; b ^= a; a ^= b; }
static BOOL SwapRedBlue32(FIBITMAP* dib) 
{
	if (FreeImage_GetImageType(dib) != FIT_BITMAP) {
		return FALSE;
	}

	const unsigned bytesperpixel = FreeImage_GetBPP(dib) / 8;
	if (bytesperpixel > 4 || bytesperpixel < 3) {
		return FALSE;
	}

	const unsigned height = FreeImage_GetHeight(dib);
	const unsigned pitch = FreeImage_GetPitch(dib);
	const unsigned lineSize = FreeImage_GetLine(dib);

	BYTE* line = FreeImage_GetBits(dib);
	for (unsigned y = 0; y < height; ++y, line += pitch) {
		for (BYTE* pixel = line; pixel < line + lineSize; pixel += bytesperpixel) {
			INPLACESWAP(pixel[0], pixel[2]);
		}
	}

	return TRUE;
}
CoTask<bool> TextureFactory::_LoadTextureByFile(ITexturePtr texture, Launch lchMode, std::string imgFullPath, ResourceFormat format, bool autoGenMipmap) ThreadSafe
{
	texture->SetLoading(); CoAwait mResMng.SwitchToLaunchService(lchMode);
	COROUTINE_VARIABLES_5(texture, lchMode, imgFullPath, format, autoGenMipmap);
	TIME_PROFILE((boost::format("\t\tresMng._LoadTextureByFile (%1% %2% %3%)") %imgFullPath %format %autoGenMipmap).str());

	int faceCount = 0;
	int mipCount = 0;
	int width = 0;
	int height = 0;
	std::vector<Data> vecData;

	boost::filesystem::path path = imgFullPath;
	static std::string gliPatterns[] = { ".dds", ".ktx", ".ktx2" };
	if (std::find(std::begin(gliPatterns), std::end(gliPatterns), path.extension()) != std::end(gliPatterns)) 
	{
		gli::texture tex = gli::load(imgFullPath);
		if (!tex.empty())
		{
			BOOST_ASSERT(tex.layers() == 1);
			BOOST_ASSERT(tex.base_face() == 0 && (tex.faces() == 1 || tex.faces() == 6));
			BOOST_ASSERT(tex.base_level() == 0);
			faceCount = tex.faces();
			mipCount = tex.levels();

			auto extent = tex.extent(0);
			width = extent.x;
			height = extent.y;

			constexpr int layer0 = 0;
			auto block_ext = gli::block_extent(tex.format());
			auto block_size = gli::block_size(tex.format());
			BOOST_ASSERT(block_size % block_ext.y == 0);
			for (int face = 0; face < faceCount; ++face) {
				for (int level = 0; level < mipCount; ++level) {
					auto extent = tex.extent(level);
					BOOST_ASSERT(tex.size(level) == block_size * FLOOR_DIV(extent.x, block_ext.x) * FLOOR_DIV(extent.y, block_ext.y));
					int pitch = block_size * FLOOR_DIV(extent.x, block_ext.x);
					vecData.push_back(Data::Make(tex.data(layer0, face, level), pitch));
				}
			}

			if (format == kFormatUnknown) {
				ResourceBaseFormat baseFormat = kRBF_Unkown;
				ResourceDataType dataType = kRDT_Max;
				int bitsPerChannel = 0;
				bool isSRGB = false;

				gli::gl GL(gli::gl::PROFILE_GL33);
				const gli::gl::format fmt = GL.translate(tex.format(), tex.swizzles());

				//https://www.khronos.org/opengl/wiki/Image_Format
				if (gli::is_compressed(tex.format())) {
					switch (fmt.Internal) {
					case gli::gl::INTERNAL_RGB_DXT1: format = kFormatBC1UNorm; break;
					case gli::gl::INTERNAL_RGBA_DXT1: format = kFormatBC1UNorm; break;
					case gli::gl::INTERNAL_RGBA_DXT3: format = kFormatBC2UNorm; break;
					case gli::gl::INTERNAL_RGBA_DXT5: format = kFormatBC3UNorm; break;
					case gli::gl::INTERNAL_SRGB_DXT1: format = kFormatBC1UNormSRgb; break;
					case gli::gl::INTERNAL_SRGB_ALPHA_DXT1: format = kFormatBC1UNormSRgb; break;
					case gli::gl::INTERNAL_SRGB_ALPHA_DXT3: format = kFormatBC2UNormSRgb; break;
					case gli::gl::INTERNAL_SRGB_ALPHA_DXT5: format = kFormatBC3UNormSRgb; break;
					default:
						BOOST_ASSERT(FALSE);
						break;
					}
				}
				else {
					switch (fmt.External) {
					case gli::gl::EXTERNAL_RED: baseFormat = kRBF_R; break;
					case gli::gl::EXTERNAL_RG: baseFormat = kRBF_RG; break;
					case gli::gl::EXTERNAL_RGB: baseFormat = kRBF_RGB; break;
					case gli::gl::EXTERNAL_BGR: baseFormat = kRBF_BGRX; break;
					case gli::gl::EXTERNAL_RGBA: baseFormat = kRBF_RGBA; break;
					case gli::gl::EXTERNAL_BGRA: baseFormat = kRBF_BGRA; break;
					case gli::gl::EXTERNAL_DEPTH: baseFormat = kRBF_D; break;
					case gli::gl::EXTERNAL_DEPTH_STENCIL: baseFormat = kRBF_DS; break;
					case gli::gl::EXTERNAL_ALPHA: baseFormat = kRBF_A; break;
					case gli::gl::EXTERNAL_LUMINANCE:
					case gli::gl::EXTERNAL_LUMINANCE_ALPHA:
					case gli::gl::EXTERNAL_SRGB_EXT:
					case gli::gl::EXTERNAL_SRGB_ALPHA_EXT:
					case gli::gl::EXTERNAL_NONE:
					case gli::gl::EXTERNAL_RED_INTEGER:
					case gli::gl::EXTERNAL_RG_INTEGER:
					case gli::gl::EXTERNAL_RGB_INTEGER:
					case gli::gl::EXTERNAL_BGR_INTEGER:
					case gli::gl::EXTERNAL_RGBA_INTEGER:
					case gli::gl::EXTERNAL_BGRA_INTEGER:
					case gli::gl::EXTERNAL_STENCIL:
					default:
						BOOST_ASSERT(FALSE);
						break;
					}

					switch (fmt.Type) {
					case gli::gl::TYPE_I8: dataType = kRDT_SNorm; bitsPerChannel = 8; break;
					case gli::gl::TYPE_U8: dataType = kRDT_UNorm; bitsPerChannel = 8; break;
					case gli::gl::TYPE_I16: dataType = kRDT_Int; bitsPerChannel = 16; break;
					case gli::gl::TYPE_U16: dataType = kRDT_UInt; bitsPerChannel = 16; break;
					case gli::gl::TYPE_I32: dataType = kRDT_Int; bitsPerChannel = 32; break;
					case gli::gl::TYPE_U32: dataType = kRDT_UInt; bitsPerChannel = 32; break;
					case gli::gl::TYPE_I64: dataType = kRDT_Int; bitsPerChannel = 64; break;
					case gli::gl::TYPE_U64: dataType = kRDT_UInt; bitsPerChannel = 64; break;
					case gli::gl::TYPE_F16: dataType = kRDT_Float; bitsPerChannel = 16; break;
					case gli::gl::TYPE_F32: dataType = kRDT_Float; bitsPerChannel = 32; break;
					case gli::gl::TYPE_F64: dataType = kRDT_Float; bitsPerChannel = 64; break;
					case gli::gl::TYPE_NONE:
					case gli::gl::TYPE_F16_OES:
					case gli::gl::TYPE_UINT32_RGB9_E5_REV:
					case gli::gl::TYPE_UINT32_RG11B10F_REV:
					case gli::gl::TYPE_UINT8_RG3B2:
					case gli::gl::TYPE_UINT8_RG3B2_REV:
					case gli::gl::TYPE_UINT16_RGB5A1:
					case gli::gl::TYPE_UINT16_RGB5A1_REV:
					case gli::gl::TYPE_UINT16_R5G6B5:
					case gli::gl::TYPE_UINT16_R5G6B5_REV:
					case gli::gl::TYPE_UINT16_RGBA4:
					case gli::gl::TYPE_UINT16_RGBA4_REV:
					case gli::gl::TYPE_UINT32_RGBA8:
					case gli::gl::TYPE_UINT32_RGBA8_REV:
					case gli::gl::TYPE_UINT32_RGB10A2:
					case gli::gl::TYPE_UINT32_RGB10A2_REV:
					case gli::gl::TYPE_UINT8_RG4_REV_GTC:
					case gli::gl::TYPE_UINT16_A1RGB5_GTC:
					default:
						BOOST_ASSERT(FALSE);
						break;
					}

					format = MakeResFormat(baseFormat, dataType, bitsPerChannel, isSRGB);
					BOOST_ASSERT(format != kFormatUnknown);
				}
			}
		
			if (mipCount == 1 && autoGenMipmap)
				mipCount = -1;

			CoAwait mResMng.SwitchToLaunchService(__LaunchSync__);
			texture->SetLoaded(mRenderSys.LoadTexture(texture, format, Eigen::Vector4i(width, height, 0, faceCount), mipCount, &vecData[0]) != nullptr);
		}
		BOOST_ASSERT(!tex.empty());
	}
	else 
	{
		static std::string flipExts[] = { ".png", ".jpg", ".jpeg", ".bmp"};
		const bool isPngJpgOrBmp = std::find(std::begin(flipExts), std::end(flipExts), path.extension()) != std::end(flipExts);

		fipImage fi;
		if (fi.load(imgFullPath.c_str()))
		{
			if (format == kFormatUnknown)
			{
				ResourceBaseFormat baseFormat = kRBF_Unkown;
				ResourceDataType dataType = kRDT_Max;
				int bitsPerChannel = 0;
				bool isSRGB = false;

				int bpp = fi.getBitsPerPixel();
				FREE_IMAGE_COLOR_TYPE fiColorType = fi.getColorType();
				switch (fiColorType)
				{
				case FIC_RGB: baseFormat = kRBF_RGB; break;
				case FIC_RGBALPHA: baseFormat = kRBF_RGBA; break;
				case FIC_MINISBLACK: baseFormat = kRBF_R; break;
				default: BOOST_ASSERT(false); break;
				}

				FREE_IMAGE_TYPE fiImgType = fi.getImageType();
				switch (fiImgType)
				{
				case FIT_BITMAP: {
					if (bpp == 24) {
						BOOL res = fi.convertTo32Bits(); BOOST_ASSERT(res && fi.getBitsPerPixel() == 32);
						dataType = kRDT_UNorm; bitsPerChannel = 8; BOOST_ASSERT(baseFormat == kRBF_RGBA || baseFormat == kRBF_RGB); baseFormat = kRBF_RGBA;
					}
					else if (bpp == 32) {
						dataType = kRDT_UNorm; bitsPerChannel = 8; BOOST_ASSERT(baseFormat == kRBF_RGBA || baseFormat == kRBF_RGB); baseFormat = kRBF_RGBA;
					}
					else if (bpp == 8) {
						dataType = kRDT_UNorm; bitsPerChannel = 8; BOOST_ASSERT(baseFormat == kRBF_R);
					}
					else {
						BOOST_ASSERT(FALSE);
					}
				}break;
				case FIT_RGB16: dataType = kRDT_Float; bitsPerChannel = 16; BOOST_ASSERT(baseFormat == kRBF_RGB && bpp == 48); break;
				case FIT_RGBA16: dataType = kRDT_Float; bitsPerChannel = 16; BOOST_ASSERT(baseFormat == kRBF_RGBA && bpp == 64); break;
				case FIT_RGBF: dataType = kRDT_Float; bitsPerChannel = 32; BOOST_ASSERT(baseFormat == kRBF_RGB && bpp == 96); break;
				case FIT_RGBAF: dataType = kRDT_Float; bitsPerChannel = 32; BOOST_ASSERT(baseFormat == kRBF_RGBA && bpp == 128); break;
				default: BOOST_ASSERT(FALSE); break;
				}

				format = MakeResFormat(baseFormat, dataType, bitsPerChannel, isSRGB);
				BOOST_ASSERT(format != kFormatUnknown);
			}

			width = fi.getWidth();
			height = fi.getHeight();
			
			if (isPngJpgOrBmp) {
			#if !defined FREEIMAGE_BIGENDIAN
				if (fi.getImageType() == FIT_BITMAP && fi.getBitsPerPixel() == 32) {
					SwapRedBlue32(fi);
				}
			#endif
				fi.flipVertical();
			}
			const int stride = fi.getScanWidth();
			vecData.push_back(Data::Make(fi.accessPixels(), stride));
			
			faceCount = 1;
			mipCount = 1;

			if (mipCount == 1 && autoGenMipmap)
				mipCount = -1;

			CoAwait mResMng.SwitchToLaunchService(__LaunchSync__);
			texture->SetLoaded(mRenderSys.LoadTexture(texture, format, Eigen::Vector4i(width, height, 0, faceCount), mipCount, &vecData[0]) != nullptr);
		}
		BOOST_ASSERT(fi.isValid());
	}

	BOOST_ASSERT(texture->IsLoaded());
	CoReturn texture->IsLoaded();
}

CoTask<bool> TextureFactory::CreateTextureByFile(ITexturePtr& texture, Launch lchMode, std::string filepath, ResourceFormat format, bool autoGenMipmap) ThreadSafe
{
	//CoAwait SwitchToLaunchService(lchMode);
	COROUTINE_VARIABLES_4(lchMode, filepath, format, autoGenMipmap);

	boost::filesystem::path fullpath = boost::filesystem::system_complete(filepath);
	std::string key = fullpath.string();

	bool resNeedLoad = false;
	texture = mTextureByKey.GetOrAdd(key, [&]() {
		auto texture = std::static_pointer_cast<ITexture>(this->mRenderSys.CreateResource(kDeviceResourceTexture));
		DEBUG_SET_RES_PATH(texture, (boost::format("path:%1%, fmt:%2%, autogen:%3%") % filepath % format % autoGenMipmap).str());
		DEBUG_SET_CALL(texture, lchMode);
		resNeedLoad = true;
		return texture;
		});
	if (resNeedLoad) {
		CoAwait this->_LoadTextureByFile(texture, lchMode, std::move(key), format, autoGenMipmap);
	}
	else {
		CoAwait mResMng.WaitResComplete(texture);
	}
	CoReturn texture->IsLoaded();
}
}
}