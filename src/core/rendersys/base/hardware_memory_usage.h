#pragma once

namespace mir {

enum HWMemoryUsage 
{
	kHWUsageDefault = 0,
	kHWUsageImmutable = 1,
	kHWUsageDynamic = 2,
	kHWUsageStaging = 3
};

}