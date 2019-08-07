#include "IResource.h"

/********** IResource **********/
IResource::IResource()
	:mCurState(E_RES_STATE_NONE)
	//:mCurState(E_RES_STATE_LOADED)
{
}

void IResource::SetCurState(enResourceState state)
{
	if (mCurState != state) {
		mCurState = state;
		if (mCurState == E_RES_STATE_LOADED)
			SetLoaded();
	}
}

IUnknown* gDeviceObject;
IUnknown*& IResource::GetDeviceObject()
{
	return gDeviceObject;
}

void IResource::AddOnLoadedListener(Listener lis)
{
	OnLoadeds.push_back(lis);
}

void IResource::SetLoaded()
{
	mCurState = E_RES_STATE_LOADED;

	std::vector<Listener> cbs; 
	cbs.swap(OnLoadeds);
	for (auto& cb : cbs)
		cb(this);
}

bool IResource::CheckLoaded() const
{
	if (!mDepends.empty()) {
		for (auto& depent : mDepends)
			if (!depent->CheckLoaded())
				return false;
		return true;
	}
	else {
		return mCurState == E_RES_STATE_LOADED;
	}
}

void IResource::CheckAndSetLoaded()
{
	if (!IsLoaded() && CheckLoaded()) {
		SetLoaded();
	}
}

void IResource::AddDependency(std::shared_ptr<IResource> res)
{
	mDepends.push_back(res);
	res->AddOnLoadedListener([=](IResource* res) {
		CheckAndSetLoaded();
	});
}
