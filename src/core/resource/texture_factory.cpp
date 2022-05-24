#include <boost/assert.hpp>
#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <OpenImageIO/imageio.h>
#include "core/base/il_helper.h"
#include "core/base/debug.h"
#include "core/resource/texture_factory.h"
#include "core/resource/resource_manager.h"

#define USE_OIIO

namespace mir {
namespace res {

TextureFactory::TextureFactory(ResourceManager& resMng)
	: mResMng(resMng)
	, mRenderSys(resMng.RenderSys())
{
}

#if defined USE_OIIO
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