#pragma once
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#include "core/rendersys/predeclare.h"
#include "core/base/declare_macros.h"

namespace mir {

interface MIR_CORE_API IObject : boost::noncopyable
{
	virtual IResourcePtr AsRes() = 0;
};

}