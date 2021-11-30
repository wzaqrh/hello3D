#include <catch.hpp>
#include <iostream>
#include "core/base/thread.h"

//#define ATOMIC_STATEMENT(LCK, STATEMENT) /*ATOMIC_LOCK(LCK);*/ STATEMENT/*; ATOMIC_UNLOCK(LCK);*/

TEST_CASE("ATOMIC MACROS", "[ATOMIC_STATEMENT]") 
{
	std::atomic<bool> lock = false;

	auto print = [](size_t threadId, size_t count) {
		for (size_t i = 0; i < count; ++i) {
			std::cout << "thread t" << threadId << " " << i << std::endl;
			std::this_thread::sleep_for(std::chrono::microseconds(50));
		}
	};

	std::thread t0([&]() {
		ATOMIC_STATEMENT(lock, print(0, 200));
	});
	std::thread t1([&]() {
		ATOMIC_STATEMENT(lock, print(1, 200));
	});
	std::thread t2([&]() {
		ATOMIC_STATEMENT(lock, print(2, 200));
	});

	t0.join();
	t1.join();
	t2.join();
}