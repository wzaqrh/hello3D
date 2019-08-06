#pragma once
#include "TInterfaceType.h"

typedef std::function<void(IResource* res, HRESULT hr)> TThreadPumpCallback;
void ResourceSetLoaded(IResource* res, HRESULT hr);

struct TThreadPumpEntry {
	IResourcePtr res;
	volatile HRESULT hr;
	TThreadPumpCallback callback;
public:
	void Clear();
	bool IsNull() const;
	bool IsAttached() const;
};
typedef std::shared_ptr<TThreadPumpEntry> TThreadPumpEntryPtr;

class TThreadPump
{
	std::vector<TThreadPumpEntryPtr> mEntries;
	ID3DX11ThreadPump* mThreadPump;
public:
	TThreadPump(ID3DX11ThreadPump* threadPump = nullptr);
	~TThreadPump();

	HRESULT AddWorkItem(IResourcePtr res, std::function<HRESULT(ID3DX11ThreadPump*, TThreadPumpEntryPtr entry)> addItemCB, TThreadPumpCallback callback=nullptr);
	HRESULT AddWorkItem(IResourcePtr res, ID3DX11DataLoader* loader, ID3DX11DataProcessor* processor, TThreadPumpCallback callback=nullptr);
	bool RemoveWorkItem(IResourcePtr res);
	void ClearWorkItems();
	void Update(float dt);
};
typedef std::shared_ptr<TThreadPump> TThreadPumpPtr;

