#include <boost/assert.hpp>
#include <boost/asio.hpp>
#include <boost/format.hpp>
#include "core/base/il_helper.h"
#include "core/base/d3d.h"
#include "core/base/input.h"
#include "core/base/debug.h"
#include "core/base/thread.h"
#include "core/resource/resource_manager.h"
#include "core/resource/material_factory.h"
#include "core/resource/assimp_resource.h"

namespace mir {

/********** LoadResourceJob **********/
void LoadResourceJob::Init(Launch launchMode, LoadResourceCallback loadResCb)
{
	if (launchMode == LaunchAsync) {
		this->Execute = [loadResCb, this](IResourcePtr res, LoadResourceJobPtr nextJob) {
			typedef std::packaged_task<bool(IResourcePtr res, LoadResourceJobPtr nextJob)> LoadResourcePkgTask;
			std::shared_ptr<LoadResourcePkgTask> pkg_task = std::make_shared<LoadResourcePkgTask>(loadResCb);
			this->Result = std::move(pkg_task->get_future());
			auto pool = Pool.lock();
			if (pool) pool->Post([=]() { (*pkg_task)(res, nextJob); });
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
	mProgramMapLock = mTextureMapLock = mMaterialMapLock = mAiSceneMapLock = false;
	mLoadTaskCtxMapLock = mResDependGraphLock = false;

	ilInit();
	
	constexpr int CThreadPoolNumber = 8;
	mThreadPool = std::make_shared<ThreadPool>(CThreadPoolNumber);
}
ResourceManager::~ResourceManager()
{
	Dispose();
}
void ResourceManager::Dispose() ThreadSafe
{
	if (mThreadPool) {
		mThreadPool = nullptr;
		ilShutDown();
	}
}

void ResourceManager::AddResourceDependency(IResourcePtr to, IResourcePtr from) ThreadSafe
{
	if (to && !to->IsLoaded()) {
		ATOMIC_STATEMENT(mResDependGraphLock, 
			this->mResDependencyGraph.AddLink(to, from && !from->IsLoaded() ? from : nullptr));
	}
}

void ResourceManager::AddLoadResourceJob(Launch launchMode, const LoadResourceCallback& loadResCb, IResourcePtr res, IResourcePtr dependRes) ThreadSafe
{
	AddResourceDependency(res, dependRes);
	res->SetPrepared();
	DEBUG_SET_CALL(res, launchMode);
	ATOMIC_STATEMENT(mLoadTaskCtxMapLock, 
		this->mLoadTaskCtxByRes[res.get()].Init(launchMode, res, loadResCb, mThreadPool));
}

void ResourceManager::AddResourceLoadedObserver(IResourcePtr res, const ResourceLoadedCallback& resLoadedCB)
{
	ATOMIC_STATEMENT(mLoadTaskCtxMapLock, 
		this->mLoadTaskCtxByRes[res.get()].AddResourceLoadedCallback(resLoadedCB));
}

void ResourceManager::UpdateForLoading() ThreadSafe
{
	ATOMIC_STATEMENT(mResDependGraphLock, const auto& topNodes = this->mResDependencyGraph.GetTopNodes(mRDGTopNodes));
	for (auto& res : topNodes) {
		if (res->IsPreparedNeedLoading()) {
			ATOMIC_STATEMENT(mLoadTaskCtxMapLock, ResourceLoadTaskContext ctx = this->mLoadTaskCtxByRes[res.get()]);
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
							if (res->IsLoaded()) ctx.FireResourceLoaded();
						}
						else {
							res->SetLoaded(true);
							ctx.FireResourceLoaded();
						}
					}
					else {
						res->SetLoaded(false);
					}
				}
			}
			else {
				res->SetLoaded(true);
				ctx.FireResourceLoaded();
			}
		}
	}
	for (auto res : topNodes) {
		if (res->IsLoaded()) {
			ATOMIC_STATEMENT(mResDependGraphLock, this->mResDependencyGraph.RemoveTopNode(res));
			ATOMIC_STATEMENT(mLoadTaskCtxMapLock, this->mLoadTaskCtxByRes.erase(res.get()));
		}
		else if (res->IsLoadedFailed()) {
			ATOMIC_STATEMENT(mResDependGraphLock, 
				this->mResDependencyGraph.RemoveConnectedGraphByTopNode(res, [](IResourcePtr node) {
				node->SetLoaded(false);
			}));
			ATOMIC_STATEMENT(mLoadTaskCtxMapLock, this->mLoadTaskCtxByRes.erase(res.get()));
		}
	}
}

IProgramPtr ResourceManager::_LoadProgram(IProgramPtr program, LoadResourceJobPtr nextJob, 
	const std::string& name, const std::string& vsEntry, const std::string& psEntry) ThreadSafe
{
	std::string vsEntryOrVS = !vsEntry.empty() ? vsEntry : "VS";
	boost::filesystem::path vsAsmPath = "shader/d3d11/" + name + "_" + vsEntryOrVS + ".cso";
	vsAsmPath = boost::filesystem::system_complete(vsAsmPath);

	if (boost::filesystem::exists(vsAsmPath)) {

		ShaderCompileDesc descVS = {
			{{"SHADER_MODEL", "40000"}},
			vsEntry, "vs_4_0", vsAsmPath.string()
		};
		auto blobVS = std::make_shared<BlobDataBytes>(input::ReadFile(vsAsmPath.string().c_str(), "rb"));

		std::string psEntryOrPS = !psEntry.empty() ? psEntry : "PS";
		boost::filesystem::path psAsmPath = "shader/d3d11/" + name + "_" + psEntryOrPS + ".cso";
		psAsmPath = boost::filesystem::system_complete(psAsmPath);
		ShaderCompileDesc descPS = {
			{{"SHADER_MODEL", "40000"}},
			psEntry, "ps_4_0", psAsmPath.string()
		};
		auto blobPS = std::make_shared<BlobDataBytes>(input::ReadFile(psAsmPath.string().c_str(), "rb"));

		if (nextJob == nullptr) {
			std::vector<IShaderPtr> shaders;
			shaders.push_back(this->mRenderSys.CreateShader(kShaderVertex, descVS, blobVS));
			shaders.push_back(this->mRenderSys.CreateShader(kShaderPixel, descPS, blobPS));
			for (auto& it : shaders)
				it->SetLoaded();
			return this->mRenderSys.LoadProgram(program, shaders);
		}
		else {
			nextJob->InitSync([=](IResourcePtr res, LoadResourceJobPtr nullJob)->bool {
				std::vector<IShaderPtr> shaders;
				shaders.push_back(this->mRenderSys.CreateShader(kShaderVertex, descVS, blobVS));
				shaders.push_back(this->mRenderSys.CreateShader(kShaderPixel, descPS, blobPS));
				for (auto& it : shaders)
					it->SetLoaded();
				return nullptr != this->mRenderSys.LoadProgram(program, shaders);
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
			IBlobDataPtr blobVS = this->mRenderSys.CompileShader(descVS, Data::Make(&bytes[0], bytes.size()));

			ShaderCompileDesc descPS = {
				{{"SHADER_MODEL", "40000"}},
				psEntry, "ps_4_0", vsPsPath
			};
			IBlobDataPtr blobPS = this->mRenderSys.CompileShader(descPS, Data::Make(&bytes[0], bytes.size()));

			if (nextJob == nullptr) {
				std::vector<IShaderPtr> shaders;
				shaders.push_back(this->mRenderSys.CreateShader(kShaderVertex, descVS, blobVS));
				shaders.push_back(this->mRenderSys.CreateShader(kShaderPixel, descPS, blobPS));
				for (auto& it : shaders)
					it->SetLoaded();
				return this->mRenderSys.LoadProgram(program, shaders);
			}
			else {
				nextJob->InitSync([=](IResourcePtr res, LoadResourceJobPtr nullJob)->bool {
					std::vector<IShaderPtr> shaders;
					shaders.push_back(this->mRenderSys.CreateShader(kShaderVertex, descVS, blobVS));
					shaders.push_back(this->mRenderSys.CreateShader(kShaderPixel, descPS, blobPS));
					for (auto& it : shaders)
						it->SetLoaded();
					return nullptr != this->mRenderSys.LoadProgram(program, shaders);
				});
				return program;
			}
		}
	}
	return nullptr;
}
IProgramPtr ResourceManager::CreateProgram(Launch launchMode, 
	const std::string& name, const std::string& vsEntry, const std::string& psEntry) ThreadSafe
{
	IProgramPtr program = nullptr;
	ProgramKey key{ name, vsEntry, psEntry };
	ATOMIC_STATEMENT(mProgramMapLock, auto findProg = this->mProgramByKey.find(key));
	if (findProg == this->mProgramByKey.end()) {
		program = std::static_pointer_cast<IProgram>(mRenderSys.CreateResource(kDeviceResourceProgram));
		ATOMIC_STATEMENT(mProgramMapLock, this->mProgramByKey.insert(std::make_pair(key, program)));
	#if defined MIR_RESOURCE_DEBUG
		DEBUG_SET_RES_PATH(program, (boost::format("name:%1%, vs:%2%, ps:%3%") % name %vsEntry %psEntry).str());
		DEBUG_SET_CALL(program, launchMode);
	#endif

		if (launchMode == LaunchAsync) {
			AddLoadResourceJob(launchMode, [=](IResourcePtr res, LoadResourceJobPtr nextJob) {
				return nullptr != _LoadProgram(std::static_pointer_cast<IProgram>(res), nextJob, name, vsEntry, psEntry);
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
	const std::string& imgFullpath, ResourceFormat format, bool autoGenMipmap) ThreadSafe
{
	ITexturePtr ret = nullptr;

	FILE* fd = fopen(imgFullpath.c_str(), "rb"); BOOST_ASSERT(fd);
	if (fd) {
		if (nextJob) this->mTexLock.lock();

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
				 
				auto& bytes = nextJob ? nextJob->Bytes : this->mTempBytes; 
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
		if (nextJob) this->mTexLock.unlock();
		fclose(fd);
	}//if fd
	return ret;
}
ITexturePtr ResourceManager::CreateTextureByFile(Launch launchMode, 
	const std::string& filepath, ResourceFormat format, bool autoGenMipmap) ThreadSafe
{
	boost::filesystem::path fullpath = boost::filesystem::system_complete(filepath);
	std::string imgFullpath = fullpath.string();

	ITexturePtr texture = nullptr;
	ATOMIC_STATEMENT(mTextureMapLock, auto findIter = this->mTextureByPath.find(imgFullpath));
	if (findIter == this->mTextureByPath.end()) {
		texture = std::static_pointer_cast<ITexture>(this->mRenderSys.CreateResource(kDeviceResourceTexture));
		ATOMIC_STATEMENT(mTextureMapLock, this->mTextureByPath.insert(std::make_pair(imgFullpath, texture)));
	
	#if defined MIR_RESOURCE_DEBUG
		DEBUG_SET_RES_PATH(texture, (boost::format("path:%1%, fmt:%2%, autogen:%3%") % filepath %format %autoGenMipmap).str());
		DEBUG_SET_CALL(texture, launchMode);
	#endif

		if (launchMode == LaunchAsync) {
			AddLoadResourceJob(launchMode, [=](IResourcePtr res, LoadResourceJobPtr nextJob) {
				return nullptr != _LoadTextureByFile(std::static_pointer_cast<ITexture>(res), nextJob,
					imgFullpath, format, autoGenMipmap);
			}, texture, nullptr);
		}
		else texture->SetLoaded(nullptr != _LoadTextureByFile(texture, nullptr, imgFullpath, format, autoGenMipmap));
	}
	else {
		texture = findIter->second;
	}
	return texture;
}

MaterialPtr ResourceManager::CreateMaterial(Launch launchMode, const std::string& matName) ThreadSafe ThreadSafe
{
	MaterialPtr material = nullptr;
	ATOMIC_STATEMENT(mMaterialMapLock, auto findIter = this->mMaterialByName.find(matName));
	if (findIter == this->mMaterialByName.end()) {
		material = this->mMaterialFac.CreateMaterial(launchMode, *this, matName);
		ATOMIC_STATEMENT(mMaterialMapLock, this->mMaterialByName.insert(std::make_pair(matName, material)));
	#if defined MIR_RESOURCE_DEBUG
		DEBUG_SET_RES_PATH(material, (boost::format("name:%1%") %matName).str());
		DEBUG_SET_CALL(material, launchMode);
	#endif
	}
	else {
		material = findIter->second;
	}
	return material;
}
MaterialPtr ResourceManager::CloneMaterial(Launch launchMode, const Material& material) ThreadSafe
{
	return this->mMaterialFac.CloneMaterial(launchMode, *this, material);
}

AiScenePtr ResourceManager::CreateAiScene(Launch launchMode, const std::string& assetPath, const std::string& redirectRes) ThreadSafe 
{
	AiScenePtr aiRes = nullptr;
	AiResourceKey key{ assetPath, redirectRes };
	ATOMIC_STATEMENT(mAiSceneMapLock, auto findIter = this->mAiSceneByKey.find(key));
	if (findIter == this->mAiSceneByKey.end()) {
		aiRes = this->mAiResourceFac.CreateAiScene(launchMode, *this, assetPath, redirectRes);
		ATOMIC_STATEMENT(mAiSceneMapLock, this->mAiSceneByKey.insert(std::make_pair(key, aiRes)));

	#if defined MIR_RESOURCE_DEBUG
		DEBUG_SET_RES_PATH(aiRes, (boost::format("path:%1%, redirect:%2%") %assetPath %redirectRes).str());
		DEBUG_SET_CALL(aiRes, launchMode);
	#endif
	}
	else {
		aiRes = findIter->second;
	}
	return aiRes;
}

}