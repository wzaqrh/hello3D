#pragma once
#include <boost/noncopyable.hpp>
#define MIR_TEST
#define MIR_D3D11_DEBUG
#define MIR_RESOURCE_DEBUG
#define MIR_TIME_DEBUG
#define MIR_LOG_LEVEL 0
#define MIR_COROUTINE_DEBUG 
//#define MIR_CPPCORO_DISABLED

namespace mir {

struct Configure : boost::noncopyable {
#define SHADOW_RAW 1 
#define SHADOW_PCF_FAST 2
#define SHADOW_PCF 3
#define SHADOW_PCSS 4
#define SHADOW_VSM 5
	int SHADOW_MODE = SHADOW_VSM;
	bool IsShadowVSM() const { return SHADOW_MODE == SHADOW_VSM; }
};

}