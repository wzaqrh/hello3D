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

ResourceManager::ResourceManager(RenderSystem& renderSys, res::MaterialFactory& materialFac, res::AiResourceFactory& aiResFac, std::shared_ptr<cppcoro::io_service> ioService)
	: mRenderSys(renderSys)
	, mMaterialFac(materialFac)
	, mAiResourceFac(aiResFac)
{
	mMainThreadId = std::this_thread::get_id();
	constexpr int CThreadPoolNumber = 8;
	mThreadPool = CreateInstance<cppcoro::static_thread_pool>(CThreadPoolNumber, ilInit, ilShutDown);
	mIoService = ioService;
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

bool ResourceManager::IsCurrentInAsyncService() const
{
	return std::this_thread::get_id() != mMainThreadId;
}
CoTask<void> ResourceManager::SwitchToLaunchService(Launch launchMode)
{
#if !defined MIR_CPPCORO_DISABLED
	if (launchMode == LaunchAsync) {
		if (! IsCurrentInAsyncService())
			CoAwait mThreadPool->schedule();
		BOOST_ASSERT(IsCurrentInAsyncService());
	}
	else {
		if (IsCurrentInAsyncService())
			CoAwait mIoService->schedule();
		BOOST_ASSERT(! IsCurrentInAsyncService());
	}
#endif
	CoReturnVoid;
}

/********** Async Support **********/
void ResourceManager::UpdateForLoading() ThreadSafe
{}

/********** Create Program **********/
inline boost::filesystem::path MakeShaderSourcePath(const std::string& name) {
	std::string filepath = "shader/" + name + ".hlsl";
	return boost::filesystem::system_complete(filepath);
}
static boost::filesystem::path MakeShaderAsmPath(const std::string& name, const ShaderCompileDesc& desc, const std::string& platform) {
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
CoTask<bool> ResourceManager::_LoadProgram(Launch launchMode, IProgramPtr program, const std::string& name, ShaderCompileDesc vertexSCD, ShaderCompileDesc pixelSCD) ThreadSafe
{
	COROUTINE_VARIABLES_5(program, launchMode, name, vertexSCD, pixelSCD);
	program->SetLoading();
	CoAwait SwitchToLaunchService(launchMode);

#if defined MIR_TIME_DEBUG
	std::string msg = (boost::format("resMng._LoadProgram %1% %2% %3%") % name %vertexSCD.EntryPoint %pixelSCD.EntryPoint).str();
	for (auto& macro : vertexSCD.Macros)
		msg += (boost::format(" %1%=%2%") % macro.Name %macro.Definition).str();
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

	CoAwait SwitchToLaunchService(__LaunchSync__);
	auto loadProgram = [blobVS, blobPS, this](IProgramPtr program)->IProgramPtr {
		std::vector<IShaderPtr> shaders;
		if (blobVS) shaders.push_back(this->mRenderSys.CreateShader(kShaderVertex, blobVS));
		if (blobPS) shaders.push_back(this->mRenderSys.CreateShader(kShaderPixel, blobPS));
		for (auto& it : shaders)
			it->SetLoaded();
		return this->mRenderSys.LoadProgram(program, shaders);
	};
	program->SetLoaded(loadProgram(program) != nullptr);
	CoReturn program->IsLoaded();
}
CoTask<bool> ResourceManager::CreateProgram(Launch launchMode, IProgramPtr& program, const std::string& name, ShaderCompileDesc vertexSCD, ShaderCompileDesc pixelSCD) ThreadSafe
{
	COROUTINE_VARIABLES_4(launchMode, name, vertexSCD, pixelSCD);
	//CoAwait SwitchToLaunchService(launchMode);

	ProgramKey key{ name, vertexSCD, pixelSCD };
	if (key.vertexSCD.ShaderModel.empty()) 
		key.vertexSCD.ShaderModel = "vs_4_0";
	if (key.pixelSCD.ShaderModel.empty()) 
		key.pixelSCD.ShaderModel = "ps_4_0";

	bool resNeedLoad = false;
	program = mProgramByKey.GetOrAdd(key, [this,&key,&resNeedLoad]() {
		auto program = std::static_pointer_cast<IProgram>(mRenderSys.CreateResource(kDeviceResourceProgram));
		DEBUG_SET_RES_PATH(program, (boost::format("name:%1%, vs:%2%, ps:%3%") %key.name %key.vertexSCD.EntryPoint %key.pixelSCD.EntryPoint).str());
		DEBUG_SET_CALL(program, launchMode);
		resNeedLoad = true;
		return program;
	});
	if (resNeedLoad) {
		CoAwait _LoadProgram(launchMode, program, key.name, key.vertexSCD, key.pixelSCD);
	}
	CoReturn program->IsLoaded();
}

/********** Create Texture **********/
CoTask<bool> ResourceManager::_LoadTextureByFile(Launch launchMode, ITexturePtr texture, const std::string& imgFullpath, ResourceFormat format, bool autoGenMipmap) ThreadSafe
{
	COROUTINE_VARIABLES_5(texture, launchMode, imgFullpath, format, autoGenMipmap);
	texture->SetLoading();
	CoAwait SwitchToLaunchService(launchMode);

	TIME_PROFILE((boost::format("resMng._LoadTextureByFile %1% %2% %3%") %imgFullpath %format %autoGenMipmap).str());
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
CoTask<bool> ResourceManager::CreateTextureByFile(Launch launchMode, ITexturePtr& texture, const std::string& filepath, ResourceFormat format, bool autoGenMipmap) ThreadSafe
{
	COROUTINE_VARIABLES_4(launchMode, filepath, format, autoGenMipmap);
	//CoAwait SwitchToLaunchService(launchMode);

	boost::filesystem::path fullpath = boost::filesystem::system_complete(filepath);
	std::string key = fullpath.string();

	bool resNeedLoad = false;
	texture = mTextureByKey.GetOrAdd(key, [&]() {
		auto texture = std::static_pointer_cast<ITexture>(this->mRenderSys.CreateResource(kDeviceResourceTexture));
		DEBUG_SET_RES_PATH(texture, (boost::format("path:%1%, fmt:%2%, autogen:%3%") %filepath %format %autoGenMipmap).str());
		DEBUG_SET_CALL(texture, launchMode);
		resNeedLoad = true;
		return texture;
	});
	if (resNeedLoad) {
		CoAwait _LoadTextureByFile(launchMode, texture, key, format, autoGenMipmap);
	}
	CoReturn texture->IsLoaded();
}

/********** Create Material **********/
CoTask <bool> ResourceManager::CreateShader(Launch launchMode, res::ShaderPtr& shader, const MaterialLoadParam& param) ThreadSafe
{
	COROUTINE_VARIABLES_2(launchMode, param);
	//CoAwait SwitchToLaunchService(launchMode);

	bool resNeedLoad = false;
	shader = mShaderByName.GetOrAdd(param, [&]() {
		auto shader = CreateInstance<res::Shader>();
		DEBUG_SET_RES_PATH(shader, (boost::format("name:%1% variant:%2%") %param.GetShaderName() %param.GetVariantDesc()).str());
		DEBUG_SET_CALL(shader, launchMode);
		resNeedLoad = true;
		return shader;
	});
	if (resNeedLoad) {
		CoAwait this->mMaterialFac.CreateShader(launchMode, shader, *this, param);
	}
	CoReturn shader->IsLoaded();
}

CoTask<bool> ResourceManager::CreateMaterial(Launch launchMode, res::MaterialInstance& matInst, const MaterialLoadParam& loadParam) ThreadSafe
{
	COROUTINE_VARIABLES_2(launchMode, loadParam);
	//CoAwait SwitchToLaunchService(launchMode);

	bool resNeedLoad = false;
	res::MaterialPtr material = mMaterialByName.GetOrAdd(loadParam, [&]() {
		auto material = CreateInstance<res::Material>();
		DEBUG_SET_RES_PATH(material, (boost::format("name:%1% variant:%2%") %loadParam.GetShaderName() %loadParam.GetVariantDesc()).str());
		DEBUG_SET_CALL(material, launchMode);
		resNeedLoad = true;
		return material;
	});
	if (resNeedLoad) {
		auto param = loadParam;
		CoAwait this->mMaterialFac.CreateMaterial(launchMode, material, *this, param);
	}
	matInst = material->CreateInstance(launchMode, *this);
	return matInst->IsLoaded();
}

/********** Create AiScene **********/
CoTask<bool> ResourceManager::CreateAiScene(Launch launchMode, res::AiScenePtr& aiScene, const std::string& assetPath, const std::string& redirectRes) ThreadSafe
{
	COROUTINE_VARIABLES_3(launchMode, assetPath, redirectRes);
	//CoAwait SwitchToLaunchService(launchMode);

	AiResourceKey key{ assetPath, redirectRes };
	bool resNeedLoad = false;
	aiScene = mAiSceneByKey.GetOrAdd(key, [&]() {
		auto aiScene = CreateInstance<res::AiScene>();
		DEBUG_SET_RES_PATH(aiScene, (boost::format("path:%1%, redirect:%2%") %assetPath %redirectRes).str());
		DEBUG_SET_CALL(aiScene, launchMode);
		resNeedLoad = true;
		return aiScene;
	});
	if (resNeedLoad) {
		CoAwait this->mAiResourceFac.CreateAiScene(launchMode, aiScene, *this, assetPath, redirectRes);
	}
	CoReturn aiScene->IsLoaded();
}

}