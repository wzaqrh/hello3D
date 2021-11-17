#pragma once
#include <boost/noncopyable.hpp>
#include "core/base/stl.h"
#include "core/mir_export.h"
#include "core/rendersys/predeclare.h"
#include "core/rendersys/base_type.h"
#include "core/rendersys/resource.h"

namespace mir {

class MIR_CORE_API ResourceManager : boost::noncopyable {
public:
	ResourceManager(RenderSystem& renderSys);
	IInputLayoutPtr CreateLayoutAsync(IProgramPtr pProgram, LayoutInputElement descArray[], size_t descCount);
private:
	RenderSystem& mRenderSys;
};

}