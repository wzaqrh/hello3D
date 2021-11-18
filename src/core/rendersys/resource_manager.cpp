#include <boost/assert.hpp>
#include <IL/il.h>
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

namespace il_helper {
static const ILenum CSupportILTypes[] = {
	IL_PNG, IL_JPG, IL_BMP, IL_TGA, IL_DDS, IL_GIF, IL_HDR, IL_ICO
};
static ILenum DetectType(FILE* fd) {
	for (int i = 0; i < sizeof(CSupportILTypes) / sizeof(CSupportILTypes[0]); ++i) {
		if (ilIsValidF(CSupportILTypes[i], fd))
			return CSupportILTypes[i];
	}
	return IL_TYPE_UNKNOWN;
}
};
ITexturePtr ResourceManager::DoCreateTexture(const std::string& imgFullpath, ResourceFormat format)
{
	ITexturePtr ret = nullptr;

	FILE* fd = fopen(imgFullpath.c_str(), "rb"); BOOST_ASSERT(fd);
	if (fd) {
		ITexturePtr texture = std::static_pointer_cast<ITexture>(mRenderSys.CreateResource(kDeviceResourceTexture));

		ILuint imageId = ilGenImage();
		ilBindImage(imageId);

		ILenum ilType = il_helper::DetectType(fd);
		if (ilType != IL_TYPE_UNKNOWN && ilLoadF(ilType, fd)) {
			ILuint width = ilGetInteger(IL_IMAGE_WIDTH), height = ilGetInteger(IL_IMAGE_HEIGHT);
			ILuint faceCount = ilGetInteger(IL_NUM_FACES) + 1, bpp = ilGetInteger(IL_IMAGE_BPP);

			format = (format != kFormatUnknown) ? format : kFormatR8G8B8A8UNorm;
			std::vector<Data> datas(faceCount, {});
		#if 1
			int faceSize = width * height * bpp;
			for (int face = 0; face < faceCount; ++face) {
				ilBindImage(imageId);
				ilActiveImage(0);
				ilActiveFace(face);
				BOOST_ASSERT(IL_NO_ERROR == ilGetError());

				datas[face].Bytes = ilGetData();
				datas[face].Size = faceSize;
			}
		#else
			std::vector<unsigned char> bytes;
			switch (format) {
			case kFormatR8G8B8A8UNorm: {
				int faceSize = width * height * sizeof(char) * 4;
				bytes.resize(faceSize * faceCount);
				for (int face = 0; face < faceCount; ++face) {
					ilActiveFace(face);
					//memcpy(&bytes[faceSize * face], ilGetData());
					ilCopyPixels(0, 0, 0, width, height, 1, IL_RGBA, IL_UNSIGNED_BYTE, &bytes[faceSize * face]);
					datas[face].Bytes = &bytes[faceSize * face];
					datas[face].Size = faceSize;
				}
			}break;
			case kFormatR32G32B32A32Float: {
				for (int face = 0; face < faceCount; ++face) {
					ilBindImage(imageId);
					ilActiveImage(0);
					ilActiveFace(face);
					BOOST_ASSERT(IL_NO_ERROR == ilGetError());

					datas[face].Bytes = ilGetData();
					datas[face].Size = width * height * sizeof(float) * 4;;
				}
			} break;
			case kFormatR16G16B16A16Float: {
				for (int f = 0; f < faceCount; ++f) {
					int face = determineFace(f, true, true);
					ilActiveFace(face);
					ilActiveMipmap(1);
					datas[0].Bytes = ilGetData();
					datas[0].Size = 0;
					if (f == 4)
						break;
				}
			}break;
			default:
				BOOST_ASSERT(false);
				break;
			}
		#endif
			constexpr int mipCount = 1;
			mRenderSys.LoadTexture(texture, format, Eigen::Vector4i(width, height, 0, faceCount), mipCount, &datas[0]);
			ret = texture;
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