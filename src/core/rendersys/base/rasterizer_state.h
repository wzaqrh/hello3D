#pragma once
#include "core/base/stl.h"
#include "core/base/math.h"

namespace mir {

enum FillMode 
{
	kFillUnkown = 0,
	kFillWireFrame = 2,
	kFillSolid = 3
};

enum CullMode 
{
	kCullUnkown = 0,
	kCullNone = 1,
	kCullFront = 2,
	kCullBack = 3
};

struct DepthBias 
{
	constexpr static DepthBias Make(float bias, float slopeScaleBias) { return DepthBias{ bias, slopeScaleBias }; }
	bool operator<(const DepthBias& r) const { return (Bias != r.Bias) ? (Bias < r.Bias) : (SlopeScaledBias < r.SlopeScaledBias); }
public:
	float Bias;
	float SlopeScaledBias;
};
inline bool operator==(const DepthBias& l, const DepthBias& r) { return l.Bias == r.Bias && l.SlopeScaledBias == r.SlopeScaledBias; }
inline bool operator!=(const DepthBias& l, const DepthBias& r) { return !(l == r); }

struct RasterizerState 
{
	bool operator<(const RasterizerState& r) const {
		if (FillMode != r.FillMode) return FillMode < r.FillMode;
		if (CullMode != r.CullMode) return CullMode < r.CullMode;
		return DepthBias < r.DepthBias;
	}
public:
	FillMode FillMode;
	CullMode CullMode;
	DepthBias DepthBias;
};

}