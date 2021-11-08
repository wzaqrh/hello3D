#include "core/rendersys/resource.h"

namespace mir {

Resource::Resource(IUnknown** deviceObj)
	:mCurState(kResourceStateNone)
	,mDeviceObj(deviceObj)
{
}

void Resource::SetCurState(ResourceState state)
{
	if (mCurState != state) {
		mCurState = state;
		if (mCurState == kResourceStateLoaded)
			SetLoaded();
	}
}

IUnknown** Resource::GetDeviceObject()
{
	return mDeviceObj;
}

void Resource::AddOnLoadedListener(std::function<void(IResource*)> lis)
{
	mOnLoadeds.push_back(lis);
}

void Resource::SetLoaded()
{
	mCurState = kResourceStateLoaded;

	std::vector<std::function<void(IResource*)>> cbs;
	cbs.swap(mOnLoadeds);
	for (auto& cb : cbs)
		cb(this);
}

bool Resource::CheckLoaded() const
{
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

void Resource::CheckAndSetLoaded()
{
	if (!IsLoaded() && CheckLoaded()) {
		SetLoaded();
	}
}

void Resource::AddDependency(IResourcePtr res)
{
	mDepends.push_back(std::static_pointer_cast<Resource>(res));
	res->AddOnLoadedListener([=](IResource* res) {
		CheckAndSetLoaded();
	});
}

}