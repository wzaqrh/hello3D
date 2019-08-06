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
	Listener OnLoaded;
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
	void AddOnLoadedListener(Listener lis);
};