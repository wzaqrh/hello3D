#pragma once
#include "std.h"

enum enResourceState {
	E_RES_STATE_NONE,
	E_RES_STATE_PREPARED,
	E_RES_STATE_LOADING,
	E_RES_STATE_LOADED,
	E_RES_STATE_UNLOADING
};
struct IResource {
	typedef std::function<void(IResource*)> Listener;
	enResourceState mCurState;
	std::vector<Listener> OnLoadeds;
	std::vector<std::shared_ptr<IResource>> mDepends;
public:
	IResource();
	bool IsLoaded() const {
		return mCurState == E_RES_STATE_LOADED;
	}
	bool IsLoading() const {
		return mCurState == E_RES_STATE_LOADING;
	}
	bool IsPrepared() const {
		return mCurState >= E_RES_STATE_PREPARED;
	}
	void SetLoaded();
public:
	void SetCurState(enResourceState state);
	
	virtual IUnknown*& GetDeviceObject();

	void AddOnLoadedListener(Listener lis);
	virtual bool CheckLoaded() const;
	void CheckAndSetLoaded();
	void AddDependency(std::shared_ptr<IResource> res);
};
typedef std::shared_ptr<IResource> IResourcePtr;