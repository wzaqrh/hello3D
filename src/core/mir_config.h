#pragma once
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#define MIR_TEST
#define MIR_D3D11_DEBUG
#define MIR_RESOURCE_DEBUG
#define MIR_TIME_DEBUG
#define MIR_LOG_LEVEL 0
#define MIR_COROUTINE_DEBUG 
//#define MIR_CPPCORO_DISABLED

namespace mir {

struct MIR_CORE_API Configure : boost::noncopyable {
	Configure();
	bool IsShadowVSM() const;
public:
	int _SHADOW_MODE;
};

}