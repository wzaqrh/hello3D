#pragma once
#include "core/rendersys/predeclare.h"
#include "core/base/base_type.h"
#include "core/base/template/container_adapter.h"
#include "core/resource/resource.h"

namespace mir {

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
enum AddressMode {
	kAddressUnkown = 0,
	kAddressWrap = 1,
	kAddressMirror = 2,
	kAddressClamp = 3,
	kAddressBorder = 4,
	kAddressMirrorOnce = 5
};
struct SamplerDesc {
	static SamplerDesc Make(SamplerFilterMode filter, CompareFunc cmpFunc,
		AddressMode addrU = kAddressWrap, AddressMode addrV = kAddressWrap, AddressMode addrW = kAddressWrap) {
		return SamplerDesc{ filter, cmpFunc, addrU, addrV, addrW };
	}
public:
	SamplerFilterMode Filter;
	CompareFunc CmpFunc;
	AddressMode AddressU, AddressV, AddressW;
};

interface ISamplerState : public IResource
{
};

}