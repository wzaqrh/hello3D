#include <boost/assert.hpp>
#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <gli/gli.hpp>
#include <OpenImageIO/imageio.h>
#include <FreeImage/FreeImagePlus.h>
#include "core/base/il_helper.h"
#include "core/base/macros.h"
#include "core/base/debug.h"
#include "core/resource/texture_factory.h"
#include "core/resource/resource_manager.h"

//#define USE_OIIO
#define USE_FREE_IMAGE

namespace mir {
namespace res {

TextureFactory::TextureFactory(ResourceManager& resMng)
: mResMng(resMng)
, mRenderSys(resMng.RenderSys())
{
}

#if defined USE_FREE_IMAGE
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
	}
	else 
	{
		fipImage fi;
		if (fi.load(imgFullPath.c_str()))
		{
			faceCount = 1;
			mipCount = 1;

			width = fi.getWidth();
			height = fi.getHeight();

			const int stride = fi.getScanWidth();
			vecData.push_back(Data::Make(fi.accessPixels(), stride));

			if (format == kFormatUnknown)
			{
				ResourceBaseFormat baseFormat = kRBF_Unkown;
				ResourceDataType dataType = kRDT_Max;
				int bitsPerChannel = 0;
				bool isSRGB = false;

				static auto CalBaseFormat = [](FREE_IMAGE_COLOR_TYPE colorType)
				{
					ResourceBaseFormat baseFormat = kRBF_Unkown;
					switch (colorType)
					{
					case FIC_RGB: baseFormat = kRBF_RGB; break;
					case FIC_RGBALPHA: baseFormat = kRBF_RGBA; break;
					default: BOOST_ASSERT(false); break;
					}
					return baseFormat;
				};
				FREE_IMAGE_COLOR_TYPE fiColorType = fi.getColorType();
				baseFormat = CalBaseFormat(fiColorType);

				int bpp = fi.getBitsPerPixel();
				FREE_IMAGE_TYPE fiImgType = fi.getImageType();
				switch (fiImgType)
				{
				case FIT_BITMAP: {
					BOOL res;
					switch (bpp) {
						//case 24: dataType = kRDT_UNorm; bitsPerChannel = 8; BOOST_ASSERT(baseFormat == kRBF_RGB); break;
					case 24: res = fi.convertTo32Bits(); BOOST_ASSERT(res); bitsPerChannel = 8; baseFormat = kRBF_RGBA; break;
					case 32: dataType = kRDT_UNorm; bitsPerChannel = 8; BOOST_ASSERT(baseFormat == kRBF_RGBA); break;
					default: BOOST_ASSERT(FALSE); break;
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

			if (mipCount == 1 && autoGenMipmap)
				mipCount = -1;

			CoAwait mResMng.SwitchToLaunchService(__LaunchSync__);
			texture->SetLoaded(mRenderSys.LoadTexture(texture, format, Eigen::Vector4i(width, height, 0, faceCount), mipCount, &vecData[0]) != nullptr);
		}
	}

	CoReturn texture->IsLoaded();
}
#elif defined USE_OIIO
CoTask<bool> TextureFactory::_LoadTextureByFile(ITexturePtr texture, Launch lchMode, std::string imgFullpath, ResourceFormat format, bool autoGenMipmap) ThreadSafe
{
	texture->SetLoading(); CoAwait mResMng.SwitchToLaunchService(lchMode);
	COROUTINE_VARIABLES_5(texture, lchMode, imgFullpath, format, autoGenMipmap);
	TIME_PROFILE((boost::format("\t\tresMng._LoadTextureByFile (%1% %2% %3%)") % imgFullpath % format % autoGenMipmap).str());

	ITexturePtr ret = nullptr;
	OIIO::ImageSpec spec;
	OIIO::ImageSpec config;
	config["oiio:UnassociatedAlpha"] = TRUE;
	config["oiio:RawColor"] = TRUE;
	if (auto inp = OIIO::ImageInput::open(imgFullpath, &config)) {
		auto get_spec_dimensions = [&](OIIO::ImageSpec& spec, int faceIdx, int mipIdx) -> bool {
			spec = inp->spec_dimensions(faceIdx, mipIdx);
			return spec.format != OIIO::TypeUnknown;
		};

		std::vector<unsigned char> bytes(1024);
		size_t bytePos = 0;
		std::vector<Data> vecData;
		int width = 0, height = 0;
		int faceCount = 0, mipCount = 0;
		while (++faceCount <= 6) {
			int mipPlus = 0;
			while (get_spec_dimensions(spec, faceCount - 1, mipPlus++)) {
				BOOST_ASSERT(spec.depth <= 1);
				//BOOST_ASSERT(spec.z_channel == -1);

				const char* compression = nullptr;
				spec.getattribute("compression", OIIO::TypeString, &compression);

				int channels = spec.nchannels;
				if (faceCount == 1 && mipPlus == 1) {
					width = spec.width;
					height = spec.height;
					if (format == kFormatUnknown) {
						auto specfmt = spec.format;
						BOOST_ASSERT(specfmt.basetype != OIIO::TypeDesc::UNKNOWN);
						BOOST_ASSERT(specfmt.aggregate == OIIO::TypeDesc::SCALAR);
						BOOST_ASSERT(specfmt.arraylen == 0);

						ResourceBaseFormat baseFormat = kRBF_Unkown;
						ResourceDataType dataType = kRDT_Max;
						int bitsPerChannel = 0;
						bool isSRGB = false;

						static auto CalBaseFormat = [](int channels)
						{
							ResourceBaseFormat baseFormat = kRBF_Unkown;
							switch (channels)
							{
							case 1: baseFormat = kRBF_R; break;
							case 2: baseFormat = kRBF_RG; break;
							case 3: baseFormat = kRBF_RGB; break;
							case 4: baseFormat = kRBF_RGBA; break;
							default: BOOST_ASSERT(false); break;
							}
							return baseFormat;
						};
						baseFormat = CalBaseFormat(spec.nchannels);

						switch (specfmt.basetype)
						{
						case OIIO::TypeDesc::UINT8: dataType = kRDT_UNorm; bitsPerChannel = 8; break;
						case OIIO::TypeDesc::INT8: dataType = kRDT_Int; bitsPerChannel = 8; break;
						case OIIO::TypeDesc::UINT16: dataType = kRDT_UInt; bitsPerChannel = 16; break;
						case OIIO::TypeDesc::INT16: dataType = kRDT_Int; bitsPerChannel = 16; break;
						case OIIO::TypeDesc::UINT32: dataType = kRDT_UInt; bitsPerChannel = 32; break;
						case OIIO::TypeDesc::INT32: dataType = kRDT_Int; bitsPerChannel = 32; break;
						case OIIO::TypeDesc::HALF: dataType = kRDT_Float; bitsPerChannel = 16; break;
						case OIIO::TypeDesc::FLOAT: dataType = kRDT_Float; bitsPerChannel = 32; break;
						default: break;
						}

						format = MakeResFormat(baseFormat, dataType, bitsPerChannel, isSRGB);
						if (format == kFormatUnknown && channels == 3) {
							baseFormat = CalBaseFormat(++channels);
							format = MakeResFormat(baseFormat, dataType, bitsPerChannel, isSRGB);
						}
						BOOST_ASSERT(format != kFormatUnknown);
					}
				}

				size_t imgSize = spec.image_bytes(true); BOOST_ASSERT(imgSize % spec.height == 0);
				if (bytePos + imgSize > bytes.size())
					bytes.resize(std::max(bytePos + imgSize, bytes.size() * 2));
				inp->read_image(faceCount - 1, mipPlus - 1, 0, channels, OIIO::TypeDesc::UNKNOWN, &bytes[bytePos]);

				vecData.emplace_back(Data::Make((void*)bytePos, imgSize / spec.height));

				bytePos += imgSize;
			}
			if (mipPlus == 1)
				break;
			if (faceCount == 1)
				mipCount = mipPlus - 1;
			BOOST_ASSERT(mipCount == mipPlus - 1);
		}

		for (auto& data : vecData)
			data.Bytes = &bytes[(int)data.Bytes];

		if (mipCount == 1 && autoGenMipmap)
			mipCount = -1;

		CoAwait mResMng.SwitchToLaunchService(__LaunchSync__);
		ret = mRenderSys.LoadTexture(texture, format, Eigen::Vector4i(width, height, 0, faceCount - 1), mipCount, &vecData[0]);

		inp->close();
	}//if fd

	texture->SetLoaded(ret != nullptr);
	CoReturn texture->IsLoaded();
}
#else
CoTask<bool> TextureFactory::_LoadTextureByFile(ITexturePtr texture, Launch lchMode, std::string imgFullpath, ResourceFormat format, bool autoGenMipmap) ThreadSafe
{
	texture->SetLoading(); CoAwait SwitchToLaunchService(lchMode);
	COROUTINE_VARIABLES_5(texture, lchMode, imgFullpath, format, autoGenMipmap);

	TIME_PROFILE((boost::format("\t\tresMng._LoadTextureByFile (%1% %2% %3%)") % imgFullpath % format % autoGenMipmap).str());
	ITexturePtr ret = nullptr;

	FILE* fd = fopen(imgFullpath.c_str(), "rb"); BOOST_ASSERT(fd);
	if (fd) {
		ILuint imageId = ilGenImage();
		ilBindImage(imageId);

		ILenum ilFileType = il_helper::DetectType(fd);
		switch (ilFileType) {
		case IL_DDS:
		case IL_TGA:
		case IL_HDR:
		case IL_KTX:
			ilSetInteger(IL_KEEP_DXTC_DATA, IL_TRUE);
			break;
		default:
			break;
		}
		BOOST_ASSERT(il_helper::CheckLastError());

		if (ilFileType != IL_TYPE_UNKNOWN && ilLoadF(ilFileType, fd))
		{
			ILuint width = ilGetInteger(IL_IMAGE_WIDTH),
				height = ilGetInteger(IL_IMAGE_HEIGHT),
				channel = ilGetInteger(IL_IMAGE_CHANNELS),
				bpp = ilGetInteger(IL_IMAGE_BPP),
				faceCount = ilGetInteger(IL_NUM_FACES) + 1,
				mipCount = ilGetInteger(IL_NUM_MIPMAPS) + 1,
				imageSize = ilGetInteger(IL_IMAGE_SIZE_OF_DATA),
				ilFormat0 = ilGetInteger(IL_IMAGE_FORMAT),
				ilFormat1 = ilGetInteger(IL_IMAGE_TYPE);

			ResourceFormat convertFormat = kFormatUnknown;//convertFormat非unknown时要转换格式
			ILenum convertImageFormat = 0, convertImageType = 0;
			if (format == kFormatUnknown) {
				//检测图片格式
				format = il_helper::ConvertImageFormatTypeToResourceFormat(ilFormat0, ilFormat1);
				if (format == kFormatUnknown) {
					//图片格式不能直接用于纹理，尝试转换成可用的
					convertImageFormat = il_helper::Convert3ChannelImageFormatTo4Channel(ilFormat0);
					convertImageType = ilFormat1;
					convertFormat = il_helper::ConvertImageFormatTypeToResourceFormat(convertImageFormat, ilFormat1);
					format = convertFormat;
				}
			}
			else {
				//检测能否 使用 或 转换成用户想要的格式
				std::tie(convertImageFormat, convertImageType) = il_helper::ConvertResourceFormatToILImageFormatType(format);
				if (convertImageFormat == 0) {
					format = kFormatUnknown;//不能使用与转换
				}
				else if (convertImageFormat != ilFormat0 || convertImageType != ilFormat1) {
					convertFormat = format;
				}
				else {
					//直接使用, 不用转换
				}
			}

			if (format != kFormatUnknown || convertFormat != kFormatUnknown) {
				std::vector<Data> datas(mipCount * faceCount, Data{});

				if (convertFormat != kFormatUnknown)
					bpp = d3d::BytePerPixel(static_cast<DXGI_FORMAT>(convertFormat));
				int faceSize = width * height * bpp;
				std::vector<unsigned char> bytes;
				bytes.resize(faceSize * faceCount * 2/*2 > 1+1/2+1/4+1/8...1/2n*/);
				size_t bytes_position = 0;

				for (int face = 0; face < faceCount; ++face) {
					for (int mip = 0; mip < mipCount; ++mip) {
						ilBindImage(imageId);
						ilActiveImage(0);
						ilActiveFace(face);
						ilActiveMipmap(mip);
						BOOST_ASSERT(il_helper::CheckLastError());

						//size_t mip_width = width >> mip, mip_height = height >> mip;
						ILuint mip_width = ilGetInteger(IL_IMAGE_WIDTH),
							mip_height = ilGetInteger(IL_IMAGE_HEIGHT);
						auto& dataFM = datas[face * mipCount + mip];
						if (convertFormat == kFormatUnknown)
						{
							//直接使用
							size_t sub_res_size = 0;
							ILuint dxtFormat = ilGetInteger(IL_DXTC_DATA_FORMAT);
							ResourceFormat compressFormat = il_helper::ConvertILFormatToResourceFormat(dxtFormat);
							if (compressFormat != kFormatUnknown) {
								sub_res_size = ilGetDXTCData(NULL, 0, dxtFormat);
								if (sub_res_size) {
									format = compressFormat;

									BOOST_ASSERT(bytes_position + sub_res_size <= bytes.size());
									ilGetDXTCData(&bytes[bytes_position], sub_res_size, dxtFormat);

									dataFM.Bytes = &bytes[bytes_position];

									size_t rowPitch, slicePitch;
									bool res = d3d::ComputePitch(static_cast<DXGI_FORMAT>(format), mip_width, mip_height,
										rowPitch, slicePitch, 0);
									BOOST_ASSERT(res);
									dataFM.Size = rowPitch;

									bytes_position += sub_res_size;
								}
								ilGetError();
							}

							if (sub_res_size == 0) {
								size_t sub_res_size = mip_height * mip_width * bpp;
								BOOST_ASSERT(bytes_position + sub_res_size <= bytes.size());
								ilCopyPixels(0, 0, 0,
									mip_width, mip_height, 1,
									ilFormat0, ilFormat1, &bytes[bytes_position]);

								dataFM.Bytes = &bytes[bytes_position];
								dataFM.Size = mip_width * bpp;
								bytes_position += sub_res_size;
							}
						}
						else {
							//转换格式
							size_t sub_res_size = mip_height * mip_width * bpp;
							BOOST_ASSERT(bytes_position + sub_res_size <= bytes.size());
							ilCopyPixels(0, 0, 0,
								mip_width, mip_height, 1,
								convertImageFormat, convertImageType, &bytes[bytes_position]);

							dataFM.Bytes = &bytes[bytes_position];
							dataFM.Size = mip_width * bpp;
							bytes_position += sub_res_size;
						}
					}//for mip
				}//for face

				if (format != kFormatUnknown) {
					if (mipCount == 1 && autoGenMipmap)
						mipCount = -1;
					ilDeleteImage(imageId);
					fclose(fd);
					CoAwait SwitchToLaunchService(__LaunchSync__);
					ret = mRenderSys.LoadTexture(texture, format, Eigen::Vector4i(width, height, 0, faceCount), mipCount, &datas[0]);
				}
			}
		}
		else {
			BOOST_ASSERT(il_helper::CheckLastError());
		}

		if (ret == nullptr) {
			ilDeleteImage(imageId);
			fclose(fd);
		}
	}//if fd

	texture->SetLoaded(ret != nullptr);
	CoReturn texture->IsLoaded();
}
#endif

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