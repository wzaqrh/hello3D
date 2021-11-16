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
	virtual void CheckAndSetLoaded() = 0;
	virtual void AddDependency(IResourcePtr res) = 0;
	virtual bool CheckLoaded() const = 0;
public:
	bool IsLoaded() { return GetCurState() == kResourceStateLoaded; }
	bool IsLoading() { return GetCurState() == kResourceStateLoading; }
	bool IsPrepared() { return GetCurState() >= kResourceStatePrepared; }
};

template<class Parent>
struct TImplResource : public Parent 
{
	static_assert(std::is_base_of<IResource, Parent>::value, "");
public:
	TImplResource() :mCurState(kResourceStateNone),mDeviceObj(nullptr) {}
	TImplResource(IUnknown** deviceObj) :mCurState(kResourceStateNone), mDeviceObj(deviceObj) {}
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
	void CheckAndSetLoaded() override final {
		if (!IsLoaded() && CheckLoaded()) {
			SetLoaded();
		}
	}
	void AddDependency(IResourcePtr res) override final {
		mDepends.push_back(std::static_pointer_cast<IResource>(res));
		res->AddOnLoadedListener([=](IResource* res) {
			CheckAndSetLoaded();
		});
	}
	bool CheckLoaded() const override final {
		if (!mDepends.empty()) {
			for (auto& depent : mDepends)
				if (!depent->CheckLoaded())
					return false;
			return true;
		}
		else {
			return mCurState == kResourceStateLoaded;
		}
	}

	void Assign(TImplResource& other, IRenderSystem& pRenderSys) {
		this->mDepends.clear();
		for (auto& depend : other.mDepends)
			this->AddDependency(depend);
		this->mCurState = other.mCurState;
	}
protected:
	void SetDeviceObject(IUnknown** deviceObj) {
		mDeviceObj = deviceObj;
	}
private:
	std::weak_ptr<IObject> mObject;
	IUnknown** mDeviceObj;
	ResourceState mCurState;
	std::vector<std::function<void(IResource*)>> mOnLoadeds;
	std::vector<IResourcePtr> mDepends;
};

}

#define AsRes(A) A