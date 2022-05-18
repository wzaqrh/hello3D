#pragma once
#include "core/base/stl.h"

namespace mir {

class DefferedSlot
{
	friend class DefferedSignal;
public:
	DefferedSlot() :mSignal(std::make_shared<unsigned>(0)) {}
	DefferedSlot(std::shared_ptr<unsigned> signal) :mSignal(signal) {}
	void Reset() { 
		mTimestamp = *mSignal; 
	}
	bool AcquireSignal() {
		bool signal = HasSignal();
		Reset();
		return signal;
	}

	bool HasSignal() const { return mTimestamp < *mSignal; }
	operator bool() { return HasSignal(); }
private:
	void SetSignal(std::shared_ptr<unsigned> signal) { mSignal = signal; }
private:
	std::shared_ptr<unsigned> mSignal;
	unsigned mTimestamp = 1;
};

class DefferedSignal
{
public:
	DefferedSignal() :mTimestamp(std::make_shared<unsigned>(1)) {}
	void Connect(DefferedSlot& slot) const { 
		slot.SetSignal(mTimestamp); 
	}
	DefferedSlot Connect() const { 
		DefferedSlot slot;
		Connect(slot); 
		return slot; 
	}
	void Emit() { (*mTimestamp)++; }
	void operator()() { Emit(); }
private:
	std::shared_ptr<unsigned> mTimestamp;
};

class DefferedConnctedSignal : public DefferedSignal
{
public:
	DefferedConnctedSignal() {
		Connect(Slot);
	}
	DefferedSlot Slot;
};

}