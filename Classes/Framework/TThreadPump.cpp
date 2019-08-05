#include "TThreadPump.h"
#include "Utility.h"

/********** TThreadPumpEntry **********/
void TThreadPumpEntry::Clear()
{
	hr = 0;
	deviceObject = nullptr;
	res = nullptr;
	callback = nullptr;
}

bool TThreadPumpEntry::IsNull() const
{
	return deviceObject != nullptr;
}

bool TThreadPumpEntry::IsAttached() const
{
	return callback != nullptr;
}

/********** TThreadPump **********/
TThreadPump::TThreadPump(ID3DX11ThreadPump* threadPump)
{
	mThreadPump = threadPump;
}

TThreadPump::~TThreadPump()
{
	ClearWorkItems();
}

void TThreadPump::ClearWorkItems()
{
	CheckHR(mThreadPump->PurgeAllItems());
	mEntries.clear();
}

void TThreadPump::AddWorkItem(TTexturePtr res, ID3DX11DataLoader* loader, ID3DX11DataProcessor* processor, std::function<void(HRESULT, void*, TTexturePtr)> callback)
{
	TThreadPumpEntryPtr entry;
	for (size_t i = 0; i < mEntries.size(); ++i) {
		if (mEntries[i]->IsNull()) {
			entry = mEntries[i];
			break;
		}
	}

	if (entry == nullptr) {
		entry = std::make_shared<TThreadPumpEntry>();
		mEntries.push_back(entry);
	}

	entry->Clear();
	entry->res = res;
	entry->callback = callback;
	CheckHR(mThreadPump->AddWorkItem(loader, processor, (HRESULT*)&entry->hr, (void**)entry->deviceObject));
}

bool TThreadPump::RemoveWorkItem(TTexturePtr res)
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

void TThreadPump::Update(float dt)
{
	UINT ioCount = 0, proceeCount = 0, deviceObjectCount = 0;
	CheckHR(mThreadPump->GetQueueStatus(&ioCount, &proceeCount, &deviceObjectCount));
	if (deviceObjectCount > 0) {
		mThreadPump->ProcessDeviceWorkItems(deviceObjectCount);

		for (size_t i = 0; i < mEntries.size(); ++i) {
			auto entry = mEntries[i];
			if (!entry->IsNull() && entry->deviceObject != nullptr) {
				HRESULT hr = entry->hr;
				void* deviceObject = (void*)entry->deviceObject;
				TTexturePtr res = entry->res;
				auto callback = entry->callback;
				entry->Clear();

				res->SetSRV((ID3D11ShaderResourceView*)deviceObject);
				callback(hr, deviceObject, res);
			}
		}
	}
}