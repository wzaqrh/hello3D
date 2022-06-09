#include "core/base/cppcoro.h"

namespace mir {
namespace coroutine {

#if !defined MIR_CPPCORO_DISABLED
void ExecuteTaskSync(cppcoro::io_service& ioService, const CoTask<bool>& task) {
	auto scopedTask = [](cppcoro::io_service& ioService, const CoTask<bool>& task)->CoTask<void> {
		cppcoro::io_work_scope scope(ioService);
		co_await task;
	};
	auto processEvent = [](cppcoro::io_service& ioService)->CoTask<bool> {
		ioService.process_events();
		CoReturn true;
	};
	cppcoro::sync_wait(cppcoro::when_all_ready(scopedTask(ioService, task), processEvent(ioService)));
	ioService.reset();
}

void ExecuteTaskSync(cppcoro::io_service& ioService, const CoTask<void>& task)
{
	ExecuteTaskSync(ioService, [&task]()->CoTask<bool> {
		CoAwait task;
		CoReturn true;
	}());
}

#endif

}
}