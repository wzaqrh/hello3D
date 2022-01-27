#pragma once
#include <boost/assert.hpp>
#include "core/base/stl.h"

namespace mir {
namespace tpl {

#define ATOMIC_LOCK(LCK) for (bool expected = false; !LCK.compare_exchange_strong(expected, true, std::memory_order_acq_rel); expected = true);
#define ATOMIC_UNLOCK(LCK) LCK.store(false, std::memory_order_release);

template<class KeyType, class ValueType> class AtomicMap {
public:
	using value_type = ValueType;
	using reference = ValueType&;
	using const_reference = const ValueType&;
	using iterator = typename std::vector<value_type>::iterator;
	using const_iterator = typename std::vector<value_type>::const_iterator;
	using pointer = ValueType*;
	using const_pointer = const ValueType*;
	using const_key_reference = const KeyType&;
public:
	void AddOrSet(const_key_reference key, const_reference value) {
		ATOMIC_LOCK(mLock);
		auto iter = mDic.find(key);
		if (iter != mDic.end()) {
			iter->second = value;
		}
		else {
			mDic.insert(std::make_pair(key, value));
		}
		ATOMIC_UNLOCK(mLock);
	}
	template<typename CreateValueFunc> void GetOrAdd(const_key_reference key, CreateValueFunc fn, reference result) {
		ATOMIC_LOCK(mLock);
		auto iter = mDic.find(key);
		if (iter != mDic.end()) {
			result = iter->second;
		}
		else {
			result = fn();
			mDic.insert(std::make_pair(key, result));
		}
		ATOMIC_UNLOCK(mLock);
	}
	template<typename CreateValueFunc> value_type GetOrAdd(const_key_reference key, CreateValueFunc fn) {
		value_type result;
		GetOrAdd(key, fn, result);
		return std::move(result);
	}
	value_type Get(const_key_reference key) const {
		value_type result;
		ATOMIC_LOCK(mLock);
		auto iter = mDic.find(key);
		if (iter != mDic.end()) {
			result = iter->second;
		}
		else {
			result = value_type();
		}
		ATOMIC_UNLOCK(mLock);
		return std::move(result);
	}
	value_type operator()(const_key_reference key) const {
		return Get(key);
	}
private:
	std::map<KeyType, ValueType> mDic;
	std::atomic<bool> mLock = false;
};

}
}