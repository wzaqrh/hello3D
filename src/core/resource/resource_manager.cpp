#include <boost/assert.hpp>
#include <boost/asio.hpp>
#include "core/base/il_helper.h"
#include "core/base/d3d.h"
#include "core/base/input.h"
#include "core/resource/resource_manager.h"
#include "core/rendersys/interface_type.h"
#include "core/rendersys/render_system.h"
#include "core/resource/material_factory.h"
#include "core/resource/assimp_resource.h"

namespace mir {

struct ThreadPoolImp {
	ThreadPoolImp() :Pool(4) {}

	boost::asio::thread_pool Pool;
};

/********** LoadResourceJob **********/
void LoadResourceJob::Init(Launch launchMode, LoadResourceCallback loadResCb)
{
	if (launchMode == Launch::Async) {
		this->Execute = [loadResCb, this](IResourcePtr res, LoadResourceJobPtr nextJob) {
			this->Result = std::move(std::async(loadResCb, res, nextJob));
		};
	}
	else {
		this->Execute = [loadResCb, this](IResourcePtr res, LoadResourceJobPtr nextJob) {
			std::packaged_task<bool(IResourcePtr, LoadResourceJobPtr)> pkg_task(loadResCb);
			this->Result = std::move(pkg_task.get_future());
			pkg_task(res, nextJob);
		};
	}
}

/********** ResourceManager **********/
ResourceManager::ResourceManager(RenderSystem& renderSys, MaterialFactory& materialFac, AiResourceFactory& aiResFac)
	: mRenderSys(renderSys)
	, mMaterialFac(materialFac)
	, mAiResourceFac(aiResFac)
{
	ilInit();
	mThreadPoolImp = std::make_shared<ThreadPoolImp>();
}
ResourceManager::~ResourceManager()
{
	ilShutDown();
}

void ResourceManager::AddResourceDependency(IResourcePtr to, IResourcePtr from)
{
	if (to && !to->IsLoaded()) 
		mResDependencyGraph.AddLink(to, from && !from->IsLoaded() ? from : nullptr);
}

void ResourceManager::AddLoadResourceJob(Launch launchMode, const LoadResourceCallback& loadResCb, IResourcePtr res, IResourcePtr dependRes)
{
	AddResourceDependency(res, dependRes);
	res->SetPrepared();
	mLoadTaskCtxByRes[res].Init(launchMode, res, loadResCb);
}

void ResourceManager::UpdateForLoading()
{
	const std::vector<IResourcePtr>& topNodes = mResDependencyGraph.GetTopNodes();
	for (auto& res : topNodes) {
		if (res->IsPreparedNeedLoading()) {
			ResourceLoadTaskContext& ctx = mLoadTaskCtxByRes[res];
			if (ctx.Res) {
				auto workExecute = std::move(ctx.WorkThreadJob->Execute);
				if (workExecute) workExecute(ctx.Res, ctx.MainThreadJob);
				BOOST_ASSERT(ctx.WorkThreadJob->Execute == nullptr);

				if (ctx.WorkThreadJob->Result.valid() 
					&& ctx.WorkThreadJob->Result.wait_for(std::chrono::milliseconds(0)) != std::future_status::timeout) 
				{
					if (ctx.WorkThreadJob->Result.get()) {
						auto mainExecute = std::move(ctx.MainThreadJob->Execute);
						if (mainExecute) {
							mainExecute(ctx.Res, nullptr);
							res->SetLoaded(ctx.MainThreadJob->Result.get());
						}
						else {
							res->SetLoaded(true);
						}
					}
					else {
						res->SetLoaded(false);
					}
				}
			}
			else {
				res->SetLoaded(true);
			}
		}
	}
	for (auto res : topNodes) {
		if (res->IsLoaded()) {
			mResDependencyGraph.RemoveTopNode(res);
			mLoadTaskCtxByRes.erase(res);
		}
		else if (res->IsLoadedFailed()) {
			mResDependencyGraph.RemoveConnectedGraphByTopNode(res, [](IResourcePtr node) {
				node->SetLoaded(false);
			});
			mLoadTaskCtxByRes.erase(res);
		}
	}
}

IProgramPtr ResourceManager::_LoadProgram(IProgramPtr program, LoadResourceJobPtr nextJob, 
	const std::string& name, const std::string& vsEntry, const std::string& psEntry)
{
	std::string vsEntryOrVS = !vsEntry.empty() ? vsEntry : "VS";
	boost::filesystem::path vsAsmPath = "shader/d3d11/" + name + "_" + vsEntryOrVS + ".cso";
	vsAsmPath = boost::filesystem::system_complete(vsAsmPath);

	if (boost::filesystem::exists(vsAsmPath)) {

		ShaderCompileDesc descVS = {
			{{"SHADER_MODEL", "40000"}},
			vsEntry, "vs_4_0", vsAsmPath.string()
		};
		auto blobVS = std::make_shared<BlobDataStandard>(input::ReadFile(vsAsmPath.string().c_str(), "rb"));

		std::string psEntryOrPS = !psEntry.empty() ? psEntry : "PS";
		boost::filesystem::path psAsmPath = "shader/d3d11/" + name + "_" + psEntryOrPS + ".cso";
		psAsmPath = boost::filesystem::system_complete(psAsmPath);
		ShaderCompileDesc descPS = {
			{{"SHADER_MODEL", "40000"}},
			psEntry, "ps_4_0", psAsmPath.string()
		};
		auto blobPS = std::make_shared<BlobDataStandard>(input::ReadFile(psAsmPath.string().c_str(), "rb"));

		if (nextJob == nullptr) {
			std::vector<IShaderPtr> shaders;
			shaders.push_back(mRenderSys.CreateShader(kShaderVertex, descVS, blobVS));
			shaders.push_back(mRenderSys.CreateShader(kShaderPixel, descPS, blobPS));
			for (auto& it : shaders)
				it->SetLoaded();
			return mRenderSys.LoadProgram(program, shaders);
		}
		else {
			nextJob->InitSync([=](IResourcePtr res, LoadResourceJobPtr nullJob)->bool {
				std::vector<IShaderPtr> shaders;
				shaders.push_back(mRenderSys.CreateShader(kShaderVertex, descVS, blobVS));
				shaders.push_back(mRenderSys.CreateShader(kShaderPixel, descPS, blobPS));
				for (auto& it : shaders)
					it->SetLoaded();
				return nullptr != mRenderSys.LoadProgram(program, shaders);
			});
			return program;
		}
	}
	else {
		std::string vsPsPath = boost::filesystem::system_complete("shader/" + name + ".fx").string();
		std::vector<char> bytes = input::ReadFile(vsPsPath.c_str(), "rb");
		if (!bytes.empty()) {
			ShaderCompileDesc descVS = {
				{{"SHADER_MODEL", "40000"}},
				vsEntry, "vs_4_0", vsPsPath
			};
			IBlobDataPtr blobVS = mRenderSys.CompileShader(descVS, Data::Make(&bytes[0], bytes.size()));

			ShaderCompileDesc descPS = {
				{{"SHADER_MODEL", "40000"}},
				psEntry, "ps_4_0", vsPsPath
			};
			IBlobDataPtr blobPS = mRenderSys.CompileShader(descPS, Data::Make(&bytes[0], bytes.size()));

			if (nextJob == nullptr) {
				std::vector<IShaderPtr> shaders;
				shaders.push_back(mRenderSys.CreateShader(kShaderVertex, descVS, blobVS));
				shaders.push_back(mRenderSys.CreateShader(kShaderPixel, descPS, blobPS));
				for (auto& it : shaders)
					it->SetLoaded();
				return mRenderSys.LoadProgram(program, shaders);
			}
			else {
				nextJob->InitSync([=](IResourcePtr res, LoadResourceJobPtr nullJob)->bool {
					std::vector<IShaderPtr> shaders;
					shaders.push_back(mRenderSys.CreateShader(kShaderVertex, descVS, blobVS));
					shaders.push_back(mRenderSys.CreateShader(kShaderPixel, descPS, blobPS));
					for (auto& it : shaders)
						it->SetLoaded();
					return nullptr != mRenderSys.LoadProgram(program, shaders);
				});
				return program;
			}
		}
	}
	return nullptr;
}
IProgramPtr ResourceManager::CreateProgram(Launch launchMode, const std::string& name, const std::string& vsEntry, const std::string& psEntry)
{
	IProgramPtr program = nullptr;
	ProgramKey key{ name, vsEntry, psEntry };
	auto findProg = mProgramByKey.find(key);
	if (findProg == mProgramByKey.end()) {
		program = std::static_pointer_cast<IProgram>(mRenderSys.CreateResource(kDeviceResourceProgram));
		mProgramByKey.insert(std::make_pair(key, program));

		if (launchMode == Launch::Async) {
			AddLoadResourceJobAsync([=](IResourcePtr res, LoadResourceJobPtr nextJob) {
				return nullptr != _LoadProgram(program, nextJob, name, vsEntry, psEntry);
			}, program, nullptr);
		}
		else {
			program = _LoadProgram(program, nullptr, name, vsEntry, psEntry);
			program->SetLoaded();
		}
	}
	else {
		program = findProg->second;
	}
	return program;
}

ITexturePtr ResourceManager::_LoadTextureByFile(ITexturePtr texture, LoadResourceJobPtr nextJob, 
	const std::string& imgFullpath, ResourceFormat format, bool autoGenMipmap)
{
	ITexturePtr ret = nullptr;

	FILE* fd = fopen(imgFullpath.c_str(), "rb"); BOOST_ASSERT(fd);
	if (fd) {
		if (nextJob) mTexLock.lock();

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
				 
				auto& bytes = nextJob ? nextJob->bytes : mTempBytes; 
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
								if (nextJob) {
									size_t sub_res_size = mip_height * mip_width * bpp;
									BOOST_ASSERT(bytes_position + sub_res_size <= bytes.size());
									ilCopyPixels(0, 0, 0,
										mip_width, mip_height, 1,
										ilFormat0, ilFormat1, &bytes[bytes_position]);

									dataFM.Bytes = &bytes[bytes_position];
									dataFM.Size = mip_width * bpp;
									bytes_position += sub_res_size;
								}
								else {
									dataFM.Bytes = ilGetData();
									dataFM.Size = mip_width * bpp;
								}
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
					if (nextJob == nullptr) {
						ret = mRenderSys.LoadTexture(texture, format, 
							Eigen::Vector4i(width, height, 0, faceCount), mipCount, &datas[0]);
					}
					else {
						nextJob->InitSync([=](IResourcePtr res, LoadResourceJobPtr nullJob)->bool {
							return nullptr != mRenderSys.LoadTexture(texture, format, 
								Eigen::Vector4i(width, height, 0, faceCount), mipCount, &datas[0]);
						});
						ret = texture;
					}
				}
			}
		}
		else {
			BOOST_ASSERT(il_helper::CheckLastError());
		}

		ilDeleteImage(imageId);
		if (nextJob) mTexLock.unlock();
		fclose(fd);
	}//if fd
	return ret;
}
ITexturePtr ResourceManager::CreateTextureByFile(Launch launchMode, const std::string& filepath, ResourceFormat format, bool autoGenMipmap)
{
	boost::filesystem::path fullpath = boost::filesystem::system_complete(filepath);
	std::string imgFullpath = fullpath.string();

	ITexturePtr texture = nullptr;
	auto findTex = mTextureByPath.find(imgFullpath);
	if (findTex == mTextureByPath.end()) {
		texture = std::static_pointer_cast<ITexture>(mRenderSys.CreateResource(kDeviceResourceTexture));
		mTextureByPath.insert(std::make_pair(imgFullpath, texture));

		if (launchMode == Launch::Async) {
			AddLoadResourceJobAsync([=](IResourcePtr res, LoadResourceJobPtr nextJob) {
				return nullptr != _LoadTextureByFile(std::static_pointer_cast<ITexture>(res), nextJob,
					imgFullpath, format, autoGenMipmap);
			}, texture, nullptr);
		}
		else texture->SetLoaded(nullptr != _LoadTextureByFile(texture, nullptr, imgFullpath, format, autoGenMipmap));
	}
	else {
		texture = findTex->second;
	}
	return texture;
}

MaterialPtr ResourceManager::CreateMaterial(Launch launchMode, const std::string& matName, bool sharedUse/*readonly*/)
{
	if (mMaterialByName.find(matName) == mMaterialByName.end())
		mMaterialByName.insert(std::make_pair(matName, mMaterialFac.CreateMaterial(launchMode, *this, matName)));

	MaterialPtr material = sharedUse ? mMaterialByName[matName] : CloneMaterial(launchMode, *mMaterialByName[matName]);
	return material;
}

MaterialPtr ResourceManager::CloneMaterial(Launch launchMode, const Material& material)
{
	return mMaterialFac.CloneMaterial(launchMode, *this, material);
}

AiScenePtr ResourceManager::CreateAiScene(Launch launchMode, const Material& material, 
	const std::string& assetPath, const std::string& redirectRes)
{
	AiScenePtr aiRes = nullptr;
	AiResourceKey key{ assetPath, redirectRes };
	auto findAiRes = mAiSceneByKey.find(key);
	if (findAiRes == mAiSceneByKey.end()) {
		aiRes = mAiResourceFac.CreateAiScene(launchMode, *this, material, assetPath, redirectRes);
		mAiSceneByKey.insert(std::make_pair(key, aiRes));
	}
	else {
		aiRes = findAiRes->second;
	}
	return aiRes;
}

}