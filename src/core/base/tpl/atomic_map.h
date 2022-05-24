#pragma once
#include <boost/assert.hpp>
#include "core/base/stl.h"

namespace mir {
namespace tpl {

#define ATOMIC_LOCK(LCK) while(LCK.test_and_set(std::memory_order_acquire)) ;
#define ATOMIC_UNLOCK(LCK) LCK.clear(std::memory_order_release)
#define ATOMIC_IS_LOCKED(LCK) LCK.test_and_set(std::memory_order_acquire)

struct AutoLock {
	AutoLock(std::atomic_flag& lock) :mLock(lock) { ATOMIC_LOCK(mLock); }
	~AutoLock() { ATOMIC_UNLOCK(mLock); }
	std::atomic_flag& mLock;
};

template<class KeyType, class ValueType> class AtomicMap {
public:
	using value_type = ValueType;
	using reference = ValueType&;
	using const_reference = const ValueType&;
	using iterator = typename std::map<KeyType, ValueType>::iterator;
	using const_iterator = typename std::map<KeyType, ValueType>::const_iterator;
	using pointer = ValueType*;
	using const_pointer = const ValueType*;
	using const_key_reference = const KeyType&;
public:
	void Clear() {
		ATOMIC_LOCK(mLock);
		mDic.clear();
		ATOMIC_UNLOCK(mLock);
	}
	void AddOrSet(const_key_reference key, const_reference value) {
		ATOMIC_LOCK(mLock);
		auto iter = mDic.find(key);
		if (iter != mDic.end()) iter->second = value;
		else mDic.insert(std::make_pair(key, value));
		ATOMIC_UNLOCK(mLock);
	}
	template<typename CreateValueFunc> void GetOrAdd(const_key_reference key, CreateValueFunc fn, reference result) {
		ATOMIC_LOCK(mLock);
		auto iter = mDic.find(key);
		if (iter != mDic.end()) result = iter->second;
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
		if (iter != mDic.end()) result = iter->second;
		else result = value_type();
		ATOMIC_UNLOCK(mLock);
		return std::move(result);
	}
	value_type operator()(const_key_reference key) const { return Get(key); }
public:
	void _Lock() { ATOMIC_LOCK(mLock); }
	void _Unlock() { ATOMIC_UNLOCK(mLock); }
	void _Clear() { mDic.clear(); }
	std::map<KeyType, ValueType>& _GetDic() { return mDic; }
	std::atomic_flag& _GetLock() const { return mLock; }
	bool _IsLocked() const { return ATOMIC_IS_LOCKED(mLock); }
private:
	std::map<KeyType, ValueType> mDic;
	mutable std::atomic_flag mLock = ATOMIC_FLAG_INIT;
};

}
}