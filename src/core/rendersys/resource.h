#pragma once
#include <Windows.h>
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#include "core/base/stl.h"
#include "core/rendersys/predeclare.h"

namespace mir {

enum ResourceState {
	kResourceStateNone,
	kResourceStatePrepared,
	kResourceStateLoading,
	kResourceStateLoadedSuccess,
	kResourceStateLoadedFailed
};
interface MIR_CORE_API IResource : boost::noncopyable 
{
	virtual ResourceState GetCurState() const = 0;
	virtual void SetCurState(ResourceState state) = 0;
public:
	void SetPrepared() {
		SetCurState(kResourceStatePrepared);
	}
	void SetLoaded(bool sucess = true) {
		SetCurState(sucess ? kResourceStateLoadedSuccess : kResourceStateLoadedFailed);
	}

	bool IsPreparedNeedLoading() const { return GetCurState() == kResourceStatePrepared; }
	bool IsLoading() const { return GetCurState() == kResourceStateLoading; }
	bool IsLoadComplete() const { return IsLoaded() || IsLoadedFailed(); }
	bool IsLoaded() const { return GetCurState() == kResourceStateLoadedSuccess; }
	bool IsLoadedFailed() const { return GetCurState() == kResourceStateLoadedFailed; } 
};

template<class _Parent>
struct ImplementResource : public _Parent 
{
	static_assert(std::is_base_of<IResource, _Parent>::value, "");
public:
	ImplementResource() :mCurState(kResourceStateNone) {}

	ResourceState GetCurState() const override final { return mCurState; }
	void SetCurState(ResourceState state) override final {
		if (mCurState != state) {
			mCurState = state;
			if (mCurState == kResourceStateLoadedSuccess)
				OnLoaded();
		}
	}

	void Assign(const ImplementResource& other) {
		this->mCurState = other.mCurState;
	}
protected:
	virtual void OnLoaded() {}
private:
	ResourceState mCurState;
};

}

#define AsRes(A) A