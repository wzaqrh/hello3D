#pragma once
#include "core/base/stl.h"
#include "core/base/math.h"

namespace mir {

struct Data {
	static Data MakeEmpty() { return Data{ nullptr, 0 }; }
	template<class T> static Data Make(const T& v) { return Data{ (void*)&v, sizeof(v) }; }
	template<class T> static Data Make(const std::vector<T>& v) { return Data{ (void*)&v[0], sizeof(T) * v.size() }; }
	static Data Make(void* data, unsigned size) { return Data{ data, size }; }
public:
	void* Bytes;
	size_t Size;
};

struct ShaderCompileMacro {
	std::string Name, Definition;
};
struct ShaderCompileDesc {
	std::vector<ShaderCompileMacro> Macros;
	std::string EntryPoint, ShaderModel, SourcePath;
};

enum BlendFunc {
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
struct BlendState {
	static BlendState MakeDisable() { return BlendState{ kBlendOne, kBlendZero }; }
	static BlendState MakeAlphaPremultiplied() { return BlendState{ kBlendOne, kBlendInvSrcAlpha }; }
	static BlendState MakeAlphaNonPremultiplied() { return BlendState{ kBlendSrcAlpha, kBlendInvSrcAlpha }; }
	static BlendState MakeAdditive() { return BlendState{ kBlendSrcAlpha, kBlendOne }; }
public:
	BlendFunc Src, Dst;
};

enum CompareFunc {
	kCompareNever = 1,
	kCompareLess = 2,
	kCompareEqual = 3,
	kCompareLessEqual = 4,
	kCompareGreater = 5,
	kCompareNotEqual = 6,
	kCompareGreaterEqual = 7,
	kCompareAlways = 8
};

enum DepthWriteMask {
	kDepthWriteMaskZero = 0,
	kDepthWriteMaskAll = 1
};
struct DepthState {
	static DepthState MakeFor2D(bool depthEnable) {
		return DepthState{ depthEnable, kCompareLess, kDepthWriteMaskZero };
	}
	static DepthState MakeFor3D() {
		return DepthState{ true, kCompareLess, kDepthWriteMaskAll };
	}
public:
	bool DepthEnable;
	CompareFunc CmpFunc;
	DepthWriteMask WriteMask;
};

enum SamplerFilterMode {
	kSamplerFilterMinMagMipPoint = 0,
	kSamplerFilterMinMagPointMipLinear = 0x1,
	kSamplerFilterMinPointMagLinearMipPoint = 0x4,
	kSamplerFilterMinPointMagMipLinear = 0x5,
	kSamplerFilterMinLinearMagMipPoint = 0x10,
	kSamplerFilterMinLinearMagPointMipLinear = 0x11,
	kSamplerFilterMinMagLinearMipPoint = 0x14,
	kSamplerFilterMinMagMipLinear = 0x15,
	kSamplerFilterAnisotropic = 0x55,
	kSamplerFilterCmpMinMagMipPoint = 0x80,
	kSamplerFilterCmpMinMagPointMipLinear = 0x81,
	kSamplerFilterCmpMinPointMagLinearMipPoint = 0x84,
	kSamplerFilterCmpMinPointMagMipLinear = 0x85,
	kSamplerFilterCmpMinLinearMagMipPoint = 0x90,
	kSamplerFilterCmpMinLinearMagPointMipLinear = 0x91,
	kSamplerFilterCmpMinMagLinearMipPoint = 0x94,
	kSamplerFilterCmpMinMagMipLinear = 0x95,
	kSamplerFilterCmpAnisotropic = 0xd5,
	kSamplerFilterMinimumMinMagMipPoint = 0x100,
	kSamplerFilterMinimumMinMagPointMipLinear = 0x101,
	kSamplerFilterMinimumMinPointMagLinearMipPoint = 0x104,
	kSamplerFilterMinimumMinPointMagMipLinear = 0x105,
	kSamplerFilterMinimumMinLinearMagMipPoint = 0x110,
	kSamplerFilterMinimumMinLinearMagPointMipLinear = 0x111,
	kSamplerFilterMinimumMinMagLinearMipPoint = 0x114,
	kSamplerFilterMinimumMinMagMipLinear = 0x115,
	kSamplerFilterMinimumAnisotropic = 0x155,
	kSamplerFilterMaximumMinMagMipPoint = 0x180,
	kSamplerFilterMaximumMinMagPointMipLinear = 0x181,
	kSamplerFilterMaximumMinPointMagLinearMipPoint = 0x184,
	kSamplerFilterMaximumMinPointMagMipLinear = 0x185,
	kSamplerFilterMaximumMinLinearMagMipPoint = 0x190,
	kSamplerFilterMaximumMinLinearMagPointMipLinear = 0x191,
	kSamplerFilterMaximumMinMagLinearMipPoint = 0x194,
	kSamplerFilterMaximumMinMagMipLinear = 0x195,
	kSamplerFilterMaximumAnisotropic = 0x1d5
};

enum ResourceFormat {
	kFormatUnknown = 0,
	kFormatR32G32B32A32Typeless = 1,
	kFormatR32G32B32A32Float = 2,
	kFormatR32G32B32A32UInt = 3,
	kFormatR32G32B32A32SInt = 4,
	kFormatR32G32B32Typeless = 5,
	kFormatR32G32B32Float = 6,
	kFormatR32G32B32UInt = 7,
	kFormatR32G32B32SInt = 8,
	kFormatR16G16B16A16Typeless = 9,
	kFormatR16G16B16A16Float = 10,
	kFormatR16G16B16A16UNorm = 11,
	kFormatR16G16B16A16UInt = 12,
	kFormatR16G16B16A16SNorm = 13,
	kFormatR16G16B16A16SInt = 14,
	kFormatR32G32Typeless = 15,
	kFormatR32G32Float = 16,
	kFormatR32G32UInt = 17,
	kFormatR32G32SInt = 18,
	kFormatR32G8X24Typeless = 19,
	kFormatD32FloatS8X24UInt = 20,
	kFormatR32FloatX8X24Typeless = 21,
	kFormatX32TypelessG8X24UInt = 22,
	kFormatR10G10B10A2Typeless = 23,
	kFormatR10G10B10A2UNorm = 24,
	kFormatR10G10B10A2UInt = 25,
	kFormatR11G11B10Float = 26,
	kFormatR8G8B8A8Typeless = 27,
	kFormatR8G8B8A8UNorm = 28,
	kFormatR8G8B8A8UNormSRgb = 29,
	kFormatR8G8B8A8UInt = 30,
	kFormatR8G8B8A8SNorm = 31,
	kFormatR8G8B8A8SInt = 32,
	kFormatR16G16Typeless = 33,
	kFormatR16G16Float = 34,
	kFormatR16G16UNorm = 35,
	kFormatR16G16UInt = 36,
	kFormatR16G16SNorm = 37,
	kFormatR16G16SInt = 38,
	kFormatR32Typeless = 39,
	kFormatD32Float = 40,
	kFormatR32Float = 41,
	kFormatR32UInt = 42,
	kFormatR32SInt = 43,
	kFormatR24G8Typeless = 44,
	kFormatD24UNormS8UInt = 45,
	kFormatR24UNormX8Typeless = 46,
	kFormatX24Typeless_G8UInt = 47,
	kFormatR8G8Typeless = 48,
	kFormatR8G8UNorm = 49,
	kFormatR8G8UInt = 50,
	kFormatR8G8SNorm = 51,
	kFormatR8G8SInt = 52,
	kFormatR16Typeless = 53,
	kFormatR16Float = 54,
	kFormatD16UNorm = 55,
	kFormatR16UNorm = 56,
	kFormatR16UInt = 57,
	kFormatR16SNorm = 58,
	kFormatR16SInt = 59,
	kFormatR8Typeless = 60,
	kFormatR8UNorm = 61,
	kFormatR8UInt = 62,
	kFormatR8SNorm = 63,
	kFormatR8SInt = 64,
	kFormatA8UNorm = 65,
	kFormatR1UNorm = 66,
	kFormatR9G9B9E5ShaderExp = 67,
	kFormatR8G8B8G8UNorm = 68,
	kFormatG8R8G8B8UNorm = 69,
	kFormatBC1Typeless = 70,
	kFormatBC1UNorm = 71,
	kFormatBC1UNormSRgb = 72,
	kFormatBC2Typeless = 73,
	kFormatBC2UNorm = 74,
	kFormatBC2UNormSRgb = 75,
	kFormatBC3Typeless = 76,
	kFormatBC3UNorm = 77,
	kFormatBC3UNormSRgb = 78,
	kFormatBC4Typeless = 79,
	kFormatBC4UNorm = 80,
	kFormatBC4SNorm = 81,
	kFormatBC5Typeless = 82,
	kFormatBC5UNorm = 83,
	kFormatBC5SNorm = 84,
	kFormatB5G6R5UNorm = 85,
	kFormatB5G5R5A1UNorm = 86,
	kFormatB8G8R8A8UNorm = 87,
	kFormatB8G8R8X8UNorm = 88,
	kFormatR10G10B10XRBiasA2UNorm = 89,
	kFormatB8G8R8A8Typeless = 90,
	kFormatB8G8R8A8UNormSRgb = 91,
	kFormatB8G8R8X8Typeless = 92,
	kFormatB8G8R8X8UNormSRgb = 93,
	kFormatBC6HTypeless = 94,
	kFormatBC6HUF16 = 95,
	kFormatBC6HSF16 = 96,
	kFormatBC7Typeless = 97,
	kFormatBC7UNorm = 98,
	kFormatBC7UNormSRgb = 99,
	kFormatAYUV = 100,
	kFormatY410 = 101,
	kFormatY416 = 102,
	kFormatNV12 = 103,
	kFormatP010 = 104,
	kFormatP016 = 105,
	kFormat420Opaque = 106,
	kFormatYUY2 = 107,
	kFormatY210 = 108,
	kFormatY216 = 109,
	kFormatNV11 = 110,
	kFormatAI44 = 111,
	kFormatIA44 = 112,
	kFormatP8 = 113,
	kFormatA8P8 = 114,
	kFormatB4G4R4A4UNorm = 115
};

enum PrimitiveTopology {
	kPrimTopologyUndefined = 0,
	kPrimTopologyPointList = 1,
	kPrimTopologyLineList = 2,
	kPrimTopologyLineStrip = 3,
	kPrimTopologyTriangleList = 4,
	kPrimTopologyTriangleStrip = 5,
	kPrimTopologyLineListAdj = 10,
	kPrimTopologyLineStripAdj = 11,
	kPrimTopologyTriangleListAdj = 12,
	kPrimTopologyTriangleStripAdj = 13,
	kPrimTopology1CtrlPointPatchList = 33,
	kPrimTopology2CtrlPointPatchList = 34,
	kPrimTopology3CtrlPointPatchList = 35,
	kPrimTopology4CtrlPointPatchList = 36,
	kPrimTopology5CtrlPointPatchList = 37,
	kPrimTopology6CtrlPointPatchList = 38,
	kPrimTopology7CtrlPointPatchList = 39,
	kPrimTopology8CtrlPointPatchList = 40,
	kPrimTopology9CtrlPointPatchList = 41,
	kPrimTopology10CtrlPointPatchList = 42,
	kPrimTopology11CtrlPointPatchList = 43,
	kPrimTopology12CtrlPointPatchList = 44,
	kPrimTopology13CtrlPointPatchList = 45,
	kPrimTopology14CtrlPointPatchList = 46,
	kPrimTopology15CtrlPointPatchList = 47,
	kPrimTopology16CtrlPointPatchList = 48,
	kPrimTopology17CtrlPointPatchList = 49,
	kPrimTopology18CtrlPointPatchList = 50,
	kPrimTopology19CtrlPointPatchList = 51,
	kPrimTopology20CtrlPointPatchList = 52,
	kPrimTopology21CtrlPointPatchList = 53,
	kPrimTopology22CtrlPointPatchList = 54,
	kPrimTopology23CtrlPointPatchList = 55,
	kPrimTopology24CtrlPointPatchList = 56,
	kPrimTopology25CtrlPointPatchList = 57,
	kPrimTopology26CtrlPointPatchList = 58,
	kPrimTopology27CtrlPointPatchList = 59,
	kPrimTopology28CtrlPointPatchList = 60,
	kPrimTopology29CtrlPointPatchList = 61,
	kPrimTopology30CtrlPointPatchList = 62,
	kPrimTopology31CtrlPointPatchList = 63,
	kPrimTopology32CtrlPointPatchList = 64,
};

enum LayoutInputClass {
	kLayoutInputPerVertexData = 0,
	kLayoutInputPerInstanceData = 1
};
struct LayoutInputElement {
	std::string SemanticName;
	unsigned SemanticIndex;
	ResourceFormat Format;
	unsigned InputSlot;
	unsigned AlignedByteOffset;
	LayoutInputClass InputSlotClass;
	unsigned InstanceDataStepRate;
};

enum ConstBufferElementType {
	kCBElementBool,
	kCBElementInt,
	kCBElementFloat,
	kCBElementFloat4,
	kCBElementMatrix,
	kCBElementStruct,
	kCBElementMax
};
struct ConstBufferDeclElement {
	std::string Name;
	ConstBufferElementType Type;
	size_t Size;
	size_t Count;
	size_t Offset;
public:
	ConstBufferDeclElement(const char* name, ConstBufferElementType type, size_t size, size_t count = 0, size_t offset = 0)
		:Name(name), Type(type), Size(size), Count(count), Offset(offset) {}
};
struct ConstBufferDecl {
	ConstBufferDeclElement& Add(const ConstBufferDeclElement& elem) {
		Elements.push_back(elem);
		return Elements.back();
	}
	ConstBufferDeclElement& Add(const ConstBufferDeclElement& elem, const ConstBufferDecl& subDecl) {
		Elements.push_back(elem);
		SubDecls.insert(std::make_pair(elem.Name, subDecl));
		return Elements.back();
	}
	ConstBufferDeclElement& Last() {
		return Elements.back();
	}
public:
	std::vector<ConstBufferDeclElement> Elements;
	std::map<std::string, ConstBufferDecl> SubDecls;
	size_t BufferSize = 0;
};

}