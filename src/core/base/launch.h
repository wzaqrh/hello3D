#pragma once
#include "core/mir_config.h"
#include "core/base/stl.h"

namespace mir {

#define LaunchMode Launch
enum Launch {
	LaunchSync,
	LaunchAsync
};
#define __LaunchSync__  LaunchSync
#define __LaunchAsync__ LaunchAsync
#define __launchMode__  launchMode
#define ResSetLaunch

}