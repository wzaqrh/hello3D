#pragma once
#include "core/base/std.h"

namespace mir {

typedef std::shared_ptr<struct IResource> IResourcePtr;
typedef std::shared_ptr<struct TResource> TResourcePtr;

enum enResourceState {
	E_RES_STATE_NONE,
	E_RES_STATE_PREPARED,
	E_RES_STATE_LOADING,
	E_RES_STATE_LOADED,
	E_RES_STATE_UNLOADING
};
interface IResource  {
public:
	bool IsLoaded() { return GetCurState() == E_RES_STATE_LOADED; }
	bool IsLoading() { return GetCurState() == E_RES_STATE_LOADING; }
	bool IsPrepared() { return GetCurState() >= E_RES_STATE_PREPARED; }
public:
	virtual enResourceState GetCurState() = 0;
	virtual void SetCurState(enResourceState state) = 0;

	virtual void SetLoaded() = 0;
	virtual IUnknown** GetDeviceObject() = 0;

	virtual void AddOnLoadedListener(std::function<void(IResource*)> cb) = 0;
	virtual void CheckAndSetLoaded() = 0;
	virtual void AddDependency(IResourcePtr res) = 0;
};

struct TResource : public IResource {
	IUnknown** mDeviceObj;
	enResourceState mCurState;
	std::vector<std::function<void(IResource*)>> OnLoadeds;
	std::vector<TResourcePtr> mDepends;
public:
	TResource() :mCurState(E_RES_STATE_NONE),mDeviceObj(nullptr) {}
	TResource(IUnknown** deviceObj);
	enResourceState GetCurState() override {
		return mCurState;
	}
	void SetCurState(enResourceState state) override;

	void SetLoaded() override;
	IUnknown** GetDeviceObject() override;

	void AddOnLoadedListener(std::function<void(IResource*)> cb) override;
	void CheckAndSetLoaded() override;
	void AddDependency(IResourcePtr res) override;
protected:
	virtual bool CheckLoaded() const;
};

}