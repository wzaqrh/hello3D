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

#if !defined MIR_CPPCORO_DISABLED
#define CoTask cppcoro::shared_task
#define CoAwait co_await
#define CoReturn co_return
#define CoReturnVoid co_return
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
#define CoAwait
#define CoReturn return
#define CoReturnVoid return DummyTask<void>();
#endif

namespace mir {
namespace coroutine {
#if !defined MIR_CPPCORO_DISABLED

inline void ExecuteTaskSync(cppcoro::io_service& ioService, const CoTask<void>& task) {
	auto scopedTask = [](cppcoro::io_service& ioService, const CoTask<void>& task)->CoTask<void> {
		cppcoro::io_work_scope scope(ioService);
		co_await task;
	};
	auto processEvent = [](cppcoro::io_service& ioService)->CoTask<void> {
		ioService.process_events();
		co_return;
	};
	cppcoro::sync_wait(cppcoro::when_all_ready(scopedTask(ioService, task), processEvent(ioService)));
	ioService.reset();
}

#else

inline void ExecuteTaskSync(cppcoro::io_service& ioService, const DummyTask<void>& task) {}

#endif
}
}