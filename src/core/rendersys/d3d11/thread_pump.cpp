#include "thread_pump.h"
#include "core/rendersys/resource.h"
#include "core/base/utility.h"

namespace mir {

void ResourceSetLoaded(IResource* res, HRESULT hr)
{
	if (! FAILED(hr))
		res->SetLoaded();
}

/********** TThreadPumpEntry **********/
void ThreadPumpEntry::Clear()
{
	hr = 0;
	res = nullptr;
	callback = nullptr;
}

bool ThreadPumpEntry::IsNull() const
{
	return callback != nullptr;
}

bool ThreadPumpEntry::IsAttached() const
{
	return res->GetDeviceObject() != nullptr;
}

/********** TThreadPump **********/
ThreadPump::ThreadPump(ID3DX11ThreadPump* threadPump)
{
	if (threadPump == nullptr)
		CheckHR(D3DX11CreateThreadPump(2, 5, &threadPump));
	mThreadPump = threadPump;
}

ThreadPump::~ThreadPump()
{
	ClearWorkItems();
}

void ThreadPump::ClearWorkItems()
{
	CheckHR(mThreadPump->PurgeAllItems());
	mEntries.clear();
}

HRESULT ThreadPump::AddWorkItem(IResourcePtr res, ID3DX11DataLoader* loader, ID3DX11DataProcessor* processor, ThreadPumpCallback callback)
{
	ThreadPumpEntryPtr entry;
	for (size_t i = 0; i < mEntries.size(); ++i) {
		if (mEntries[i]->IsNull()) {
			entry = mEntries[i];
			break;
		}
	}

	if (entry == nullptr) {
		entry = std::make_shared<ThreadPumpEntry>();
		mEntries.push_back(entry);
	}

	entry->Clear();
	entry->res = res;
	entry->callback = callback;
	return mThreadPump->AddWorkItem(loader, processor, (HRESULT*)&entry->hr, (void**)res->GetDeviceObject());
}

HRESULT ThreadPump::AddWorkItem(IResourcePtr res, std::function<HRESULT(ID3DX11ThreadPump*, ThreadPumpEntryPtr entry)> addItemCB, ThreadPumpCallback callback)
{
	ThreadPumpEntryPtr entry;
	for (size_t i = 0; i < mEntries.size(); ++i) {
		if (mEntries[i]->IsNull()) {
			entry = mEntries[i];
			break;
		}
	}

	if (entry == nullptr) {
		entry = std::make_shared<ThreadPumpEntry>();
		mEntries.push_back(entry);
	}

	entry->Clear();
	entry->res = res;
	entry->callback = callback;
	return addItemCB(mThreadPump, entry);
}

bool ThreadPump::RemoveWorkItem(IResourcePtr res)
{
	bool result = false;
	for (size_t i = 0; i < mEntries.size(); ++i) {
		if (mEntries[i]->res == res) {
			mEntries[i]->Clear();
			result = true;
			break;
		}
	}
	return result;
}

void ThreadPump::Update(float dt)
{
	UINT ioCount = 0, proceeCount = 0, deviceObjectCount = 0;
	CheckHR(mThreadPump->GetQueueStatus(&ioCount, &proceeCount, &deviceObjectCount));

	if (ioCount > 0 || proceeCount > 0) {
		ioCount = ioCount;
	}

	if (deviceObjectCount > 0) {
		mThreadPump->ProcessDeviceWorkItems(deviceObjectCount);

		for (size_t i = 0; i < mEntries.size(); ++i) {
			auto entry = mEntries[i];
			if (!entry->IsNull() && entry->IsAttached()) {
				HRESULT hr = entry->hr;
				IResourcePtr res = entry->res;
				auto callback = entry->callback;
				entry->Clear();
				if (callback) callback(res.get(), hr);
			}
		}
	}
	else {
		for (size_t i = 0; i < mEntries.size(); ++i) {
			auto entry = mEntries[i];
			if (FAILED(entry->hr)) {
				HRESULT hr = entry->hr;
				IResourcePtr res = entry->res;
				auto callback = entry->callback;
				entry->Clear();
				if (callback) callback(entry->res.get(), hr);
			}
		}
	}
}

}