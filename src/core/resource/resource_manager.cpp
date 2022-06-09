#include <boost/assert.hpp>
#include <boost/asio.hpp>
#include <boost/format.hpp>
#include "core/base/debug.h"
#include "core/base/macros.h"
#include "core/base/thread.h"
#include "core/resource/resource_manager.h"
#include "core/resource/device_res_factory.h"
#include "core/resource/texture_factory.h"
#include "core/resource/program_factory.h"
#include "core/resource/material_factory.h"
#include "core/resource/assimp_factory.h"

#define USE_OIIO

namespace mir {

ResourceManager::ResourceManager(RenderSystem& renderSys, std::shared_ptr<cppcoro::io_service> ioService)
: mRenderSys(renderSys)
{
	mMaterialFac = CreateInstance<res::MaterialFactory>(*this);
	mProgramFac = CreateInstance<res::ProgramFactory>(*this);
	mTextureFac = CreateInstance<res::TextureFactory>(*this);
	mAiResFac = CreateInstance<res::AiResourceFactory>(*this);
	mDeviceResFac = CreateInstance<res::DeviceResFactory>(mRenderSys);

	mMainThreadId = std::this_thread::get_id();
	TIME_PROFILE((boost::format("resMng.main_tid %1%") % mMainThreadId).str());

	constexpr int CThreadPoolNumber = 8;
#if defined USE_OIIO
	mThreadPool = CreateInstance<cppcoro::static_thread_pool>(CThreadPoolNumber);
	mIoService = ioService;
#else
	mThreadPool = CreateInstance<cppcoro::static_thread_pool>(CThreadPoolNumber, ilInit, ilShutDown);
	mIoService = ioService;
	ilInit();
#endif	
}
ResourceManager::~ResourceManager()
{
	Dispose();
}
void ResourceManager::Dispose() ThreadSafe
{
	if (mThreadPool) {
		mThreadPool = nullptr;
	#if !defined USE_OIIO
		ilShutDown();
	#endif
	}
}

bool ResourceManager::IsCurrentInAsyncService() const
{
	return std::this_thread::get_id() != mMainThreadId;
}
CoTask<void> ResourceManager::SwitchToLaunchService(Launch launchMode)
{
#if !defined MIR_CPPCORO_DISABLED
	if (launchMode == LaunchAsync) {
		//if (! IsCurrentInAsyncService())
		CoAwait mThreadPool->schedule();
		BOOST_ASSERT(IsCurrentInAsyncService());
	}
	else {
		if (IsCurrentInAsyncService())
			CoAwait mIoService->schedule();
		BOOST_ASSERT(! IsCurrentInAsyncService());
	}
#endif
	CoReturnVoid;
}
CoTask<void> ResourceManager::WaitResComplete(IResourcePtr res, std::chrono::microseconds interval)
{
#if !defined MIR_CPPCORO_DISABLED
	bool asyncMode = IsCurrentInAsyncService();
	while (!res->IsLoadComplete()) {
		CoAwait mIoService->schedule_after(interval);
	}
	if (asyncMode) {
		CoAwait mThreadPool->schedule();
		BOOST_ASSERT(IsCurrentInAsyncService());
	}
#endif
	CoReturnVoid;
}

/********** Async Support **********/
CoTask<void> ResourceManager::UpdateFrame(float dt) ThreadSafe
{
#if MIR_MATERIAL_HOTLOAD
	if (mMaterialFac->PurgeOutOfDates()) {
		mProgramFac->PurgeAll();
	}	 
#endif
	CoReturn;
}

}