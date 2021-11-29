#pragma once
#include "core/mir_config.h"
#include "core/base/stl.h"

namespace mir {

#if defined MIR_RESOURCE_DEBUG
enum LaunchMode {
	LaunchSync,
	LaunchAsync
};
struct Launch 
{
	Launch() :Mode() {}
	Launch(LaunchMode mode, const char* callStack) :Mode(mode), CallStack(callStack) {}
	Launch(const Launch& mode, const char* callStack) :Mode(mode.Mode), CallStack(callStack) {}
	Launch& operator=(LaunchMode mode) {
		Mode = mode;
		CallStack.clear();
		return *this;
	}
	bool operator==(LaunchMode mode) const {
		return Mode == mode;
	}
	bool operator!=(LaunchMode mode) const {
		return Mode != mode;
	}
	LaunchMode Mode;
	std::string CallStack;
};
#define __LaunchSync__  Launch(LaunchSync, __FUNCTION__)
#define __LaunchAsync__ Launch(LaunchAsync, __FUNCTION__)
#define __launchMode__  Launch(launchMode, __FUNCTION__)
#define ResSetLaunch	res->_Debug._CallStack = launchMode.CallStack
#else
#define LaunchMode Launch
enum Launch {
	LaunchSync,
	LaunchAsync
};
#define __LaunchSync__  LaunchSync
#define __LaunchAsync__ LaunchAsync
#define __launchMode__  launchMode
#define ResSetLaunch
#endif

}