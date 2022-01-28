#include "core/base/cppcoro.h"

namespace mir {
namespace coroutine {

//void ExecuteTaskSync(cppcoro::io_service& ioService, const CoTask<bool>& task) {
//	auto scopedTask = [](cppcoro::io_service& ioService, const CoTask<bool>& task)->CoTask<void> {
//		cppcoro::io_work_scope scope(ioService);
//		co_await task;
//	};
//	auto processEvent = [](cppcoro::io_service& ioService)->CoTask<bool> {
//		ioService.process_events();
//		co_return;
//	};
//	cppcoro::sync_wait(cppcoro::when_all_ready(scopedTask(ioService, task), processEvent(ioService)));
//	ioService.reset();
//}

}
}