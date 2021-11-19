#include <boost/assert.hpp>
#include "core/base/il_helper.h"
#include "core/rendersys/resource_manager.h"
#include "core/rendersys/render_system.h"
#include "core/rendersys/interface_type.h"

namespace mir {

ResourceManager::ResourceManager(RenderSystem& renderSys)
	:mRenderSys(renderSys)
{
	ilInit();
}

ResourceManager::~ResourceManager()
{
	ilShutDown();
}

void ResourceManager::UpdateForLoading()
{
	const std::vector<IResourcePtr>& topNodes = mResDependencyTree.TopNodes();
	for (auto& res : topNodes) {
		if (res->IsPreparedNeedLoading()) {
			auto task = mLoadTaskByRes[res];
			task(res);
		}
	}
	for (auto& res : topNodes) {
		if (res->IsLoaded()) {
			mResDependencyTree.RemoveNode(res);
			mLoadTaskByRes.erase(res);
		}
	}
}

void ResourceManager::AddResourceDependency(IResourcePtr node, IResourcePtr parent)
{
	mResDependencyTree.AddNode(node, parent);
}

ITexturePtr ResourceManager::DoCreateTexture(const std::string& imgFullpath, ResourceFormat format)
{
	ITexturePtr ret = nullptr;

	FILE* fd = fopen(imgFullpath.c_str(), "rb"); BOOST_ASSERT(fd);
	if (fd) {
		ITexturePtr texture = std::static_pointer_cast<ITexture>(mRenderSys.CreateResource(kDeviceResourceTexture));

		ILuint imageId = ilGenImage();
		ilBindImage(imageId);

		ilSetInteger(IL_KEEP_DXTC_DATA, IL_TRUE);
		BOOST_ASSERT(il_helper::CheckLastError());

		ILenum ilType = il_helper::DetectType(fd);
		if (ilType != IL_TYPE_UNKNOWN && ilLoadF(ilType, fd)) {
			ILuint width = ilGetInteger(IL_IMAGE_WIDTH), 
			height  = ilGetInteger(IL_IMAGE_HEIGHT),
			channel = ilGetInteger(IL_IMAGE_CHANNELS), 
			bpp		= ilGetInteger(IL_IMAGE_BPP),
			faceCount = ilGetInteger(IL_NUM_FACES) + 1,
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
				std::vector<Data> datas(faceCount, {});
				std::vector<unsigned char> bytes;
				int faceSize = width * height * bpp;
				for (int face = 0; face < faceCount; ++face) {
					ilBindImage(imageId);
					ilActiveImage(0);
					ilActiveFace(face);
					BOOST_ASSERT(il_helper::CheckLastError());

					if (convertFormat == kFormatUnknown) {
						ILuint compressMode = ilGetInteger(IL_COMPRESS_MODE);

						size_t compressSize = 0;
						if (ilType == IL_DDS) {
							ILuint dxtFormat = ilGetInteger(IL_DXTC_DATA_FORMAT);
							format = il_helper::ConvertILFormatToResourceFormat(dxtFormat);

							compressSize = ilGetDXTCData(NULL, 0, dxtFormat);
							if (compressSize) {
								size_t position = bytes.size();
								bytes.resize(position + compressSize);
								ilGetDXTCData(&bytes[position], imageSize, dxtFormat);
								datas[face].Bytes = &bytes[position];
								datas[face].Size = faceSize;
							}
							il_helper::CheckLastError();
						}
						
						if (compressSize == 0) {
							datas[face].Bytes = ilGetData();
							datas[face].Size = faceSize;
						}
					}
					else {
						if (bytes.empty()) bytes.resize(faceSize * faceCount);
						ilCopyPixels(0, 0, 0, width, height, 1, convertImageFormat, ilFormat1, &bytes[faceSize * face]);
						datas[face].Bytes = &bytes[faceSize * face];
						datas[face].Size = faceSize;
					}
				}

				if (format != kFormatUnknown) {
					constexpr int mipCount = 1;
					mRenderSys.LoadTexture(texture, format, Eigen::Vector4i(width, height, 0, faceCount), mipCount, &datas[0]);
					ret = texture;
				}
			}
		}

		ilDeleteImage(imageId);
		fclose(fd);
	}
	return ret;
}

ITexturePtr ResourceManager::CreateTexture(const std::string& filepath, ResourceFormat format)
{
	boost::filesystem::path fullpath = boost::filesystem::system_complete(filepath);
	std::string imgFullpath = fullpath.string();

	ITexturePtr texture = nullptr;
	if (mTexByPath.find(imgFullpath) == mTexByPath.end()) {
		texture = DoCreateTexture(imgFullpath, format);

		mTexByPath.insert(std::make_pair(imgFullpath, texture));
	}
	else {
		texture = mTexByPath[imgFullpath];
	}
	return texture;
}

}