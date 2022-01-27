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
	TemplateArgs RETURN_TYPE CREATE_FUNC##Sync(T &&...args) ##__VA_ARGS__  { return CREATE_FUNC(__LaunchSync__, std::forward<T>(args)...); }\
	TemplateArgs RETURN_TYPE CREATE_FUNC##Async(T &&...args) ##__VA_ARGS__ { return CREATE_FUNC(__LaunchAsync__, std::forward<T>(args)...); }

#define DECLARE_STATIC_TASK_CREATE_CONSTRUCTOR(CLASS, PARAM1, PARAM2) \
	TemplateArgs static cppcoro::shared_task<std::shared_ptr<CLASS>> Create(PARAM1 param1, PARAM2 param2, T &&...args) { \
		std::shared_ptr<CLASS> ret = std::shared_ptr<CLASS>(new CLASS(param1, param2)); \
		if (co_await ret->Init(std::forward<T>(args)...)) { co_return ret; } \
		else { co_return nullptr; } \
	}



