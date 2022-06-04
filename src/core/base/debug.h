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

void MIR_CORE_API ResourceAddDebugDevice(IResourcePtr res, void* device, const std::string& devName);
void MIR_CORE_API SetDebugPrivData(IResourcePtr res, const std::string& privData);
void MIR_CORE_API SetDebugResourcePath(IResourcePtr res, const std::string& resPath);
void MIR_CORE_API SetDebugCallStack(IResourcePtr res, const std::string& callstack);

#define LOG_LEVEL_VERVOSE 0
#define LOG_LEVEL_DEBUG	1
#define LOG_LEVEL_INFO 2
#define LOG_LEVEL_WARN 3
#define LOG_LEVEL_ERROR 4
#define LOG_LEVEL_NEVER 5
void Log(const std::string& msg, int level = LOG_LEVEL_VERVOSE);
void Log(const D3DCAPS9& caps);



}
}

#if defined MIR_LOG_LEVEL && MIR_LOG_LEVEL < LOG_LEVEL_NEVER
#define DEBUG_LOG(MSG1, LV)			mir::debug::Log(MSG1, LV)
#define DEBUG_LOG_VERVOSE(MSG1)		DEBUG_LOG(MSG1, LOG_LEVEL_VERVOSE)
#define DEBUG_LOG_DEBUG(MSG1)		DEBUG_LOG(MSG1, LOG_LEVEL_DEBUG)
#define DEBUG_LOG_INFO(MSG1)		DEBUG_LOG(MSG1, LOG_LEVEL_INFO)
#define DEBUG_LOG_WARN(MSG1)		DEBUG_LOG(MSG1, LOG_LEVEL_WARN)
#define DEBUG_LOG_ERROR(MSG1)		DEBUG_LOG(MSG1, LOG_LEVEL_ERROR)
#else
#define DEBUG_LOG(MSG1, LV)	
#define DEBUG_LOG_VERVOSE(MSG1)	
#define DEBUG_LOG_DEBUG(MSG1)
#define DEBUG_LOG_INFO(MSG1)
#define DEBUG_LOG_WARN(MSG1)
#define DEBUG_LOG_ERROR(MSG1)
#endif

#if defined _DEBUG
#define CheckHR(HR)					mir::debug::CheckHResultFailed(HR)
#else
#define CheckHR(HR)					FAILED(HR)	
#endif

#if defined MIR_TIME_DEBUG
#define TIME_PROFILE(MSG1)				mir::debug::TimeProfile tp0(MSG1)
#define TIME_PROFILE1(MSG1)				mir::debug::TimeProfile tp1(MSG1)
#define TIME_PROFILE2(MSG1)				mir::debug::TimeProfile tp2(MSG1)
#else
#define TIME_PROFILE(MSG1)	
#define TIME_PROFILE1(MSG1)
#define TIME_PROFILE2(MSG1)	
#endif

#if defined MIR_RESOURCE_DEBUG
#define DEBUG_RES_ADD_DEVICE(A, DEVICE, DEVNAME) mir::debug::ResourceAddDebugDevice(A, DEVICE, DEVNAME)
#define DEBUG_SET_PRIV_DATA(A, NAME)	mir::debug::SetDebugPrivData(A, NAME)
#define DEBUG_SET_RES_PATH(A, PATH)		mir::debug::SetDebugResourcePath(A, PATH)
#define DEBUG_SET_CALL(A, CALL)
#else
#define DEBUG_RES_ADD_DEVICE(A, DEVICE, DEVNAME)
#define SET_DEBUG_NAME(A, NAME)		
#define DEBUG_SET_RES_PATH(A, PATH) 
#define DEBUG_SET_CALL(A, CALL)		
#endif

#define BOOST_ASSERT_IF_THEN_ELSE(COND, AND, OR) BOOST_ASSERT((COND) ? (AND) : (OR))
#define BOOST_ASSERT_IF_THEN(COND, AND) BOOST_ASSERT_IF_THEN_ELSE(COND, AND, true)

#if defined MIR_COROUTINE_DEBUG
#define __VAR const auto&
#define COROUTINE_VARIABLES __VAR __this = this
#define COROUTINE_VARIABLES_1(a1) COROUTINE_VARIABLES; __VAR __##a1 = a1
#define COROUTINE_VARIABLES_2(a1, a2) COROUTINE_VARIABLES_1(a1); __VAR __##a2 = a2
#define COROUTINE_VARIABLES_3(a1, a2, a3) COROUTINE_VARIABLES_2(a1, a2); __VAR __##a3 = a3
#define COROUTINE_VARIABLES_4(a1, a2, a3, a4) COROUTINE_VARIABLES_3(a1, a2, a3); __VAR __##a4 = a4 
#define COROUTINE_VARIABLES_5(a1, a2, a3, a4, a5) COROUTINE_VARIABLES_4(a1, a2, a3, a4); __VAR __##a5 = a5
#define COROUTINE_VARIABLES_6(a1, a2, a3, a4, a5, a6) COROUTINE_VARIABLES_5(a1, a2, a3, a4, a5); __VAR __##a6 = a6
#define COROUTINE_VARIABLES_7(a1, a2, a3, a4, a5, a6, a7) COROUTINE_VARIABLES_6(a1, a2, a3, a4, a5, a6); __VAR __##a7 = a7
#else
#define COROUTINE_VARIABLES
#define COROUTINE_VARIABLES_1(a1)
#define COROUTINE_VARIABLES_2(a1, a2)
#define COROUTINE_VARIABLES_3(a1, a2, a3)
#define COROUTINE_VARIABLES_4(a1, a2, a3, a4)
#define COROUTINE_VARIABLES_5(a1, a2, a3, a4, a5)
#define COROUTINE_VARIABLES_6(a1, a2, a3, a4, a5, a6)
#define COROUTINE_VARIABLES_7(a1, a2, a3, a4, a5, a6, a7)
#endif