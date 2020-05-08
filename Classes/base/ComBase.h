#pragma once
#include "wrl/client.h"
using Microsoft::WRL::ComPtr;

template <class BASE, const IID* piid = &__uuidof(BASE) >
class ComBase : public BASE
{
protected:
	ULONG   m_nRefCount;
public:
	ComBase() : m_nRefCount(0) {}

	STDMETHODIMP QueryInterface(REFIID riid, void** ppv)
	{
		if (riid == IID_IUnknown || riid == *piid) {
			*ppv = this;
			AddRef();
			return S_OK;
		}
		else {
			return *ppv = 0, E_NOINTERFACE;
		}
	}

	STDMETHODIMP_(ULONG) AddRef() { return ++m_nRefCount; }

	STDMETHODIMP_(ULONG) Release()
	{
		if (!--m_nRefCount)
		{
			delete this;
			return 0;
		}
		return m_nRefCount;
	}

	virtual ~ComBase() {}
};

template <class BASE0, class BASE1, const IID* piid0 = &__uuidof(BASE0), const IID* piid1 = &__uuidof(BASE1)>
class ComBase1 : public BASE0, public BASE1
{
protected:
	ULONG   m_nRefCount;
public:
	ComBase1() : m_nRefCount(0) {}

	STDMETHODIMP QueryInterface(REFIID riid, void** ppv)
	{
		if (riid == IID_IUnknown || riid == *piid0 || riid == *piid1) {
			*ppv = this;
			AddRef();
			return S_OK;
		}
		else {
			return *ppv = 0, E_NOINTERFACE;
		}
	}

	STDMETHODIMP_(ULONG) AddRef() { return ++m_nRefCount; }

	STDMETHODIMP_(ULONG) Release()
	{
		if (!--m_nRefCount)
		{
			delete this;
			return 0;
		}
		return m_nRefCount;
	}

	virtual ~ComBase1() {}
};

template <class T>
class TComCast
{
	const ComPtr<T>& mPtr;
public:
	TComCast(const ComPtr<T>& ptr) :mPtr(ptr) {}

	template<class U> ComPtr<U> Cast() {
		ComPtr<U> ret;
		mPtr.As(&ret);
		assert(ret.Get() != nullptr);
		return ret;
	}

	template<class U> U* As() {
		return mPtr ? static_cast<U*>(mPtr.Get()) : nullptr;
	}
};
template<class T> TComCast<T> PtrCast(const ComPtr<T>& ptr) { return TComCast<T>(ptr); }
template<class T, class U> U* PtrCast(T* ptr) { 
	U* ret = nullptr;
	ptr->QueryInterface(__uuidof(U), (void**)&ret);
	return ret;
}

template<class T> ComPtr<T> MakePtr() { return ComPtr<T>(new T()); }
template<class T, class P0> ComPtr<T> MakePtr(P0 p0) { return ComPtr<T>(new T(p0)); }
template<class T, class P0, class P1> ComPtr<T> MakePtr(P0 p0, P1 p1) { return ComPtr<T>(new T(p0, p1)); }
template<class T, class P0, class P1, class P2> ComPtr<T> MakePtr(P0 p0, P1 p1, P2 p2) { return ComPtr<T>(new T(p0, p1, p2)); }
template<class T, class P0, class P1, class P2, class P3> ComPtr<T> MakePtr(P0 p0, P1 p1, P2 p2, P3 p3) { return ComPtr<T>(new T(p0, p1, p2, p3)); }



#define INHERIT_COM(X) //DECLSPEC_UUID(X)