#pragma once
#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include "core/rendersys/interface_type.h"

namespace mir {

typedef std::function<void(IResource* res, HRESULT hr)> ThreadPumpCallback;
void ResourceSetLoaded(IResource* res, HRESULT hr);

struct ThreadPumpEntry {
	IResourcePtr res;
	volatile HRESULT hr;
	ThreadPumpCallback callback;
public:
	void Clear();
	bool IsNull() const;
	bool IsAttached() const;
};
typedef std::shared_ptr<ThreadPumpEntry> ThreadPumpEntryPtr;

class ThreadPump
{
	std::vector<ThreadPumpEntryPtr> mEntries;
	ID3DX11ThreadPump* mThreadPump;
public:
	ThreadPump(ID3DX11ThreadPump* threadPump = nullptr);
	~ThreadPump();

	HRESULT AddWorkItem(IResourcePtr res, std::function<HRESULT(ID3DX11ThreadPump*, ThreadPumpEntryPtr entry)> addItemCB, ThreadPumpCallback callback=nullptr);
	HRESULT AddWorkItem(IResourcePtr res, ID3DX11DataLoader* loader, ID3DX11DataProcessor* processor, ThreadPumpCallback callback=nullptr);
	bool RemoveWorkItem(IResourcePtr res);
	void ClearWorkItems();
	void Update(float dt);
};
//typedef std::shared_ptr<ThreadPump> ThreadPumpPtr;

}

