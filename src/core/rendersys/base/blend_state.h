#pragma once

namespace mir {

enum BlendFunc 
{
	kBlendZero = 1,
	kBlendOne = 2,
	kBlendSrcColor = 3,
	kBlendInvSrcColor = 4,
	kBlendSrcAlpha = 5,
	kBlendInvSrcAlpha = 6,
	kBlendDstAlpha = 7,
	kBlendInvDstAlpha = 8,
	kBlendDstColor = 9,
	kBlendInvDstColor = 10,
	kBlendSrcAlphaSat = 11,
	kBlendBlendFactor = 14,
	kBlendInvBlendFactor = 15,
	kBlendSrc1Color = 16,
	kBlendInvSrc1Color = 17,
	kBlendSrc1Alpha = 18,
	kBlendInvSrc1Alpha = 19
};

struct BlendState 
{
	constexpr static BlendState MakeDisable() { return BlendState{ kBlendOne, kBlendZero }; }
	constexpr static BlendState MakeAlphaPremultiplied() { return BlendState{ kBlendOne, kBlendInvSrcAlpha }; }
	constexpr static BlendState MakeAlphaNonPremultiplied() { return BlendState{ kBlendSrcAlpha, kBlendInvSrcAlpha }; }
	constexpr static BlendState MakeAdditive() { return BlendState{ kBlendSrcAlpha, kBlendOne }; }
	constexpr static BlendState Make(BlendFunc src, BlendFunc dst) { return BlendState{ src, dst }; }
public:
	BlendFunc Src, Dst;
};
inline bool operator==(const BlendState& l, const BlendState& r) { return l.Src == r.Src && l.Dst == r.Dst; }
inline bool operator!=(const BlendState& l, const BlendState& r) { return !(l == r); }
inline bool operator<(const BlendState& l, const BlendState& r) { return (l.Src != r.Src) ? (l.Src < r.Src) : (l.Dst < r.Dst); }

}