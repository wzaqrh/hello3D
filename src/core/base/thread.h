#pragma once
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <thread>
#include "core/base/stl.h"
#include "core/base/declare_macros.h"

namespace mir {

//#define UE_ASIO_THREAD_POOL
#if defined UE_ASIO_THREAD_POOL
class ThreadPool
{
public:
	ThreadPool(size_t threadNum) {
		mPool = std::make_unique<boost::asio::thread_pool>(threadNum);
	}
	~ThreadPool() {
		mPool->stop();
		mPool = nullptr;
	}
	TemplateArgs void Post(T &&...args) {
		boost::asio::post(*mPool, std::forward<T>(args)...);
	}
private:
	std::unique_ptr<boost::asio::thread_pool> mPool;
	boost::asio::io_service mIoService;
	std::unique_ptr<boost::asio::io_service::work> mIoServiceWork;
	boost::asio::detail::thread_group mThreadGroup;
};
#else
//https://blog.csdn.net/guotianqing/article/details/100730340
class ThreadPool
{
public:
	ThreadPool(size_t threadNum, 
		std::function<void()> onThreadStart = nullptr, 
		std::function<void()> onThreadFinish = nullptr) {
		mIoServiceWork = std::make_unique<boost::asio::io_service::work>(mIoService);
		mThreadGroup.create_threads([onThreadStart,onThreadFinish,this]() {
			std::thread::id thId = std::this_thread::get_id();
			if (onThreadStart) onThreadStart();
			mIoService.run();
			if (onThreadFinish) onThreadFinish();
		}, threadNum);
	}
	~ThreadPool() {
		Stop();
	}
	void Stop() {
		if (mIoServiceWork) {
			mIoService.stop();
			mThreadGroup.join();
			mIoServiceWork = nullptr;
		}
	}
	TemplateArgs void Post(T &&...args) {
		mIoService.post(std::forward<T>(args)...);
	}
private:
	boost::asio::io_service mIoService;
	std::unique_ptr<boost::asio::io_service::work> mIoServiceWork;
	boost::asio::detail::thread_group mThreadGroup;
};
#endif

#define ATOMIC_LOCK(LCK) for (bool expected = false; !LCK.compare_exchange_strong(expected, true, std::memory_order_acq_rel); expected = true);
#define ATOMIC_UNLOCK(LCK) LCK.store(false, std::memory_order_release);
#define ATOMIC_STATEMENT(LCK, STATEMENT) do { ATOMIC_LOCK(LCK); STATEMENT; ATOMIC_UNLOCK(LCK); } while(0)

}
