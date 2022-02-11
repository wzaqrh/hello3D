#pragma once
#include <cppcoro/task.hpp>
#include <cppcoro/shared_task.hpp>
#include <cppcoro/static_thread_pool.hpp>
#include <cppcoro/io_service.hpp>
#include <cppcoro/sync_wait.hpp>
#include <cppcoro/resume_on.hpp>
#include <cppcoro/schedule_on.hpp>
#include <cppcoro/when_all.hpp>
#include <cppcoro/when_all_ready.hpp>
#include "core/mir_config.h"
#include "core/mir_export.h"
#include "core/base/stl.h"

#if !defined MIR_CPPCORO_DISABLED
#define CoTask cppcoro::shared_task
using CoTaskVector = std::vector<CoTask<bool>>;
#define CoAwait co_await
#define CoReturn co_return
#define CoReturnVoid co_return
#define WhenAll cppcoro::when_all
#define WhenAllReady cppcoro::when_all_ready
#else
template<class T> class DummyTask {
	T Value;
public:
	DummyTask() :Value(T()) {}
	DummyTask(const T& value) :Value(value) {}
	operator T() { return Value; }

	T& operator->() { return Value; }
	const T& operator->() const { return Value; }

	T& operator()() { return Value; }
	const T& operator()() const { return Value; }
};
template<> class DummyTask<void> {
	DummyTask<void>& operator()() { return *this; }
	const DummyTask<void>& operator()() const { return *this; }
};
#define CoTask DummyTask
using CoTaskVector = std::vector<CoTask<bool>>;
#define CoAwait
#define CoReturn return
#define CoReturnVoid return DummyTask<void>();
#define WhenAll()
#define WhenAllReady()
#endif

namespace mir {
namespace coroutine {
#if !defined MIR_CPPCORO_DISABLED

void MIR_CORE_API ExecuteTaskSync(cppcoro::io_service& ioService, const CoTask<bool>& task);

#else

inline void ExecuteTaskSync(cppcoro::io_service& ioService, const DummyTask<bool>& task) {}

#endif
}
}