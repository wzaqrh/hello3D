#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include "core/base/debug.h"
#include "core/base/macros.h"
#include "core/base/data.h"
#include "core/base/input.h"
#include "core/rendersys/blob.h"
#include "core/resource/program_factory.h"
#include "core/resource/resource_manager.h"

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

boost::filesystem::path ProgramFactory::MakeShaderSourcePath(const std::string& name) const
{
	std::string filepath = mShaderDir + name + ".hlsl";
	return boost::filesystem::system_complete(filepath);
}
boost::filesystem::path ProgramFactory::MakeShaderAsmPath(const std::string& name, const ShaderCompileDesc& desc, const std::string& platform) const
{
	std::string asmName = name;
	asmName += "_" + desc.EntryPoint;
	asmName += " " + desc.ShaderModel;

	boost::filesystem::create_directories(mShaderDir + "asm/" + platform);
#if 0
	for (const auto& macro : desc.Macros)
		asmName += " (" + macro.Name + "=" + macro.Definition + ")";
#else
	std::string macrosStr = "";
	for (const auto& macro : desc.Macros)
		macrosStr += " (" + macro.Name + "=" + macro.Definition + ")";

	std::string macrosHashStr;
	bool hashMatch = false;
	int loopCount = 0;
	do {
		loopCount++;
		size_t macrosHash = std::hash<std::string>()(macrosStr + boost::lexical_cast<std::string>(loopCount));
		macrosHashStr = boost::lexical_cast<std::string>(macrosHash);
		std::string macrosHashFilename = (mShaderDir + "asm/" + platform + "/") + asmName + " " + macrosHashStr + ".txt";
		FILE* fhash = fopen(macrosHashFilename.c_str(), "rb");
		if (fhash) {
			std::string rdMacrosStr;
			char buf[2048];
			while (int rd = fread(buf, 1, 2047, fhash)) {
				buf[rd] = 0;
				rdMacrosStr += buf;
			}
			fclose(fhash);

			if (rdMacrosStr == macrosStr) {
				hashMatch = true;
			}
		}
		else {
			fhash = fopen(macrosHashFilename.c_str(), "wb");
			fwrite(macrosStr.c_str(), 1, macrosStr.size(), fhash);
			fclose(fhash);

			hashMatch = true;
		}
	} while (!hashMatch);
	asmName += " " + macrosHashStr;
#endif

#if defined MIR_RESOURCE_DEBUG
	auto sourcePath = MakeShaderSourcePath(name);
	time_t time = boost::filesystem::last_write_time(sourcePath);

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

	tm lwt;
	gmtime_s(&lwt, &time);
	boost::format fmt(" [%d-%d-%d %d.%d.%d]");
	fmt% lwt.tm_year% lwt.tm_mon% lwt.tm_mday;
	fmt% lwt.tm_hour% lwt.tm_min% lwt.tm_sec;
	asmName += fmt.str();
#endif
	asmName += ".cso";
	std::string filepath = mShaderDir + "asm/" + platform + "/" + asmName;
	return boost::filesystem::system_complete(filepath);
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
				BOOST_ASSERT(input::WriteFile(psAsmPath.string().c_str(), "wb", blobPS->GetBytes(), blobPS->GetSize()));
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