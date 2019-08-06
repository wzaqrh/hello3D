#include "IResource.h"

/********** IResource **********/
IResource::IResource()
	:mCurState(E_RES_STATE_LOADED)
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

void IResource::AddOnLoadedListener(Listener lis)
{
	OnLoaded = lis;
}

void IResource::SetLoaded()
{
	mCurState = E_RES_STATE_LOADED;
	if (OnLoaded)
		OnLoaded(this);
	OnLoaded = nullptr;
}
