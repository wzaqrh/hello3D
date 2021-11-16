#pragma once
#include <windows.h>
#include <d3d11.h>
#include <d3d9.h>
#include <string>
#include "core/mir_export.h"

namespace mir {
namespace debug {
	
class MIR_CORE_API TimeProfile {
	std::string mName;
	unsigned int mCurTime;
public:
	TimeProfile(const std::string& name);
	~TimeProfile();
};

class MIR_CORE_API Timer {
	double mLastTime = 0.0;
public:
	double mDeltaTime = 0.0;
public:
	double Update();
};

bool CheckHResultFailed(HRESULT result);

void SetDebugName(ID3D11DeviceChild* child, const std::string& name);

void Log(const char* msg);
void Log(const D3DCAPS9& caps);

}
}

#ifdef _DEBUG
#define CheckHR(HR)					mir::debug::CheckHResultFailed(HR)
#define SET_DEBUG_NAME(A, NAME)		mir::debug::SetDebugName(A, NAME)
#define TIME_PROFILE(NAME)			mir::debug::TimeProfile NAME(#NAME)
#define TIME_PROFILE2(NAME1,NAME2)	mir::debug::TimeProfile NAME(#NAME1+(":"+NAME2))
#else
#define CheckHR(HR)					FAILED(HR)
#define SET_DEBUG_NAME(A,NAME)
#define TIME_PROFILE(NAME)
#define TIME_PROFILE2(NAME1,NAME2)
#endif