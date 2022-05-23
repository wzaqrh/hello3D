#pragma once
#include "core/base/tpl/vector.h"
#include "core/rendersys/predeclare.h"
#include "core/rendersys/base/compare_func.h"
#include "core/resource/resource.h"

namespace mir {

enum SamplerFilterModeMask 
{
	kSFMMBase = 0,
	kSFMMCmp = 0x80,
	kSFMMMinimum = 0x100,
	kSFMMMaximum = 0x180,
	kSFMMAnisotropic = 0x55,
	kSFMMLinear_Mip = 0x01,
	kSFMMLinear_Mag = 0x04,
	kSFMMLinear_Min = 0x10,
};

enum SamplerFilterMode 
{
	kSamplerFilterMinMagMipPoint = kSFMMBase,
	kSamplerFilterMinMagPointMipLinear = kSFMMLinear_Mip,
	kSamplerFilterMinPointMagLinearMipPoint = kSFMMLinear_Mag,
	kSamplerFilterMinPointMagMipLinear = kSFMMLinear_Mag | kSFMMLinear_Mip,
	kSamplerFilterMinLinearMagMipPoint = kSFMMLinear_Min,
	kSamplerFilterMinLinearMagPointMipLinear = kSFMMLinear_Min | kSFMMLinear_Mip,
	kSamplerFilterMinMagLinearMipPoint = kSFMMLinear_Min | kSFMMLinear_Mag,
	kSamplerFilterMinMagMipLinear = kSFMMLinear_Min | kSFMMLinear_Mag | kSFMMLinear_Mip,
	kSamplerFilterAnisotropic = kSFMMAnisotropic,
	
	kSamplerFilterCmpMinMagMipPoint = kSFMMCmp,
	kSamplerFilterCmpMinMagPointMipLinear = kSamplerFilterMinMagPointMipLinear | kSFMMCmp,
	kSamplerFilterCmpMinPointMagLinearMipPoint = kSamplerFilterMinPointMagLinearMipPoint | kSFMMCmp,
	kSamplerFilterCmpMinPointMagMipLinear = kSamplerFilterMinPointMagMipLinear | kSFMMCmp,
	kSamplerFilterCmpMinLinearMagMipPoint = kSamplerFilterMinLinearMagMipPoint | kSFMMCmp,
	kSamplerFilterCmpMinLinearMagPointMipLinear = kSamplerFilterMinLinearMagPointMipLinear | kSFMMCmp,
	kSamplerFilterCmpMinMagLinearMipPoint = kSamplerFilterMinMagLinearMipPoint | kSFMMCmp,
	kSamplerFilterCmpMinMagMipLinear = kSamplerFilterMinMagMipLinear | kSFMMCmp,
	kSamplerFilterCmpAnisotropic = kSamplerFilterAnisotropic | kSFMMCmp,

	kSamplerFilterMinimumMinMagMipPoint = kSFMMMinimum,
	kSamplerFilterMinimumMinMagPointMipLinear = kSamplerFilterMinMagPointMipLinear | kSFMMMinimum,
	kSamplerFilterMinimumMinPointMagLinearMipPoint = kSamplerFilterMinPointMagLinearMipPoint | kSFMMMinimum,
	kSamplerFilterMinimumMinPointMagMipLinear = kSamplerFilterMinPointMagMipLinear | kSFMMMinimum,
	kSamplerFilterMinimumMinLinearMagMipPoint = kSamplerFilterMinLinearMagMipPoint | kSFMMMinimum,
	kSamplerFilterMinimumMinLinearMagPointMipLinear = kSamplerFilterMinLinearMagPointMipLinear | kSFMMMinimum,
	kSamplerFilterMinimumMinMagLinearMipPoint = kSamplerFilterMinMagLinearMipPoint | kSFMMMinimum,
	kSamplerFilterMinimumMinMagMipLinear = kSamplerFilterMinMagMipLinear | kSFMMMinimum,
	kSamplerFilterMinimumAnisotropic = kSamplerFilterAnisotropic | kSFMMMinimum,

	kSamplerFilterMaximumMinMagMipPoint = kSFMMMaximum,
	kSamplerFilterMaximumMinMagPointMipLinear = kSamplerFilterMinMagPointMipLinear | kSFMMMaximum,
	kSamplerFilterMaximumMinPointMagLinearMipPoint = kSamplerFilterMinPointMagLinearMipPoint | kSFMMMaximum,
	kSamplerFilterMaximumMinPointMagMipLinear = kSamplerFilterMinPointMagMipLinear | kSFMMMaximum,
	kSamplerFilterMaximumMinLinearMagMipPoint = kSamplerFilterMinLinearMagMipPoint | kSFMMMaximum,
	kSamplerFilterMaximumMinLinearMagPointMipLinear = kSamplerFilterMinLinearMagPointMipLinear | kSFMMMaximum,
	kSamplerFilterMaximumMinMagLinearMipPoint = kSamplerFilterMinMagLinearMipPoint | kSFMMMaximum,
	kSamplerFilterMaximumMinMagMipLinear = kSamplerFilterMinMagMipLinear | kSFMMMaximum,
	kSamplerFilterMaximumAnisotropic = kSamplerFilterAnisotropic | kSFMMMaximum
};

enum AddressMode 
{
	kAddressUnkown = 0,
	kAddressWrap = 1,
	kAddressMirror = 2,
	kAddressClamp = 3,
	kAddressBorder = 4,
	kAddressMirrorOnce = 5
};

struct SamplerDesc 
{
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