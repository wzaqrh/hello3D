#include <boost/assert.hpp>
#include "core/base/il_helper.h"
#include "core/base/d3d.h"
#include "core/base/input.h"
#include "core/rendersys/resource_manager.h"
#include "core/rendersys/interface_type.h"
#include "core/rendersys/render_system.h"
#include "core/rendersys/material_factory.h"

namespace mir {

ResourceManager::ResourceManager(RenderSystem& renderSys, MaterialFactory& materialFac)
	:mRenderSys(renderSys)
	,mMaterialFac(materialFac)
{
	ilInit();
}
ResourceManager::~ResourceManager()
{
	ilShutDown();
}

void ResourceManager::UpdateForLoading()
{
	const std::vector<IResourcePtr>& topNodes = mResDependencyGraph.TopNodes();
	for (auto& res : topNodes) {
		if (res->IsPreparedNeedLoading()) {
			auto task = mLoadTaskByRes[res];
			if (task) task(res);
			else res->SetLoaded();
		}
	}
	for (auto res : topNodes) {
		if (res->IsLoaded()) {
			mResDependencyGraph.RemoveNode(res);
			mLoadTaskByRes.erase(res);
		}
	}
}

void ResourceManager::AddResourceDependency(IResourcePtr to, IResourcePtr from)
{
	if (to && !to->IsLoaded()) 
		mResDependencyGraph.AddLink(to, from && !from->IsLoaded() ? from : nullptr);
}

IProgramPtr ResourceManager::_LoadProgram(IProgramPtr program, const std::string& name, const std::string& vsEntry, const std::string& psEntry)
{
	if (boost::filesystem::path(name).extension().empty()) {
		std::string fullname = boost::filesystem::system_complete("shader/d3d11/" + name).string();
		//auto program = mRenderSys.CreateResource(kDeviceResourceProgram);
		std::vector<IShaderPtr> shaders;
		{
			ShaderCompileDesc desc;
			desc.EntryPoint = vsEntry;
			desc.ShaderModel = "vs_4_0";
			desc.SourcePath = fullname;

			std::string vsEntryOrVS = !vsEntry.empty() ? vsEntry : "VS";
			std::string vsFullPath = name + "_" + vsEntryOrVS + ".cso";
			std::vector<char> buffer = input::ReadFile(vsFullPath.c_str(), "rb");
			auto blob = std::make_shared<BlobDataStandard>(buffer);
			
			shaders.push_back(mRenderSys.CreateShader(kShaderVertex, desc, blob));
		}
		{
			ShaderCompileDesc desc;
			desc.EntryPoint = psEntry;
			desc.ShaderModel = "ps_4_0";
			desc.SourcePath = fullname;

			std::string psEntryOrPS = !psEntry.empty() ? psEntry : "PS";
			std::string psFullName = name + "_" + psEntryOrPS + ".cso";
			std::vector<char> buffer = input::ReadFile(psFullName.c_str(), "rb");
			auto blob = std::make_shared<BlobDataStandard>(buffer);

			shaders.push_back(mRenderSys.CreateShader(kShaderPixel, desc, blob));
		}
		for (auto& it : shaders)
			it->SetLoaded();
		return mRenderSys.LoadProgram(program, shaders);
	}
	else {
		std::string fullname = boost::filesystem::system_complete("shader/" + name).string();
		std::vector<char> bytes = input::ReadFile(fullname.c_str(), "rb");
		if (!bytes.empty()) {
			//auto program = mRenderSys.CreateResource(kDeviceResourceProgram);
			std::vector<IShaderPtr> shaders;
			{
				ShaderCompileDesc desc = {
					{{"SHADER_MODEL", "40000"}},
					vsEntry, "vs_4_0", fullname
				};
				IBlobDataPtr blob = mRenderSys.CompileShader(desc, Data::Make(&bytes[0], bytes.size()));

				shaders.push_back(mRenderSys.CreateShader(kShaderVertex, desc, blob));
			}
			{
				ShaderCompileDesc desc = {
					{{"SHADER_MODEL", "40000"}},
					psEntry, "ps_4_0", fullname
				};
				IBlobDataPtr blob = mRenderSys.CompileShader(desc, Data::Make(&bytes[0], bytes.size()));

				shaders.push_back(mRenderSys.CreateShader(kShaderPixel, desc, blob));
			}
			for (auto& it : shaders)
				it->SetLoaded();
			return mRenderSys.LoadProgram(program, shaders);
		}
	}
	return nullptr;
}
IProgramPtr ResourceManager::_CreateProgram(bool async, const std::string& name, const std::string& vsEntry, const std::string& psEntry)
{
	IProgramPtr program = nullptr;
	ProgramKey key{ name, vsEntry, psEntry };
	auto findProg = mProgramByKey.find(key);
	if (findProg == mProgramByKey.end()) {
		program = std::static_pointer_cast<IProgram>(mRenderSys.CreateResource(kDeviceResourceProgram));
		mProgramByKey.insert(std::make_pair(key, program));

		if (async) {
			program->SetPrepared();
			AddResourceDependency(program, nullptr);
			mLoadTaskByRes[program] = [=](IResourcePtr res) {
				_LoadProgram(program, name, vsEntry, psEntry);
				program->SetLoaded();
			};
		}
		else {
			program = _LoadProgram(program, name, vsEntry, psEntry);
			program->SetLoaded();
		}
	}
	else {
		program = findProg->second;
	}
	return program;
}

ITexturePtr ResourceManager::_LoadTextureByFile(ITexturePtr texture, const std::string& imgFullpath, ResourceFormat format, bool autoGenMipmap)
{
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
			ilSetInteger(IL_KEEP_DXTC_DATA, IL_TRUE); 
			break;
		default: 
			break;
		}
		BOOST_ASSERT(il_helper::CheckLastError());
		
		if (ilFileType != IL_TYPE_UNKNOWN && ilLoadF(ilFileType, fd)) {
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
				std::vector<Data> datas(mipCount * faceCount, {});

				if (convertFormat != kFormatUnknown)
					bpp = d3d::BytePerPixel(static_cast<DXGI_FORMAT>(convertFormat));
				int faceSize = width * height * bpp;
				auto& bytes = mTempBytes;
				bytes.resize(faceSize * mipCount * faceCount);
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
						if (convertFormat == kFormatUnknown) {
							//直接使用
							ILuint compressMode = ilGetInteger(IL_COMPRESS_MODE);

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
								dataFM.Bytes = ilGetData();
								dataFM.Size = mip_width * bpp;
							}
						}
						else {
							//转换格式
							size_t sub_res_size = mip_width * mip_height * bpp;
							
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
					mRenderSys.LoadTexture(texture, format, Eigen::Vector4i(width, height, 0, faceCount), mipCount, &datas[0]);
					ret = texture;
				}
			}
		}
		else {
			BOOST_ASSERT(il_helper::CheckLastError());
		}

		ilDeleteImage(imageId);
		fclose(fd);
	}
	return ret;
}
ITexturePtr ResourceManager::_CreateTextureByFile(bool async, const std::string& filepath, ResourceFormat format, bool autoGenMipmap)
{
	boost::filesystem::path fullpath = boost::filesystem::system_complete(filepath);
	std::string imgFullpath = fullpath.string();

	ITexturePtr texture = nullptr;
	auto findTex = mTexByPath.find(imgFullpath);
	if (findTex == mTexByPath.end()) {
		texture = std::static_pointer_cast<ITexture>(mRenderSys.CreateResource(kDeviceResourceTexture));
		mTexByPath.insert(std::make_pair(imgFullpath, texture));

		if (async) {
			texture->SetPrepared();
			AddResourceDependency(texture, nullptr);
			mLoadTaskByRes[texture] = [=](IResourcePtr res) {
				_LoadTextureByFile(texture, imgFullpath, format, autoGenMipmap);
				texture->SetLoaded();
			};
		}
		else {
			texture = _LoadTextureByFile(texture, imgFullpath, format, autoGenMipmap);
			texture->SetLoaded();
		}
	}
	else {
		texture = findTex->second;
	}
	return texture;
}

MaterialPtr ResourceManager::CreateMaterial(const std::string& matName, bool sharedUse/*readonly*/)
{
	if (mMaterials.find(matName) == mMaterials.end())
		mMaterials.insert(std::make_pair(matName, mMaterialFac.CreateMaterial(*this, matName)));

	MaterialPtr material = sharedUse ? mMaterials[matName] : mMaterials[matName]->Clone(*this);
	return material;
}

}