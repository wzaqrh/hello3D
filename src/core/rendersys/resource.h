#pragma once
#include "core/base/std.h"

#ifdef USE_EXPORT_COM
typedef ComPtr<struct IResource> IResourcePtr;
typedef ComPtr<struct TResource> TResourcePtr;
#else
typedef std::shared_ptr<struct IResource> IResourcePtr;
typedef std::shared_ptr<struct TResource> TResourcePtr;
#endif

enum enResourceState {
	E_RES_STATE_NONE,
	E_RES_STATE_PREPARED,
	E_RES_STATE_LOADING,
	E_RES_STATE_LOADED,
	E_RES_STATE_UNLOADING
};
MIDL_INTERFACE("14542070-5390-402A-9655-0F162D1C6A74") 
IResource : public IUnknown {
public:
	bool IsLoaded() { return GetCurState() == E_RES_STATE_LOADED; }
	bool IsLoading() { return GetCurState() == E_RES_STATE_LOADING; }
	bool IsPrepared() { return GetCurState() >= E_RES_STATE_PREPARED; }
public:
	virtual STDMETHODIMP_(enResourceState) GetCurState() = 0;
	virtual STDMETHODIMP_(void) SetCurState(enResourceState state) = 0;

	virtual STDMETHODIMP_(void) SetLoaded() = 0;
	virtual STDMETHODIMP_(IUnknown**) GetDeviceObject() = 0;

	virtual STDMETHODIMP_(void) AddOnLoadedListener(std::function<void(IResource*)> cb) = 0;
	virtual STDMETHODIMP_(void) CheckAndSetLoaded() = 0;
	virtual STDMETHODIMP_(void) AddDependency(IResourcePtr res) = 0;
};

struct INHERIT_COM("20FB61DE-C191-489C-972E-D12D01A82ECF")
TResource : public ComBase<IResource> {
	IUnknown** mDeviceObj;
	enResourceState mCurState;
	std::vector<std::function<void(IResource*)>> OnLoadeds;
	std::vector<TResourcePtr> mDepends;
public:
	TResource() :mCurState(E_RES_STATE_NONE),mDeviceObj(nullptr) {}
	TResource(IUnknown** deviceObj);
	STDMETHODIMP_(enResourceState) GetCurState() override {
		return mCurState;
	}
	STDMETHODIMP_(void) SetCurState(enResourceState state) override;

	STDMETHODIMP_(void) SetLoaded() override;
	STDMETHODIMP_(IUnknown**) GetDeviceObject() override;

	STDMETHODIMP_(void) AddOnLoadedListener(std::function<void(IResource*)> cb) override;
	STDMETHODIMP_(void) CheckAndSetLoaded() override;
	STDMETHODIMP_(void) AddDependency(IResourcePtr res) override;
protected:
	virtual bool CheckLoaded() const;
};

