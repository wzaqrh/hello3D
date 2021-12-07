#pragma once
#include <boost/assert.hpp>
#include <windows.h>
#include <d3d11.h>
#include <d3d9.h>
#include "core/mir_export.h"
#include "core/predeclare.h"
#include "core/mir_config.h"
#include "core/base/stl.h"

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
bool CheckCompileFailed(HRESULT hr, ID3DBlob* pErrorBlob);
bool CheckCompileFailed(HRESULT hr, IBlobDataPtr data);

void MIR_CORE_API ResourceAddDebugDevice(IResourcePtr res, void* device);
void MIR_CORE_API SetDebugPrivData(IResourcePtr res, const std::string& privData);
void MIR_CORE_API SetDebugResourcePath(IResourcePtr res, const std::string& resPath);
void MIR_CORE_API SetDebugCallStack(IResourcePtr res, const std::string& callstack);

void Log(const char* msg);
void Log(const D3DCAPS9& caps);

}
}

#if defined _DEBUG
#define CheckHR(HR)					mir::debug::CheckHResultFailed(HR)
#define TIME_PROFILE(NAME)			mir::debug::TimeProfile NAME(#NAME)
#define TIME_PROFILE2(NAME1,NAME2)	mir::debug::TimeProfile NAME(#NAME1+(":"+NAME2))
#define DEBUG_LOG(MSG1)				mir::debug::Log(MSG1)
#else
#define CheckHR(HR)					FAILED(HR)
#define TIME_PROFILE(NAME)
#define TIME_PROFILE2(NAME1,NAME2)
#define DEBUG_LOG(MSG1)				
#endif

#if defined MIR_RESOURCE_DEBUG
#define DEBUG_RES_ADD_DEVICE(A, DEVICE) mir::debug::ResourceAddDebugDevice(A, DEVICE)
#define DEBUG_SET_PRIV_DATA(A, NAME)	mir::debug::SetDebugPrivData(A, NAME)
#define DEBUG_SET_RES_PATH(A, PATH)		mir::debug::SetDebugResourcePath(A, PATH)
#define DEBUG_SET_CALL(A, CALL)			mir::debug::SetDebugCallStack(A, CALL.CallStack)
#else
#define DEBUG_RES_ADD_DEVICE(A, DEVICE)
#define SET_DEBUG_NAME(A, NAME)		
#define DEBUG_SET_RES_PATH(A, PATH) 
#define DEBUG_SET_CALL(A, CALL)		
#endif

#define BOOST_ASSERT_IF_THEN_ELSE(COND, AND, OR) BOOST_ASSERT((COND) ? (AND) : (OR))
#define BOOST_ASSERT_IF_THEN(COND, AND) BOOST_ASSERT_IF_THEN_ELSE(COND, AND, true)

