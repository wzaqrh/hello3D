#pragma once

#define interface struct

#define DECLARE_STATIC_CREATE_CONSTRUCTOR(CLASS) \
	template <typename... T> static std::shared_ptr<CLASS> \
	Create(T &&...args) { return std::shared_ptr<CLASS>(new CLASS(std::forward<T>(args)...)); }

#define DECLARE_LAUNCH_FUNCTIONS(RETURN_TYPE, CREATE_FUNC) \
	template <typename... T>\
	RETURN_TYPE CREATE_FUNC##Sync(T &&...args) {\
		return CREATE_FUNC(Launch::Sync, std::forward<T>(args)...);\
	}\
	template <typename... T>\
	RETURN_TYPE CREATE_FUNC##Async(T &&...args) {\
		return CREATE_FUNC(Launch::Async, std::forward<T>(args)...);\
	}

#define TemplateArgs template <typename... T>