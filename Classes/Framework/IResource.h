#pragma once
#include "std.h"

enum enResourceState {
	E_RES_STATE_NONE,
	E_RES_STATE_PREPARED,
	E_RES_STATE_LOADING,
	E_RES_STATE_LOADED,
	E_RES_STATE_UNLOADING
};
MIDL_INTERFACE("14542070-5390-402A-9655-0F162D1C6A74") 
IResource : public IUnknown {
	typedef std::function<void(IResource*)> Listener;
	enResourceState mCurState;
	std::vector<Listener> OnLoadeds;
	std::vector<ComPtr<IResource>> mDepends;
public:
	IResource();
	bool IsLoaded() const { return mCurState == E_RES_STATE_LOADED; }
	bool IsLoading() const { return mCurState == E_RES_STATE_LOADING; }
	bool IsPrepared() const { return mCurState >= E_RES_STATE_PREPARED; }
	void SetLoaded();
public:
	void SetCurState(enResourceState state);
	
	virtual IUnknown*& GetDeviceObject();

	void AddOnLoadedListener(Listener lis);
	virtual bool CheckLoaded() const;
	void CheckAndSetLoaded();
	void AddDependency(ComPtr<IResource> res);
};
typedef ComPtr<IResource> IResourcePtr;