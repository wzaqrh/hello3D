#pragma once
#include "core/base/stl.h"

namespace mir {

struct Data {
	static Data MakeNull() { return Data{ nullptr, 0 }; }

	template<class T> static Data MakeSize(const T& v) { return Data{ nullptr, sizeof(v) }; }
	template<class T> static Data MakeSize(const std::vector<T>& v) { return Data{ nullptr, sizeof(T) * v.size() }; }
	template<class T> static Data MakeSize(std::vector<T>&& v) { static_assert(false, ""); }
	template<class T, size_t _ArraySize> static Data MakeSize(const std::array<T, _ArraySize>& v) {
		return Data{ nullptr, sizeof(T) * _ArraySize };
	}
	template<class T, size_t _ArraySize> static Data MakeSize(std::array<T, _ArraySize>&& v) { static_assert(false, ""); }
	static Data MakeSize(size_t size) { return Data{ nullptr, size }; }

	template<class T> static Data Make(const T& v) { return Data{ (void*)&v, sizeof(v) }; }
	template<class T> static Data Make(const std::vector<T>& v) { return Data{ (void*)&v[0], sizeof(T) * v.size() }; }
	template<class T> static Data Make(std::vector<T>&& v) { static_assert(false, ""); }
	template<class T, size_t _ArraySize> static Data Make(const std::array<T, _ArraySize>& v) {
		return Data{ (void*)&v[0], sizeof(T) * _ArraySize };
	}
	template<class T, size_t _ArraySize> static Data Make(std::array<T, _ArraySize>&& v) { static_assert(false, ""); }
	static Data Make(const void* data, size_t size) { return Data{ data, size }; }

	bool NotNull() const { return Bytes != nullptr; }
public:
	const void* Bytes;
	size_t Size;
};

}