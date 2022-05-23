#pragma once
#include "core/rendersys/base/compare_func.h"

namespace mir {

enum DepthWriteMask 
{
	kDepthWriteMaskZero = 0,
	kDepthWriteMaskAll = 1
};

struct DepthState 
{
	constexpr static DepthState MakeFor2D() { return DepthState{ kCompareLess, kDepthWriteMaskZero, false }; }
	constexpr static DepthState MakeFor3D(bool depthEnable = true) { return DepthState{ kCompareLess, kDepthWriteMaskAll, depthEnable }; }
	constexpr static DepthState Make(CompareFunc cmp, DepthWriteMask mask, bool enable = true) { return DepthState{ cmp, mask, enable }; }
	bool operator<(const DepthState& r) const {
		if (DepthEnable != r.DepthEnable) return DepthEnable < r.DepthEnable;
		if (CmpFunc != r.CmpFunc) return CmpFunc < r.CmpFunc;
		return WriteMask < r.WriteMask;
	}
public:
	CompareFunc CmpFunc;
	DepthWriteMask WriteMask;
	bool DepthEnable;
};
inline bool operator==(const DepthState& l, const DepthState& r) { return l.DepthEnable == r.DepthEnable && l.CmpFunc == r.CmpFunc && l.WriteMask == r.WriteMask; }
inline bool operator!=(const DepthState& l, const DepthState& r) { return !(l == r); }


}