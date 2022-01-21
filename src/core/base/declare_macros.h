#pragma once

#define interface struct

#define DECLARE_STATIC_CREATE_CONSTRUCTOR(CLASS) \
	template <typename... T> static std::shared_ptr<CLASS> \
	Create(T &&...args) { return std::shared_ptr<CLASS>(new CLASS(std::forward<T>(args)...)); }

#define DECLARE_LAUNCH_FUNCTIONS(RETURN_TYPE, CREATE_FUNC, ...) \
	template <typename... T> RETURN_TYPE CREATE_FUNC##Sync(T &&...args) ##__VA_ARGS__  { return CREATE_FUNC(__LaunchSync__, std::forward<T>(args)...); }\
	template <typename... T> RETURN_TYPE CREATE_FUNC##Async(T &&...args) ##__VA_ARGS__ { return CREATE_FUNC(__LaunchAsync__, std::forward<T>(args)...); }

#define TemplateArgs template <typename... T>
#define TemplateT template <typename T>
#define ConstTRef const T&

#define ThreadSafe