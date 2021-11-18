#include <boost/assert.hpp>
#include <IL/il.h>
#include "core/rendersys/resource_manager.h"
#include "core/rendersys/render_system.h"
#include "core/rendersys/interface_type.h"

namespace mir {

ResourceManager::ResourceManager(RenderSystem& renderSys)
	:mRenderSys(renderSys)
{}

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
	IL_PNG, /*IL_JPG, IL_JP2,*/ IL_BMP, IL_TGA, IL_DDS, IL_GIF, IL_HDR, IL_ICO
};
static ILenum DetectType(FILE* fd) {
	for (int i = 0; i < sizeof(CSupportILTypes) / sizeof(CSupportILTypes[0]); ++i) {
		if (ilIsValidF(CSupportILTypes[i], fd))
			return CSupportILTypes[i];
	}
	return IL_TYPE_UNKNOWN;
}
};

static int determineFace(int i, bool isDDS, bool isCube)
{
	int image = i;
	if (isDDS) {
		if (isCube) {
			if (4 == image) {
				image = 5;
			}
			else if (5 == image) {
				image = 4;
			}
		}
	}
	return(image);
}
const wchar_t* AnsiToUnicode(const char* szStr)
{
	int nLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szStr, -1, NULL, 0);
	if (nLen == 0) {
		return NULL;
	}
	wchar_t* pResult = new wchar_t[nLen];
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szStr, -1, pResult, nLen);
	return pResult;
}
ITexturePtr ResourceManager::DoCreateTexture(const std::string& imgFullpath, ResourceFormat format)
{
	ITexturePtr ret = nullptr;

	FILE* fd = fopen(imgFullpath.c_str(), "rb"); BOOST_ASSERT(fd);
	if (fd) {
		ITexturePtr texture = std::static_pointer_cast<ITexture>(mRenderSys.CreateResource(kDeviceResourceTexture));

		ILuint image = ilGenImage();
		ilBindImage(image);

		ILenum ilType = il_helper::DetectType(fd);
		//if (ilType != IL_TYPE_UNKNOWN && ilLoadImage(AnsiToUnicode(imgFullpath.c_str()))) {
		if (ilType != IL_TYPE_UNKNOWN && ilLoadF(ilType, fd)) {
			if (ilType == IL_DDS) {
				// DirectDraw Surfaces have their origin at upper left
				ilEnable(IL_ORIGIN_SET);
				ilOriginFunc(IL_ORIGIN_UPPER_LEFT);
			}
			else {
				ilDisable(IL_ORIGIN_SET);
			}

			ILuint width = ilGetInteger(IL_IMAGE_WIDTH), height = ilGetInteger(IL_IMAGE_HEIGHT);
			ILuint faceCount = ilGetInteger(IL_NUM_FACES) + 1, bpp = ilGetInteger(IL_IMAGE_BPP);

			format = (format != kFormatUnknown) ? format : kFormatR8G8B8A8UNorm;
			std::vector<unsigned char> bytes;
			std::vector<Data> datas(faceCount, {});
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
				int faceSize = width * height * sizeof(float) * 4;
				bytes.resize(faceSize * faceCount);
				for (int f = 0; f < faceCount; ++f) {
					//ilActiveFace(face);
					////ilCopyPixels(0, 0, 0, width, height, 1, IL_RGBA, IL_FLOAT, &bytes[faceSize * face]);
					////datas[face].Bytes = &bytes[faceSize * face];
					//datas[face].Bytes = ilGetData();
					//datas[face].Size = faceSize;

					unsigned int format1 = ilGetInteger(IL_IMAGE_FORMAT);

					int err = ilGetError();
					BOOST_ASSERT(IL_NO_ERROR == err);

					ilActiveImage(0);
					err = ilGetError();
					BOOST_ASSERT(IL_NO_ERROR == err);

					int face = determineFace(f, true, true);
					ilActiveFace(face);
					err = ilGetError();
					BOOST_ASSERT(IL_NO_ERROR == err);

					datas[0].Bytes = ilGetData();
					datas[0].Size = faceSize;
					if (f == 4)
						break;
				}
			} break;
			case kFormatR16G16B16A16Float: {
				for (int f = 0; f < faceCount; ++f) {
					int face = determineFace(f, true, true);
					ilActiveFace(face);
					ilActiveMipmap(1);
					datas[0].Bytes = ilGetData();
					datas[0].Size = 0;
					if (f == 3)
						break;
				}
			}break;
			default:
				BOOST_ASSERT(false);
				break;
			}

			faceCount = 1;
			constexpr int mipCount = 1;
			mRenderSys.LoadTexture(texture, format, Eigen::Vector4i(width, height, 0, faceCount), mipCount, &datas[0]);
			ret = texture;
		}

		ilDeleteImage(image);
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