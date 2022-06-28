#pragma once
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"

//#define MIR_RENDERSYS_DEBUG
//#define MIR_RESOURCE_DEBUG
#define MIR_TIME_DEBUG
//#define MIR_LOG_LEVEL 1
//#define MIR_COROUTINE_DEBUG 
//#define MIR_MATERIAL_HOTLOAD 1
//#define MIR_GRAPHICS_DEBUG 1
//#define MIR_MEMLEAK_DEBUG

namespace mir {

struct MIR_CORE_API Configure : boost::noncopyable {
	Configure();
	void SetShadowMode(int shadowMode) { _SHADOW_MODE = shadowMode; }
	int GetShadowMode() const { return _SHADOW_MODE; }
	bool IsShadowVSM() const;

	void SetReverseZ(bool reverseZ) { _REVERSE_Z = reverseZ; }
	bool IsReverseZ() const { return _REVERSE_Z; }
	
	void SetColorSpace(int colorSpace) { _COLORSPACE = colorSpace; }
	int GetColorSpace() const { return _COLORSPACE; }
	bool IsGammaSpace() const;

	void SetDebugChannel(int debugChannel);
public:
	int _SHADOW_MODE;
	int _REVERSE_Z;
	int _COLORSPACE;
	int _DEBUG_CHANNEL;
};

}