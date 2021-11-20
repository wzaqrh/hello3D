#pragma once
#include <Windows.h>
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#include "core/base/stl.h"
#include "core/rendersys/predeclare.h"

namespace mir {

enum ResourceState {
	kResourceStateNone,
	kResourceStatePrepared,
	kResourceStateLoading,
	kResourceStateLoaded,
	kResourceStateUnloading
};
interface MIR_CORE_API IResource : boost::noncopyable 
{
	virtual IObjectPtr AsObject() const = 0;

	virtual ResourceState GetCurState() = 0;
	virtual void SetCurState(ResourceState state) = 0;

	virtual void SetLoaded() = 0;
	virtual IUnknown** GetDeviceObject() = 0;

	virtual void AddOnLoadedListener(std::function<void(IResource*)> cb) = 0;
public:
	bool IsLoaded() { return GetCurState() == kResourceStateLoaded; }
	bool IsLoading() { return GetCurState() == kResourceStateLoading; }
	bool IsPreparedNeedLoading() { return GetCurState() == kResourceStatePrepared; }
};

template<class _Parent>
struct ImplementResource : public _Parent 
{
	static_assert(std::is_base_of<IResource, _Parent>::value, "");
public:
	ImplementResource() :mCurState(kResourceStateNone),mDeviceObj(nullptr) {}
	ImplementResource(IUnknown** deviceObj) :mCurState(kResourceStateNone), mDeviceObj(deviceObj) {}
	IObjectPtr AsObject() const override final { return mObject.lock(); }

	ResourceState GetCurState() override final { return mCurState; }
	void SetCurState(ResourceState state) override final {
		if (mCurState != state) {
			mCurState = state;
			if (mCurState == kResourceStateLoaded)
				SetLoaded();
		}
	}

	IUnknown** GetDeviceObject() override final { return mDeviceObj; }
	void SetLoaded() override final {
		mCurState = kResourceStateLoaded;

		std::vector<std::function<void(IResource*)>> cbs;
		cbs.swap(mOnLoadeds);
		for (auto& cb : cbs)
			cb(this);
	}

	void AddOnLoadedListener(std::function<void(IResource*)> cb) override final {
		mOnLoadeds.push_back(cb);
	}

	void Assign(ImplementResource& other) {
		this->mCurState = other.mCurState;
	}
protected:
	virtual void OnLoaded() {}
	void SetDeviceObject(IUnknown** deviceObj) {
		mDeviceObj = deviceObj;
	}
private:
	std::weak_ptr<IObject> mObject;
	IUnknown** mDeviceObj;
	ResourceState mCurState;
	std::vector<std::function<void(IResource*)>> mOnLoadeds;
};

template<class _Resource>
struct ResourceContainer {
	typedef std::future<_Resource> right_value;
	typedef std::shared_ptr<_Resource> value_type;
	typedef const value_type& const_reference;
public:
	ResourceContainer(const ResourceContainer& other) = delete;
	ResourceContainer& operator=(const ResourceContainer& other) = delete;
	ResourceContainer(value_type&& other) :mValue(std::forward<value_type>(other)) {}
	ResourceContainer(right_value&& other) :mAsyncSlot(std::forward<right_value>(other)) {}
	ResourceContainer& operator=(value_type&& other) {
		if (mAsyncSlot.valid()) 
			mAsyncSlot.get();//todo(fixed me): may blocked
		mValue = std::forward<value_type>(other);
		return *this;
	}
	ResourceContainer& operator=(right_value&& other) {
		mValue = nullptr;
		mAsyncSlot = std::forward<right_value>(other);
		return *this;
	}
	const_reference operator->() const {
		return Get();
	}
	bool IsReady() const {
		return WaitFor(0) != nullptr;
	}
	bool operator==(std::nullptr_t) const {
		return !IsReady();
	}
	bool operator!=(std::nullptr_t) const {
		return IsReady();
	}

	const_reference WaitFor(size_t duration = 0) {
		if (mValue == nullptr && mAsyncSlot.valid()) {
			std::future_status status = mAsyncSlot.wait_for(std::chrono::microseconds(duration));
			if (status == std::future_status::ready) {
				mValue = mAsyncSlot.get();
			}
		}
		return mValue;
	}
	const_reference Get() const {
		return mValue;
	}
private:
	std::future<value_type> mAsyncSlot;
	value_type mValue;
};

}

#define AsRes(A) A