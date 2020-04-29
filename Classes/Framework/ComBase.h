#pragma once
#include "wrl/client.h"
using Microsoft::WRL::ComPtr;

template < class BASE_INTERFACE, const IID* piid = &__uuidof(BASE_INTERFACE) >
class ComBase :
	public BASE_INTERFACE
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

	STDMETHODIMP_(ULONG) AddRef()
	{
		return ++m_nRefCount;
	}

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

template <class T>
class TComCast
{
	ComPtr<T> mPtr;
public:
	TComCast(ComPtr<T> ptr) :mPtr(ptr) {}

	template<class U> ComPtr<U> Cast() {
		ComPtr<U> ret;
		mPtr.As(&ret);
		assert(ret.Get() != nullptr);
		return ret;
	}

	template<class U> U* As() {
		return static_cast<U*>(mPtr.Get());
	}
};
template<class T> TComCast<T> PtrCast(ComPtr<T> ptr) { return TComCast<T>(ptr); }

template<class T> ComPtr<T> MakePtr() { return ComPtr<T>(new T()); }
template<class T, class P0> ComPtr<T> MakePtr(P0 p0) { return ComPtr<T>(new T(p0)); }
template<class T, class P0, class P1> ComPtr<T> MakePtr(P0 p0, P1 p1) { return ComPtr<T>(new T(p0, p1)); }
template<class T, class P0, class P1, class P2> ComPtr<T> MakePtr(P0 p0, P1 p1, P2 p2) { return ComPtr<T>(new T(p0, p1, p2)); }
template<class T, class P0, class P1, class P2, class P3> ComPtr<T> MakePtr(P0 p0, P1 p1, P2 p2, P3 p3) { return ComPtr<T>(new T(p0, p1, p2, p3)); }

#define INHERIT_COM(X) //DECLSPEC_UUID(X)