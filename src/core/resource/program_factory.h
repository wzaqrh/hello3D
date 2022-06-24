#pragma once
#include <boost/noncopyable.hpp>
#include <boost/filesystem.hpp>
#include "core/base/stl.h"
#include "core/base/math.h"
#include "core/base/cppcoro.h"
#include "core/base/launch.h"
#include "core/base/tpl/atomic_map.h"
#include "core/base/declare_macros.h"
#include "core/rendersys/predeclare.h"
#include "core/resource/predeclare.h"
#include "core/rendersys/program.h"

namespace mir {
namespace res {

class MIR_CORE_API ProgramFactory : boost::noncopyable
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	ProgramFactory(ResourceManager& resMng, const std::string& shaderDir);
	~ProgramFactory();

	CoTask<bool> CreateProgram(IProgramPtr& program, Launch lchMode, std::string name, ShaderCompileDesc vertexSCD, ShaderCompileDesc pixelSCD) ThreadSafe ThreadMaySwitch;
	DECLARE_COTASK_FUNCTIONS(IProgramPtr, CreateProgram, ThreadSafe ThreadMaySwitch);

	void PurgeAll() ThreadSafe;
private:
	CoTask<bool> _LoadProgram(IProgramPtr program, Launch lchMode, std::string name, ShaderCompileDesc vertexSCD, ShaderCompileDesc pixelSCD) ThreadSafe ThreadMaySwitch;
	boost::filesystem::path MakeShaderSourcePath(const std::string& name) const ThreadSafe;
	boost::filesystem::path MakeShaderAsmPath(const std::string& name, const ShaderCompileDesc& desc, const std::string& platform, time_t& time, std::string& serializeStr) const ThreadSafe;
	bool ReadShaderAsm(const boost::filesystem::path& asmPath, std::vector<char>& bin, time_t time, const std::string& serializeStr) const ThreadSafe;
	void WriteShaderAsm(const boost::filesystem::path& asmPath, const char* pByte, size_t size, time_t time, const std::string& serializeStr) const ThreadSafe;
private:
	ResourceManager& mResMng;
	RenderSystem& mRenderSys;
	std::string mShaderDir, mShaderExt, mPlatformName;
	struct ProgramKey {
		std::string name;
		ShaderCompileDesc vertexSCD, pixelSCD;
		bool operator<(const ProgramKey& other) const {
			if (name != other.name) return name < other.name;
			if (!(vertexSCD == other.vertexSCD)) return vertexSCD < other.vertexSCD;
			return pixelSCD < other.pixelSCD;
		}
	};
	tpl::AtomicMap<ProgramKey, IProgramPtr> mProgramByKey;
};

}
}