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
	ID3DX11ThreadPump* mThreadPump;
	std::vector<TThreadPumpEntryPtr> mEntries;
public:
	TThreadPump(ID3DX11ThreadPump* threadPump);
	~TThreadPump();

	void AddWorkItem(TTexturePtr res, ID3DX11DataLoader* loader, ID3DX11DataProcessor* processor, std::function<void(HRESULT, void*, TTexturePtr)> callback);
	bool RemoveWorkItem(TTexturePtr res);
	void ClearWorkItems();
	void Update(float dt);
};

