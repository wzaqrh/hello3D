#include <windows.h>
#include <xnamath.h>
#include <catch.hpp>
#include <iostream>
#include <cppcoro/when_all_ready.hpp>
#include "test/unit_test/unit_test.h"

using namespace cppcoro;

#if !defined MIR_CPPCORO_DISABLED
struct test_io_service {
	std::shared_ptr<cppcoro::io_service> mIoService;
	std::string mFirst, mSecond;
	test_io_service() {
		mIoService = std::make_shared<cppcoro::io_service>();
	}

#define IO_SERVICE_MODE 2
	shared_task<void> GetFirst(std::string& result) {
	#if IO_SERVICE_MODE == 1
		cppcoro::io_work_scope ioScope(*mIoService);
		co_await mIoService->schedule_after(std::chrono::seconds(1));
	#elif IO_SERVICE_MODE == 2
		cppcoro::io_work_scope ioScope(*mIoService);
		co_await mIoService->schedule();
	#endif
		mFirst = std::string("hello ");
		result = mFirst;
		co_return;
	}
	shared_task<void> GetSecond(std::string& result) {
	#if IO_SERVICE_MODE == 1
		cppcoro::io_work_scope ioScope(*mIoService);
		co_await mIoService->schedule_after(std::chrono::seconds(1));
	#elif IO_SERVICE_MODE == 2
		cppcoro::io_work_scope ioScope(*mIoService);
		co_await mIoService->schedule();
	#endif
		mSecond = std::string("world");
		result = mSecond;
		co_return;
	}
	shared_task<std::string> GetSum() {
		std::string s1, s2;
	#if IO_SERVICE_MODE == 1
		co_await cppcoro::when_all_ready(GetFirst(s1), GetSecond(s2));
	#elif IO_SERVICE_MODE == 2
		co_await GetFirst(s1);
		co_await GetSecond(s2);
	#endif
		return s1 + s2;
	}
	CoTask<void> Execute(std::string& result) {
	#if IO_SERVICE_MODE
		cppcoro::io_work_scope ioScope(*mIoService);
	#endif
		result = co_await GetSum();
	}
	CoTask<void> ProcessEvents() {
		mIoService->process_events();
		co_return;
	}

	std::string operator()() {
		std::string str;
		cppcoro::sync_wait(cppcoro::when_all_ready(
			Execute(str)
		#if IO_SERVICE_MODE
			, ProcessEvents()
		#endif
		));
		return str;
	}
};
TEST_CASE("cppcoro co_await", "[cppcoro.co_await]") {
	std::cout << test_io_service()() << std::endl;
}

struct test_static_thread_pool {
	std::thread::id mMainThreadId;
	cppcoro::static_thread_pool mThreadPool;
	cppcoro::io_service mIoService;
	test_static_thread_pool() :mThreadPool(8) {
		mMainThreadId = std::this_thread::get_id();
	}
public:
	shared_task<std::string> get_desc_one__()
	{
		std::string result = "my name";
		CoReturn result;
	}
//#define USE_RESUME_ON 1
	shared_task<std::string> get_first_()
	{
		REQUIRE(std::this_thread::get_id() == mMainThreadId);

		co_await mThreadPool.schedule();
		REQUIRE(std::this_thread::get_id() != mMainThreadId);

		std::string result = "my name";
	#if !USE_RESUME_ON
		co_await mIoService.schedule();
	#endif
		return result;
	}
	shared_task<std::string> get_first() {
	#if USE_RESUME_ON 
		return get_first_() | cppcoro::resume_on(mIoService);
	#else
		return get_first_();
	#endif
	}

	shared_task<std::string> get_second()
	{
		REQUIRE(std::this_thread::get_id() == mMainThreadId);

		co_await mThreadPool.schedule();
		REQUIRE(std::this_thread::get_id() != mMainThreadId);

		std::string result = "my name";

		co_await mIoService.schedule();
		return result;
	}

	shared_task<std::string> get_desc_two() {
		std::string result = co_await get_first();
		result += co_await get_second();
		REQUIRE(std::this_thread::get_id() == mMainThreadId);

		return result + " is zelda";
	}

#define IOSEVICE_BLOCK
#if defined IOSEVICE_BLOCK
	CoTask<void> Execute(std::string& result) {
		cppcoro::io_work_scope ioScope(mIoService);
		result = co_await get_desc_two();
	}
	CoTask<void> ProcessEvents() {
		mIoService.process_events();
		CoReturn;
	}

	std::string operator()() {
		std::string str;
		cppcoro::sync_wait(cppcoro::when_all_ready(
			Execute(str)
			, ProcessEvents()
		));
		REQUIRE(std::this_thread::get_id() == mMainThreadId);
		return str;
	}
#else
	CoTask<void> Execute(std::string& result) {
		result = co_await get_desc_two();
	}

	std::string operator()() {
		std::string str;
		/*auto exe = */Execute(str);
		REQUIRE(std::this_thread::get_id() == mMainThreadId);
		/*while (!exe.is_ready()) {
			mIoService.process_pending_events();
			std::this_thread::sleep_for(std::chrono::microseconds(100));
		}*/
		return str;
	}
#endif
};
TEST_CASE("cppcoro static_thread_pool", "[cppcoro.static_thread_pool]") {
	std::cout << test_static_thread_pool()() << std::endl;
}

struct test_when_all {
	task<std::string> GetOne() {
		co_return std::string("1");
	}
	task<int> GetTwo() {
		co_return 1;
	}
	std::string operator()() {
		sync_wait(when_all(GetOne(), GetTwo()));
		return "1";
	}
};
TEST_CASE("cppcoro when_all", "[cppcoro.when_all]") {
	std::cout << test_when_all()() << std::endl;
}

struct test_fmap {
	task<std::string> one() {
		co_return "test_fmap";
	};
	std::string operator()() {
		auto converted_one = [&](std::string& result) {
		#if 0
			auto f = [&](std::string s)->bool {
				result = s;
				return !s.empty();
			};
		#else
			struct strToBool {
				std::string& Result;
				strToBool(std::string& result) :Result(result) {}
				bool operator()(std::string&& s) {
					Result = std::move(s);
					return ! Result.empty();
				}
			} f(result);
		#endif
			return one() | fmap(f);
		};
		std::string result;
		sync_wait(converted_one(result));
		return result;
	}
};
TEST_CASE("cppcoro fmap", "[cppcoro.fmap]") {
	std::cout << test_fmap()() << std::endl;
}
#endif