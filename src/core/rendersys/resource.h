#pragma once
#include <Windows.h>
#include <boost/noncopyable.hpp>
#include "core/base/std.h"
#include "core/mir_export.h"
#include "core/rendersys/predeclare.h"

namespace mir {

enum ResourceState {
	kResourceStateNone,
	kResourceStatePrepared,
	kResourceStateLoading,
	kResourceStateLoaded,
	kResourceStateUnloading
};
interface MIR_CORE_API IResource : boost::noncopyable {
public:
	bool IsLoaded() { return GetCurState() == kResourceStateLoaded; }
	bool IsLoading() { return GetCurState() == kResourceStateLoading; }
	bool IsPrepared() { return GetCurState() >= kResourceStatePrepared; }
public:
	virtual ResourceState GetCurState() = 0;
	virtual void SetCurState(ResourceState state) = 0;

	virtual void SetLoaded() = 0;
	virtual IUnknown** GetDeviceObject() = 0;

	virtual void AddOnLoadedListener(std::function<void(IResource*)> cb) = 0;
	virtual void CheckAndSetLoaded() = 0;
	virtual void AddDependency(IResourcePtr res) = 0;
};

struct MIR_CORE_API Resource : public IResource {
	IUnknown** mDeviceObj;
	ResourceState mCurState;
	std::vector<std::function<void(IResource*)>> mOnLoadeds;
	std::vector<ResourcePtr> mDepends;
public:
	Resource() :mCurState(kResourceStateNone),mDeviceObj(nullptr) {}
	Resource(IUnknown** deviceObj);
	ResourceState GetCurState() override { return mCurState; }
	void SetCurState(ResourceState state) override;

	void SetLoaded() override;
	IUnknown** GetDeviceObject() override;

	void AddOnLoadedListener(std::function<void(IResource*)> cb) override;
	void CheckAndSetLoaded() override;
	void AddDependency(IResourcePtr res) override;
protected:
	virtual bool CheckLoaded() const;
};

}