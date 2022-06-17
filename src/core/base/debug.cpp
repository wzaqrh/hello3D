#include <boost/assert.hpp>
#include <boost/format.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "core/base/debug.h"
#include "core/rendersys/blob.h"
#include "core/resource/resource.h"
#include "core/rendersys/d3d11/d3d_utils.h"
#include "core/rendersys/d3d11/render_system11.h"

namespace mir {
namespace debug {

/********** TTimeProfile **********/
TimeProfile::TimeProfile(const std::string& name)
{
	mName = name;
	mCurTime = timeGetTime();

	int t2 = mCurTime % 1000;
	int t1 = mCurTime / 1000 % 1000;
	int t0 = mCurTime / 1000 / 1000;	
	debug::Log((boost::format("%1% timestamp %2%,%3%,%4%") %mName %t0 %t1 %t2).str());
}

TimeProfile::~TimeProfile()
{
	debug::Log((boost::format("%1% takes %2%ms") %mName.c_str() %(timeGetTime()-mCurTime)).str());
}

/********** Timer **********/
double Timer::Update()
{
	double time = GetTickCount() / 1000.0;
	mDeltaTime = mLastTime == 0.0f ? 0.0 : time - mLastTime;
	mLastTime = time;
	return mDeltaTime;
}

/********** Log **********/
void Log(const std::string& msg, int level)
{
#if defined MIR_LOG_LEVEL
	if (level < MIR_LOG_LEVEL) return;
#endif
	char szInfo[260];
	sprintf(szInfo, "t%d ", std::this_thread::get_id());
	OutputDebugStringA(szInfo);
	OutputDebugStringA(msg.c_str());
	OutputDebugStringA("\n");
}

#if defined MIR_RESOURCE_DEBUG
/********** debug res **********/
#define IS_D3D11 dynamic_cast<RenderSystem11*>(res->_Debug._RenderSys)
void ResourceAddDebugDevice(IResourcePtr res, void* device, const std::string& devName) {
	if (IS_D3D11) d3d::ResourceAddDebugDevice(res, device, devName);
}
void SetDebugPrivData(IResourcePtr res, const std::string& privData) {
	if (IS_D3D11) d3d::SetDebugPrivData(res, privData);
}
void SetDebugResourcePath(IResourcePtr res, const std::string& resPath) {
	if (IS_D3D11) d3d::SetDebugResourcePath(res, resPath);
}
void SetDebugCallStack(IResourcePtr res, const std::string& callstack) {
	if (IS_D3D11) d3d::SetDebugCallStack(res, callstack);
}
#endif

}
}