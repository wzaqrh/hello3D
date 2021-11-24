#pragma once
#include <Windows.h>
#include <boost/noncopyable.hpp>
#include "core/mir_config.h"
#include "core/mir_export.h"
#include "core/base/stl.h"
#if defined MIR_RESOURCE_DEBUG
#include "core/base/launch.h"
#endif
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
#if defined MIR_RESOURCE_DEBUG
	std::string _ResourcePath;
	Launch _CallStack;
#endif
	virtual ~IResource() {}
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

	ResourceState GetCurState() const override final { return mCurState.load(std::memory_order_consume); }
	void SetCurState(ResourceState state) override final {
		if (mCurState.load(std::memory_order_consume) != state) {
			mCurState.store(state, std::memory_order_release);
			if (state == kResourceStateLoadedSuccess)
				OnLoaded();
		}
	}

	void Assign(const ImplementResource& other) {
		this->mCurState.store(other.mCurState.load(std::memory_order_consume), std::memory_order_release);
	}
protected:
	virtual void OnLoaded() {}
private:
	std::atomic<ResourceState> mCurState;
};

}

#define AsRes(A) A