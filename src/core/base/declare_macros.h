#pragma once
#include "core/base/cppcoro.h"

#define interface struct
#define ThreadSafe
#define TemplateArgs template <typename... T>
#define TemplateT template <typename T>

#define DECLARE_STATIC_CREATE_CONSTRUCTOR(CLASS) \
	template <typename... T> static std::shared_ptr<CLASS> \
	Create(T &&...args) { return std::shared_ptr<CLASS>(new CLASS(std::forward<T>(args)...)); }

#define DECLARE_STATIC_TASK_CREATE_CONSTRUCTOR(CLASS, P1, P2, P3) DECLARE_STATIC_CREATE_CONSTRUCTOR(CLASS);

#define DECLARE_COTASK_FUNCTIONS(RETURN_TYPE, CREATE_FUNC, ...) \
	TemplateArgs RETURN_TYPE CREATE_FUNC##ITV/*InTaskVector*/(CoTaskVector& tasks, T &&...args) ##__VA_ARGS__ { \
		RETURN_TYPE result; \
		auto task = CREATE_FUNC(result, std::forward<T>(args)...); \
		tasks.push_back(task); \
		return result; \
	} \
	TemplateArgs CoTask<RETURN_TYPE> CREATE_FUNC##T/*Task*/(T &&...args) ##__VA_ARGS__  { \
		RETURN_TYPE result; \
		CoAwait CREATE_FUNC(result, std::forward<T>(args)...); \
		return result; \
	} \
	TemplateArgs RETURN_TYPE CREATE_FUNC##S/*Sync*/(T &&...args) ##__VA_ARGS__  { \
		RETURN_TYPE result; \
		auto task = CREATE_FUNC(result, std::forward<T>(args)...); \
		mir::coroutine::ExecuteTaskSync(*mIoService, task); \
		return result; \
	}