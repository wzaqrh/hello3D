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

struct ScissorState
{
	static ScissorState MakeDisable() { return ScissorState{ false }; }
	static ScissorState Make(int l, int t, int r, int b) { return ScissorState{ true,std::vector<Eigen::Vector4i>{Eigen::Vector4i(l,t,r,b)} }; }
	bool operator<(const ScissorState& other) const {
		if (ScissorEnable != other.ScissorEnable) return ScissorEnable < other.ScissorEnable;
		if (Rects.size() != other.Rects.size()) return Rects.size() < other.Rects.size();
		if (!Rects.empty()) {
			const int* lr = (int*)&Rects[0];
			const int* rr = (int*)&other.Rects[0];
			for (size_t i = 0; i < Rects.size() * 4; ++i) {
				if (lr[i] != rr[i]) return lr[i] < rr[i];
			}
		}
		return false;
	}
public:
	bool ScissorEnable;
	std::vector<Eigen::Vector4i> Rects;
};
inline bool operator==(const ScissorState& l, const ScissorState& r) { return l.ScissorEnable == r.ScissorEnable && l.Rects == r.Rects; }
inline bool operator!=(const ScissorState& l, const ScissorState& r) { return !(l == r); }

struct RasterizerState 
{
	bool operator<(const RasterizerState& r) const {
		if (FillMode != r.FillMode) return FillMode < r.FillMode;
		if (CullMode != r.CullMode) return CullMode < r.CullMode;
		if (DepthBias != r.DepthBias) return DepthBias < r.DepthBias;
		return Scissor < r.Scissor;
	}
public:
	FillMode FillMode;
	CullMode CullMode;
	DepthBias DepthBias;
	ScissorState Scissor;
};

}