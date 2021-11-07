#pragma once
#include "core/base/std.h"

namespace mir {

typedef std::shared_ptr<struct IResource> IResourcePtr;
typedef std::shared_ptr<struct Resource> ResourcePtr;

enum ResourceState {
	kResourceStateNone,
	kResourceStatePrepared,
	kResourceStateLoading,
	kResourceStateLoaded,
	kResourceStateUnloading
};
interface IResource  {
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

struct Resource : public IResource {
	IUnknown** mDeviceObj;
	ResourceState mCurState;
	std::vector<std::function<void(IResource*)>> OnLoadeds;
	std::vector<ResourcePtr> mDepends;
public:
	Resource() :mCurState(kResourceStateNone),mDeviceObj(nullptr) {}
	Resource(IUnknown** deviceObj);
	ResourceState GetCurState() override {
		return mCurState;
	}
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