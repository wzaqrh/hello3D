#include "IResource.h"

/********** IResource **********/
TResource::TResource(IUnknown** deviceObj)
	:mCurState(E_RES_STATE_NONE)
	,mDeviceObj(deviceObj)
{
}

void TResource::SetCurState(enResourceState state)
{
	if (mCurState != state) {
		mCurState = state;
		if (mCurState == E_RES_STATE_LOADED)
			SetLoaded();
	}
}

IUnknown** TResource::GetDeviceObject()
{
	return mDeviceObj;
}

void TResource::AddOnLoadedListener(std::function<void(IResource*)> lis)
{
	OnLoadeds.push_back(lis);
}

void TResource::SetLoaded()
{
	mCurState = E_RES_STATE_LOADED;

	std::vector<std::function<void(IResource*)>> cbs;
	cbs.swap(OnLoadeds);
	for (auto& cb : cbs)
		cb(this);
}

bool TResource::CheckLoaded() const
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

void TResource::CheckAndSetLoaded()
{
	if (!IsLoaded() && CheckLoaded()) {
		SetLoaded();
	}
}

void TResource::AddDependency(IResourcePtr res)
{
	mDepends.push_back(PtrCast(res).As1<TResource>());
	res->AddOnLoadedListener([=](IResource* res) {
		CheckAndSetLoaded();
	});
}
