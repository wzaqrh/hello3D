#pragma once
#include "core/base/cppcoro.h"

#define interface struct
#define ThreadSafe
#define TemplateArgs template <typename... T>
#define TemplateT template <typename T>

#define DECLARE_STATIC_CREATE_CONSTRUCTOR(CLASS) \
	template <typename... T> static std::shared_ptr<CLASS> \
	Create(T &&...args) { return std::shared_ptr<CLASS>(new CLASS(std::forward<T>(args)...)); }

#define DECLARE_LAUNCH_FUNCTIONS(RETURN_TYPE, CREATE_FUNC, ...) \
	TemplateArgs RETURN_TYPE CREATE_FUNC##Sync(T &&...args) ##__VA_ARGS__  { \
		RETURN_TYPE result; \
		mir::coroutine::ExecuteTaskSync(*mIoService, CREATE_FUNC(__LaunchSync__, result, std::forward<T>(args)...)); \
		return result; \
	}\
	TemplateArgs CoTask<RETURN_TYPE> CREATE_FUNC##Async(T &&...args) ##__VA_ARGS__  { \
		RETURN_TYPE result; \
		CoAwait CREATE_FUNC(__LaunchAsync__, result, std::forward<T>(args)...); \
		return result; \
	}

#define DECLARE_STATIC_TASK_CREATE_CONSTRUCTOR(CLASS, P1, P2, P3) \
	DECLARE_STATIC_CREATE_CONSTRUCTOR(CLASS);/*\
	TemplateArgs static CoTask<std::shared_ptr<CLASS>> CreateAsync(P1 p1, P2 p2, P3 p3, T &&...args) { \
		std::shared_ptr<CLASS> ret = std::shared_ptr<CLASS>(new CLASS(p1, p2, p3)); \
		if (CoAwait ret->Init(std::forward<T>(args)...)) { CoReturn ret; } \
		else { CoReturn nullptr; } \
	}*/