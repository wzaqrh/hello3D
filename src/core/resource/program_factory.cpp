#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include "core/base/debug.h"
#include "core/base/macros.h"
#include "core/base/data.h"
#include "core/base/input.h"
#include "core/base/md5.h"
#include "core/rendersys/blob.h"
#include "core/resource/program_factory.h"
#include "core/resource/resource_manager.h"

//#define MIR_SHADER_CACHE

namespace mir {
namespace res {

ProgramFactory::ProgramFactory(ResourceManager& resMng, const std::string& shaderDir)
: mResMng(resMng)
, mRenderSys(resMng.RenderSys())
, mShaderDir(shaderDir)
{
}
ProgramFactory::~ProgramFactory()
{
	DEBUG_LOG_MEMLEAK("progFac.destrcutor");
}

struct ShaderCompileDescHelper {
public:
	ShaderCompileDescHelper(const ShaderCompileDesc& scd, const std::string& src): mSCD(scd), mSource(src) {}
	const std::string& GetSerializeString() {
		if (mSerializeString.empty()) {
			std::string& ss = mSerializeString;
			ss = "src:" + mSource;
			ss += ";entry:" + mSCD.EntryPoint;
			ss += ";sm:" + mSCD.ShaderModel;
			for (const auto& m : mSCD.Macros) {
				ss += ";" + m.Name + ":" + m.Definition;
			}
		}
		return mSerializeString;
	}
	void GetMd5(uint8_t digest[16]) {
		const std::string& ss = GetSerializeString();
		md5((const uint8_t*)ss.c_str(), ss.length(), digest);
	}
	std::string GetMd5String() {
		uint32_t digest[4];
		GetMd5((uint8_t*)digest);
		return (boost::format("%x%x%x%x") %digest[0] %digest[1] %digest[2] %digest[3]).str();
	}
public:
	std::string mSerializeString;
	const ShaderCompileDesc& mSCD;
	const std::string& mSource;
};

boost::filesystem::path ProgramFactory::MakeShaderSourcePath(const std::string& name) const
{
	std::string filepath = mShaderDir + name + ".hlsl";
	return boost::filesystem::system_complete(filepath);
}
boost::filesystem::path ProgramFactory::MakeShaderAsmPath(const std::string& name, const ShaderCompileDesc& desc, const std::string& platform, time_t& time, std::string& serializeStr) const
{
	std::string asmDir = mShaderDir + "asm_" + platform + "/";
	boost::filesystem::create_directories(asmDir);

	ShaderCompileDescHelper scdHelper(desc, name);
	std::string md5Str = scdHelper.GetMd5String();
	std::string asmFilePath = asmDir + md5Str + ".ntasm";

	serializeStr = scdHelper.GetSerializeString();

	time = 0;
#if defined MIR_RESOURCE_DEBUG || defined MIR_SHADER_CACHE
	auto sourcePath = MakeShaderSourcePath(name);
	time = boost::filesystem::last_write_time(sourcePath);

	time_t cgincs_time = 0;
	{
		boost::filesystem::directory_iterator diter(mShaderDir), dend;
		for (; diter != dend; ++diter) {
			if (boost::filesystem::is_regular_file(*diter) && (*diter).path().extension() == ".cginc") {
				time_t htime = boost::filesystem::last_write_time((*diter).path());
				cgincs_time = std::max(cgincs_time, htime);
			}
		}
	}
	time = std::max(time, cgincs_time);
#endif

	return boost::filesystem::system_complete(asmFilePath);
}
#define NTASM_NAME_LEN (1024-64)
#define NTASM_TIME_SIZE (64)
void ProgramFactory::WriteShaderAsm(const boost::filesystem::path& asmPath, const char* pByte, size_t size, time_t time, const std::string& serializeStr) const ThreadSafe
{
	std::vector<char> bin(pByte, pByte + size);
	
	size_t position = bin.size();
	bin.resize(position + NTASM_NAME_LEN);
	BOOST_ASSERT(serializeStr.length() < NTASM_NAME_LEN);
	strcpy(&bin[position], serializeStr.c_str());

	position = bin.size();
	bin.resize(position + NTASM_TIME_SIZE);
	std::string timeStr = boost::lexical_cast<std::string>(time);
	BOOST_ASSERT(timeStr.length() < NTASM_TIME_SIZE);
	strcpy(&bin[position], timeStr.c_str());

	input::WriteFile(asmPath.string().c_str(), "wb", &bin[0], bin.size());
}
bool ProgramFactory::ReadShaderAsm(const boost::filesystem::path& asmPath, std::vector<char>& bin, time_t time, const std::string& serializeStr) const
{
	bin.clear();
	if (boost::filesystem::exists(asmPath)) {
		bin = input::ReadFile(asmPath.string().c_str(), "rb");
		if (bin.size() >= NTASM_NAME_LEN + NTASM_TIME_SIZE) {
			size_t position = bin.size() - NTASM_NAME_LEN - NTASM_TIME_SIZE;
			std::string fileSeriStr(bin.begin() + position, bin.begin() + position + NTASM_NAME_LEN);
			if (strcmp(fileSeriStr.c_str(), serializeStr.c_str()) == 0) {
				position += NTASM_NAME_LEN;
				std::string fileTimeStr(bin.begin() + position, bin.begin() + position + NTASM_TIME_SIZE);
				time_t fileTime = std::stoi(fileTimeStr);
				if (fileTime >= time) {
					bin.resize(bin.size() - NTASM_NAME_LEN - NTASM_TIME_SIZE);
					return true;
				}
			}
		}
	}
	return false;
}

CoTask<bool> ProgramFactory::_LoadProgram(IProgramPtr program, Launch lchMode, std::string name, ShaderCompileDesc vertexSCD, ShaderCompileDesc pixelSCD) ThreadSafe
{
	program->SetLoading();
	CoAwait mResMng.SwitchToLaunchService(lchMode);
	COROUTINE_VARIABLES_5(program, lchMode, name, vertexSCD, pixelSCD);

#if defined MIR_TIME_DEBUG || defined MIR_RESOURCE_DEBUG
	std::string msg = (boost::format("\t\tresMng._LoadProgram (%1% %2% %3%") %name %vertexSCD.EntryPoint % pixelSCD.EntryPoint).str();
	for (auto& macro : vertexSCD.Macros)
		msg += (boost::format(" %1%=%2%") %macro.Name %macro.Definition).str();
	msg += ")";
	TIME_PROFILE(msg);
#endif

	IBlobDataPtr blobVS, blobPS;
	if (!vertexSCD.EntryPoint.empty()) {
		time_t time;
		std::string serializeStr;
		std::vector<char> bin;
		boost::filesystem::path vsAsmPath = MakeShaderAsmPath(name, vertexSCD, mRenderSys.GetPlatform(), time, serializeStr);
		if (ReadShaderAsm(vsAsmPath, bin, time, serializeStr)) {
			blobVS = CreateInstance<BlobDataBytes>(std::move(bin));
		}
		else {
			vertexSCD.SourcePath = MakeShaderSourcePath(name).string();
			bin = input::ReadFile(vertexSCD.SourcePath.c_str(), "rb");
			BOOST_ASSERT(!bin.empty());
			if (!bin.empty()) {
				blobVS = this->mRenderSys.CompileShader(vertexSCD, Data::Make(bin));
			#if defined MIR_RESOURCE_DEBUG || defined MIR_SHADER_CACHE
				WriteShaderAsm(vsAsmPath, blobVS->GetBytes(), blobVS->GetSize(), time, serializeStr);
			#endif
			}
		}
	}

	if (!pixelSCD.EntryPoint.empty()) {
		time_t time;
		std::string serializeStr;
		std::vector<char> bin;
		boost::filesystem::path psAsmPath = MakeShaderAsmPath(name, pixelSCD, mRenderSys.GetPlatform(), time, serializeStr);
		if (ReadShaderAsm(psAsmPath, bin, time, serializeStr)) {
			blobPS = CreateInstance<BlobDataBytes>(std::move(bin));
		}
		else {
			pixelSCD.SourcePath = MakeShaderSourcePath(name).string();
			bin = input::ReadFile(pixelSCD.SourcePath.c_str(), "rb");
			BOOST_ASSERT(!bin.empty());
			if (!bin.empty()) {
				blobPS = this->mRenderSys.CompileShader(pixelSCD, Data::Make(bin));
			#if defined MIR_RESOURCE_DEBUG || defined MIR_SHADER_CACHE
				WriteShaderAsm(psAsmPath, blobPS->GetBytes(), blobPS->GetSize(), time, serializeStr);
			#endif
			}
		}
	}

	CoAwait mResMng.SwitchToLaunchService(__LaunchSync__);
	auto loadProgram = [blobVS, blobPS, this](IProgramPtr program)->IProgramPtr {
		std::vector<IShaderPtr> shaders;
		if (blobVS) shaders.push_back(this->mRenderSys.CreateShader(kShaderVertex, blobVS));
		if (blobPS) shaders.push_back(this->mRenderSys.CreateShader(kShaderPixel, blobPS));
		for (auto& it : shaders)
			it->SetLoaded();
		return this->mRenderSys.LoadProgram(program, shaders);
	};
	program->SetLoaded(loadProgram(program) != nullptr);
	DEBUG_SET_PRIV_DATA(program, msg);
	CoReturn program->IsLoaded();
}

CoTask<bool> ProgramFactory::CreateProgram(IProgramPtr& program, Launch lchMode, std::string name, ShaderCompileDesc vertexSCD, ShaderCompileDesc pixelSCD) ThreadSafe
{
	//CoAwait SwitchToLaunchService(lchMode);
	COROUTINE_VARIABLES_4(lchMode, name, vertexSCD, pixelSCD);

	ProgramKey key{ std::move(name), std::move(vertexSCD), std::move(pixelSCD) };
	if (key.vertexSCD.ShaderModel.empty())
		key.vertexSCD.ShaderModel = "vs_4_0";
	if (key.pixelSCD.ShaderModel.empty())
		key.pixelSCD.ShaderModel = "ps_4_0";

	bool resNeedLoad = false;
	program = mProgramByKey.GetOrAdd(key, [this, &key, &resNeedLoad]() {
		auto program = std::static_pointer_cast<IProgram>(mRenderSys.CreateResource(kDeviceResourceProgram));
		DEBUG_SET_RES_PATH(program, (boost::format("name:%1%, vs:%2%, ps:%3%") % key.name % key.vertexSCD.EntryPoint % key.pixelSCD.EntryPoint).str());
		DEBUG_SET_CALL(program, lchMode);
		resNeedLoad = true;
		return program;
		});
	if (resNeedLoad) {
		CoAwait this->_LoadProgram(program, lchMode, std::move(key.name), std::move(key.vertexSCD), std::move(key.pixelSCD));
	}
	else {
		CoAwait mResMng.WaitResComplete(program);
	}
	CoReturn program->IsLoaded();
}

void ProgramFactory::PurgeAll() ThreadSafe
{
	mProgramByKey.Clear();
}

}
}