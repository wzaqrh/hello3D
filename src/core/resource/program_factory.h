#pragma once
#include <boost/noncopyable.hpp>
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
	ProgramFactory(ResourceManager& resMng);

	CoTask<bool> CreateProgram(IProgramPtr& program, Launch lchMode, std::string name, ShaderCompileDesc vertexSCD, ShaderCompileDesc pixelSCD) ThreadSafe;
	DECLARE_COTASK_FUNCTIONS(IProgramPtr, CreateProgram, ThreadSafe);

	void PurgeAll() ThreadSafe;
private:
	CoTask<bool> _LoadProgram(IProgramPtr program, Launch lchMode, std::string name, ShaderCompileDesc vertexSCD, ShaderCompileDesc pixelSCD) ThreadSafe;
private:
	ResourceManager& mResMng;
	RenderSystem& mRenderSys;
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