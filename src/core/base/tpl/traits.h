#pragma once
#include "core/base/stl.h"

namespace mir {
namespace tpl {

#define TemplateArgs template <typename... T>
#define TemplateT template <typename T>
#define TemplateTArray template <typename T, size_t N>
#define TArray std::array<T, N>

template <typename T, size_t N> constexpr size_t array_size(const std::array<T,N>& value) { return N; }
template <typename T> constexpr size_t array_size(const T& value) { return 1; }

TemplateT struct has_function_valid : public std::false_type {};
TemplateT struct has_function_name : public std::false_type {};

TemplateT struct is_shared_ptr : std::false_type {};
TemplateT struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

#define call_memfun(ReturnElement, ElementFunc) \
	if constexpr (is_shared_ptr<value_type>::value) ReturnElement->ElementFunc;\
	else ReturnElement.ElementFunc
}
}
