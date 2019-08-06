#pragma once
#include "TInterfaceType.h"

struct TThreadPumpEntry {
	volatile HRESULT hr;
	volatile void* deviceObject;
	std::function<void(HRESULT, void*, TTexturePtr)> callback;
	TTexturePtr res;
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

	HRESULT AddWorkItem(TTexturePtr res, std::function<HRESULT(ID3DX11ThreadPump*, TThreadPumpEntryPtr entry)> addItemCB, std::function<void(HRESULT, void*, TTexturePtr)> callback=nullptr);
	void AddWorkItem(TTexturePtr res, ID3DX11DataLoader* loader, ID3DX11DataProcessor* processor, std::function<void(HRESULT, void*, TTexturePtr)> callback=nullptr);
	bool RemoveWorkItem(TTexturePtr res);
	void ClearWorkItems();
	void Update(float dt);
};
typedef std::shared_ptr<TThreadPump> TThreadPumpPtr;

