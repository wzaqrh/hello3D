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

namespace mir {
namespace coroutine {

inline void ExecuteTaskSync(cppcoro::io_service& ioService, const cppcoro::shared_task<void>& task) {
	auto scopedTask = [](cppcoro::io_service& ioService, const cppcoro::shared_task<void>& task)->cppcoro::shared_task<void> {
		cppcoro::io_work_scope scope(ioService);
		co_await task;
	};
	auto processEvent = [](cppcoro::io_service& ioService)->cppcoro::shared_task<void> {
		ioService.process_events();
		co_return;
	};
	cppcoro::sync_wait(cppcoro::when_all_ready(scopedTask(ioService, task), processEvent(ioService)));
	ioService.reset();
}

}
}