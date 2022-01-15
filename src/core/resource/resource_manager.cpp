#include <boost/assert.hpp>
#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include "core/base/il_helper.h"
#include "core/base/d3d.h"
#include "core/base/input.h"
#include "core/base/debug.h"
#include "core/base/macros.h"
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
			std::shared_ptr<LoadResourcePkgTask> pkg_task = CreateInstance<LoadResourcePkgTask>(loadResCb);
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

	constexpr int CThreadPoolNumber = 8;
	mThreadPool = CreateInstance<ThreadPool>(CThreadPoolNumber, ilInit, ilShutDown);
	ilInit();
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

/********** Async Support **********/
void ResourceManager::UpdateForLoading() ThreadSafe
{
	ATOMIC_STATEMENT(mResDependGraphLock, this->mResDependencyGraph.GetTopNodes(mRDGTopNodes));
	for (auto& res : mRDGTopNodes) {
		BOOST_ASSERT(res->IsLoadingOrComplete());
		if (res->IsLoading()) {
			ResourceLoadTaskContext ctx;
			ATOMIC_STATEMENT(mLoadTaskCtxMapLock, ctx = this->mLoadTaskCtxByRes[res.get()]);
			if (ctx.Res) 
			{
				auto workExecute = std::move(ctx.WorkThreadJob->Execute);
				if (workExecute) {
					//TIME_PROFILE("\t\t'" + res->_Debug.GetResPath() + "' Execute");
					workExecute(ctx.Res, ctx.MainThreadJob);
				}
				BOOST_ASSERT(ctx.WorkThreadJob->Execute == nullptr);

				if (ctx.WorkThreadJob->Result.valid()
					&& ctx.WorkThreadJob->Result.wait_for(std::chrono::milliseconds(0)) != std::future_status::timeout) 
				{
					if (ctx.WorkThreadJob->Result.get()) 
					{
						auto mainExecute = std::move(ctx.MainThreadJob->Execute);
						if (mainExecute) {
							//TIME_PROFILE("\t\t'" + res->_Debug.GetResPath() + "' mainExecute");
							mainExecute(ctx.Res, nullptr);
							res->SetLoaded(ctx.MainThreadJob->Result.get());
							if (res->IsLoaded()) ctx.FireResourceLoaded();
						}
						else {
							res->SetLoaded(true);
							ctx.FireResourceLoaded();
						}
					#if defined MIR_RESOURCE_DEBUG
						DEBUG_LOG_DEBUG("\t\tres '" + res->_Debug.GetResPath() + "' loaded " + IF_AND_OR(res->IsLoaded(), "success", "fail"));
					#endif
					}
					else {
						res->SetLoaded(false);
					#if defined MIR_RESOURCE_DEBUG
						DEBUG_LOG_DEBUG("\t\tres '" + res->_Debug.GetResPath() + "' loaded fail");
					#endif
					}//ctx.WorkThreadJob.Result
				}//ctx.WorkThreadJob.Result.wait success
			}//ctx.Res
			else {
				res->SetLoaded(true);
				ctx.FireResourceLoaded();
			}
		}
	}
	for (auto res : mRDGTopNodes) {
		if (res->IsLoaded()) {
			ATOMIC_STATEMENT(mResDependGraphLock, this->mResDependencyGraph.RemoveTopNode(res));
			ATOMIC_STATEMENT(mLoadTaskCtxMapLock, this->mLoadTaskCtxByRes.erase(res.get()));
		}
		else if (res->IsLoadedFailed()) {
			ATOMIC_STATEMENT(mResDependGraphLock,
				this->mResDependencyGraph.RemoveConnectedGraphByTopNode(res, [](IResourcePtr node) {
				node->SetLoaded(false);
			})
			);
			ATOMIC_STATEMENT(mLoadTaskCtxMapLock, this->mLoadTaskCtxByRes.erase(res.get()));
		}
	}
}

void ResourceManager::AddResourceDependencyRecursive(IResourcePtr to) ThreadSafe
{
	if (to && !to->IsLoadComplete()) {
		std::vector<IResourcePtr> depends;
	#if 1
		to->GetLoadDependencies(depends);
		if (!depends.empty()) {
			to->SetLoading();
			ATOMIC_STATEMENT(mResDependGraphLock,
				this->mResDependencyGraph.AddLink(to, IF_AND_NULL(depends[0] && !depends[0]->IsLoaded(), depends[0]));
			for (auto& from : boost::make_iterator_range(depends.begin() + 1, depends.end()))
				if (from && !from->IsLoaded())
					this->mResDependencyGraph.AddLink(to, from);
			);
			for (auto& iter : depends)
				AddResourceDependencyRecursive(iter);
		}
		else {
			AddResourceDependency(to, nullptr);
		}
	#else
		size_t position = 0; to->GetLoadDependencies(depends);
		while (position < depends.size()) {
			size_t prev_position = position;
			position = depends.size();
			for (size_t i = prev_position; i < position; ++i) {
				if (depends[i]) 
					depends[i]->GetLoadDependencies(depends);
			}
		}
		if (!depends.empty()) {
			ATOMIC_STATEMENT(mResDependGraphLock,
				this->mResDependencyGraph.AddLink(to, IF_AND_NULL(depends[0] && !depends[0]->IsLoaded(), depends[0]));
				for (auto& from : boost::make_iterator_range(depends.begin() + 1, depends.end()))
					if (from && !from->IsLoaded()) 
						this->mResDependencyGraph.AddLink(to, from);
			);
		}
		else {
			AddResourceDependency(to, nullptr);
		}
	#endif
	}
}
void ResourceManager::AddResourceDependency(IResourcePtr to, IResourcePtr from) ThreadSafe
{
	if (to && !to->IsLoadComplete()) {
		to->SetLoading();
		ATOMIC_STATEMENT(mResDependGraphLock, 
			this->mResDependencyGraph.AddLink(to, IF_AND_NULL(from && !from->IsLoaded(), from)));
	}
}
void ResourceManager::AddLoadResourceJob(Launch launchMode, const LoadResourceCallback& loadResCb, IResourcePtr res, IResourcePtr dependRes) ThreadSafe
{
	AddResourceDependency(res, dependRes);
	DEBUG_SET_CALL(res, launchMode);
	ATOMIC_STATEMENT(mLoadTaskCtxMapLock, 
		this->mLoadTaskCtxByRes[res.get()].Init(launchMode, res, loadResCb, mThreadPool));
}
void ResourceManager::AddResourceLoadedObserver(IResourcePtr res, const ResourceLoadedCallback& resLoadedCB)
{
	ATOMIC_STATEMENT(mLoadTaskCtxMapLock, 
		this->mLoadTaskCtxByRes[res.get()].AddResourceLoadedCallback(resLoadedCB));
}

/********** Create Program **********/
inline boost::filesystem::path MakeShaderSourcePath(const std::string& name) {
	std::string filepath = "shader/" + name + ".hlsl";
	return boost::filesystem::system_complete(filepath);
}
inline boost::filesystem::path MakeShaderAsmPath(const std::string& name, const ShaderCompileDesc& desc, const std::string& platform) {
	std::string asmName = name;
	asmName += "_" + desc.EntryPoint;
	asmName += " " + desc.ShaderModel;
	for (const auto& macro : desc.Macros)
		asmName += " (" + macro.Name + "=" + macro.Definition + ")";
#if defined MIR_RESOURCE_DEBUG
	auto sourcePath = MakeShaderSourcePath(name);
	tm lwt;
	time_t time = boost::filesystem::last_write_time(sourcePath);
	{
		boost::filesystem::directory_iterator diter("shader/"), dend;
		for (; diter != dend; ++diter) {
			if (boost::filesystem::is_regular_file(*diter) && (*diter).path().extension() == ".cginc") {
				time_t htime = boost::filesystem::last_write_time((*diter).path());
				time = std::max(time, htime);
			}
		}
	}

	gmtime_s(&lwt, &time);
	boost::format fmt(" [%d-%d-%d %d.%d.%d]");
	fmt %lwt.tm_year %lwt.tm_mon %lwt.tm_mday;
	fmt %lwt.tm_hour %lwt.tm_min %lwt.tm_sec;
	asmName += fmt.str();
	boost::filesystem::create_directories("shader/asm/" + platform);
#endif
	asmName += ".cso";
	std::string filepath = "shader/asm/" + platform + "/" + asmName;
	return boost::filesystem::system_complete(filepath);
}
IProgramPtr ResourceManager::_LoadProgram(IProgramPtr program, LoadResourceJobPtr nextJob, 
	const std::string& name, ShaderCompileDesc vertexSCD, ShaderCompileDesc pixelSCD) ThreadSafe
{
#if defined MIR_TIME_DEBUG
	std::string msg = (boost::format("resMng._LoadProgram %1% %2% %3%") %name %vertexSCD.EntryPoint %pixelSCD.EntryPoint).str();
	for (auto& macro : vertexSCD.Macros)
		msg += (boost::format(" %1%=%2%") %macro.Name %macro.Definition).str();
	TIME_PROFILE(msg);
#endif

	IBlobDataPtr blobVS, blobPS;
	if (!vertexSCD.EntryPoint.empty()) {
		boost::filesystem::path vsAsmPath = MakeShaderAsmPath(name, vertexSCD, mRenderSys.GetPlatform());
		if (boost::filesystem::exists(vsAsmPath)) {
			blobVS = CreateInstance<BlobDataBytes>(input::ReadFile(vsAsmPath.string().c_str(), "rb"));
		}
		else {
			vertexSCD.SourcePath = MakeShaderSourcePath(name).string();
			std::vector<char> bytes = input::ReadFile(vertexSCD.SourcePath.c_str(), "rb");
			if (!bytes.empty()) {
				blobVS = this->mRenderSys.CompileShader(vertexSCD, Data::Make(bytes));
			#if defined MIR_RESOURCE_DEBUG
				input::WriteFile(vsAsmPath.string().c_str(), "wb", blobVS->GetBytes(), blobVS->GetSize());
			#endif
			}
		}
	}

	if (!pixelSCD.EntryPoint.empty()) {
		boost::filesystem::path psAsmPath = MakeShaderAsmPath(name, pixelSCD, mRenderSys.GetPlatform());
		if (boost::filesystem::exists(psAsmPath)) {
			blobPS = CreateInstance<BlobDataBytes>(input::ReadFile(psAsmPath.string().c_str(), "rb"));
		}
		else {
			pixelSCD.SourcePath = MakeShaderSourcePath(name).string();
			std::vector<char> bytes = input::ReadFile(pixelSCD.SourcePath.c_str(), "rb");
			if (!bytes.empty()) {
				blobPS = this->mRenderSys.CompileShader(pixelSCD, Data::Make(bytes));
			#if defined MIR_RESOURCE_DEBUG
				input::WriteFile(psAsmPath.string().c_str(), "wb", blobPS->GetBytes(), blobPS->GetSize());
			#endif
			}
		}
	}

	auto loadProgram = [blobVS, blobPS, this](IProgramPtr program)->IProgramPtr {
		std::vector<IShaderPtr> shaders;
		if (blobVS) shaders.push_back(this->mRenderSys.CreateShader(kShaderVertex, blobVS));
		if (blobPS) shaders.push_back(this->mRenderSys.CreateShader(kShaderPixel, blobPS));
		for (auto& it : shaders)
			it->SetLoaded();
		return this->mRenderSys.LoadProgram(program, shaders);
	};
	if (nextJob == nullptr) {
		return loadProgram(program);
	}
	else {
		nextJob->InitSync([loadProgram](IResourcePtr res, LoadResourceJobPtr nullJob)->bool {
			return nullptr != loadProgram(std::static_pointer_cast<IProgram>(res));
		});
		return program;
	}
}
IProgramPtr ResourceManager::CreateProgram(Launch launchMode,
	const std::string& name, ShaderCompileDesc vertexSCD, ShaderCompileDesc pixelSCD) ThreadSafe
{
	if (vertexSCD.ShaderModel.empty()) vertexSCD.ShaderModel = "vs_4_0";
	vertexSCD.Macros.push_back({ "SHADER_MODEL", "40000" });

	if (pixelSCD.ShaderModel.empty()) pixelSCD.ShaderModel = "ps_4_0";
	pixelSCD.Macros.push_back({ "SHADER_MODEL", "40000" });

	bool resNeedLoad = false;
	IProgramPtr program = nullptr;
	ProgramKey key{ name, vertexSCD, pixelSCD };
	ATOMIC_STATEMENT(mProgramMapLock,
		program = this->mProgramByKey[key];
		if (program == nullptr) {
			program = std::static_pointer_cast<IProgram>(mRenderSys.CreateResource(kDeviceResourceProgram));
			this->mProgramByKey[key] = program;
			DEBUG_SET_RES_PATH(program, (boost::format("name:%1%, vs:%2%, ps:%3%") % name %vertexSCD.EntryPoint %pixelSCD.EntryPoint).str());
			DEBUG_SET_CALL(program, launchMode);
			resNeedLoad = true;
		}
	);
	if (resNeedLoad) {
		if (launchMode == LaunchAsync) {
			AddLoadResourceJob(launchMode, [this, name, vertexSCD, pixelSCD](IResourcePtr res, LoadResourceJobPtr nextJob) {
				return nullptr != _LoadProgram(std::static_pointer_cast<IProgram>(res), nextJob, name, vertexSCD, pixelSCD);
			}, program, nullptr);
		}
		else {
			program->SetLoaded(nullptr != _LoadProgram(program, nullptr, name, vertexSCD, pixelSCD));
		}
	}
	return program;
}

/********** Create Texture **********/
ITexturePtr ResourceManager::_LoadTextureByFile(ITexturePtr texture, LoadResourceJobPtr nextJob, 
	const std::string& imgFullpath, ResourceFormat format, bool autoGenMipmap) ThreadSafe
{
	TIME_PROFILE((boost::format("resMng._LoadTextureByFile %1% %2% %3%") %imgFullpath %format %autoGenMipmap).str());
	ITexturePtr ret = nullptr;
	
	FILE* fd = fopen(imgFullpath.c_str(), "rb"); BOOST_ASSERT(fd);
	if (fd) {
		//this->mTexLock.lock();

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
				 
				auto& bytes = nextJob ? nextJob->Bytes : this->mTempBytes; 
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
		//this->mTexLock.unlock();
		fclose(fd);
	}//if fd
	return ret;
}
ITexturePtr ResourceManager::CreateTextureByFile(Launch launchMode,
	const std::string& filepath, ResourceFormat format, bool autoGenMipmap) ThreadSafe
{
	boost::filesystem::path fullpath = boost::filesystem::system_complete(filepath);
	std::string key = fullpath.string();

	bool resNeedLoad = false;
	ITexturePtr texture = nullptr;
	ATOMIC_STATEMENT(mTextureMapLock,
		texture = this->mTextureByPath[key];
		if (texture == nullptr) {
			texture = std::static_pointer_cast<ITexture>(this->mRenderSys.CreateResource(kDeviceResourceTexture));
			this->mTextureByPath[key] = texture;
			DEBUG_SET_RES_PATH(texture, (boost::format("path:%1%, fmt:%2%, autogen:%3%") % filepath %format %autoGenMipmap).str());
			DEBUG_SET_CALL(texture, launchMode);
			resNeedLoad = true;
		}
	);
	if (resNeedLoad) {
		if (launchMode == LaunchAsync) {
			AddLoadResourceJob(launchMode, [=](IResourcePtr res, LoadResourceJobPtr nextJob) {
				return nullptr != _LoadTextureByFile(std::static_pointer_cast<ITexture>(res), nextJob,
					key, format, autoGenMipmap);
			}, texture, nullptr);
		}
		else texture->SetLoaded(nullptr != _LoadTextureByFile(texture, nullptr, key, format, autoGenMipmap));
	}
	return texture;
}

/********** Create Material **********/
MaterialPtr ResourceManager::CreateMaterial(Launch launchMode, const MaterialLoadParam& matName) ThreadSafe ThreadSafe
{
	bool resNeedLoad = false;
	MaterialPtr material = nullptr;
	ATOMIC_STATEMENT(mMaterialMapLock, 
		material = this->mMaterialByName[matName];
		if (material == nullptr) {
			material = CreateInstance<Material>();
			this->mMaterialByName[matName] = material;
			DEBUG_SET_RES_PATH(material, (boost::format("name:%1% variant:%2%") %matName.ShaderName %matName.CalcVariantName()).str());
			DEBUG_SET_CALL(material, launchMode);
			resNeedLoad = true;
		}
	);
	if (resNeedLoad) {
		this->mMaterialFac.CreateMaterial(launchMode, *this, matName, material);
	}
	return material;
}
MaterialPtr ResourceManager::CloneMaterial(Launch launchMode, const Material& material) ThreadSafe
{
	return this->mMaterialFac.CloneMaterial(launchMode, *this, material);
}

/********** Create AiScene **********/
AiScenePtr ResourceManager::CreateAiScene(Launch launchMode, const std::string& assetPath, const std::string& redirectRes) ThreadSafe 
{
	bool resNeedLoad = false;
	AiScenePtr aiRes = nullptr;
	AiResourceKey key{ assetPath, redirectRes };
	ATOMIC_STATEMENT(mAiSceneMapLock, 
		aiRes = this->mAiSceneByKey[key];
		if (aiRes == nullptr) {
			aiRes = CreateInstance<AiScene>();
			this->mAiSceneByKey[key] = aiRes;
			DEBUG_SET_RES_PATH(aiRes, (boost::format("path:%1%, redirect:%2%") %assetPath %redirectRes).str());
			DEBUG_SET_CALL(aiRes, launchMode);
			resNeedLoad = true;
		}
	);
	if (resNeedLoad) {
		this->mAiResourceFac.CreateAiScene(launchMode, *this, assetPath, redirectRes, aiRes);
	}
	return aiRes;
}

}