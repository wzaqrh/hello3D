#pragma once
#include <boost/asio.hpp>
#include "core/base/stl.h"
#include "core/base/declare_macros.h"

namespace mir {

class ThreadPool
{
public:
	ThreadPool(int threadNum) {
		Pool = std::make_unique<boost::asio::thread_pool>(threadNum);
	}
	~ThreadPool() {
		Pool->stop();
		Pool = nullptr;
	}
	TemplateArgs void Post(T &&...args) {
		boost::asio::post(*Pool, std::forward<T>(args)...);
	}
private:
	std::unique_ptr<boost::asio::thread_pool> Pool;
};

#define ATOMIC_LOCK(LCK) for (bool expected = false; !LCK.compare_exchange_strong(expected, true, std::memory_order_acq_rel); expected = true);
#define ATOMIC_UNLOCK(LCK) LCK.store(false, std::memory_order_release);

#define ATOMIC_STATEMENT(LCK, STATEMENT) do { ATOMIC_LOCK(LCK); STATEMENT; ATOMIC_UNLOCK(LCK); } while(0)

}
