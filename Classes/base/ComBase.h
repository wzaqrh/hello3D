#pragma once
#include "std.h"
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

#ifdef USE_EXPORT_COM
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
#else
template <class T>
class TComCast
{
	const std::shared_ptr<T>& mPtr;
public:
	TComCast(const std::shared_ptr<T>& ptr) :mPtr(ptr) {}

	template<class U> std::shared_ptr<U> Cast() {
		return std::static_pointer_cast<U>(mPtr);
	}

	template<class U> std::shared_ptr<U> As1() {
		return std::static_pointer_cast<U>(mPtr);
	}
	template<class U> U* As() {
		return mPtr ? static_cast<U*>(mPtr.get()) : nullptr;
	}
};
template<class T> TComCast<T> PtrCast(const std::shared_ptr<T>& ptr) { 
	return TComCast<T>(ptr); 
}
template<class T, class U> U* PtrCast(T* ptr) {
	U* ret = nullptr;
	ptr->QueryInterface(__uuidof(U), (void**)&ret);
	return ret;
}

#define PtrRaw(T) T.get()

template<class T> struct ComDeleter { void operator()(T* p) { p->Release(); } };
template<class T, bool isCom> struct MakeShared {};
template<class T> struct MakeShared<T, true> {
	std::shared_ptr<T> operator()(T* rawPtr) { return std::shared_ptr<T>(rawPtr, ComDeleter<T>()); }
};
template<class T> struct MakeShared<T, false> {
	std::shared_ptr<T> operator()(T* rawPtr) { return std::shared_ptr<T>(rawPtr); }
};

template<class T, bool isCom> struct TakeOwn {};
template<class T> struct TakeOwn<T, true> {
	void operator()(T* rawPtr) { rawPtr->AddRef(); }
};
template<class T> struct TakeOwn<T, false> {
	void operator()(T* rawPtr) { }
};

template<class T> std::shared_ptr<T> MakePtr(T* rawPtr) {
	TakeOwn<T, std::is_base_of<IUnknown, T>::value>()(rawPtr);
	return MakeShared<T, std::is_base_of<IUnknown, T>::value>()(rawPtr);
}
template<class T> std::shared_ptr<T> MakePtr() { 
	T* rawPtr = new T();
	TakeOwn<T, std::is_base_of<IUnknown, T>::value>()(rawPtr);
	return MakeShared<T, std::is_base_of<IUnknown, T>::value>()(rawPtr);
}
template<class T, class P0> std::shared_ptr<T> MakePtr(P0 p0) { 
	T* rawPtr = new T(p0);
	TakeOwn<T, std::is_base_of<IUnknown, T>::value>()(rawPtr);
	return MakeShared<T, std::is_base_of<IUnknown, T>::value>()(rawPtr);
}
template<class T, class P0, class P1> std::shared_ptr<T> MakePtr(P0 p0, P1 p1) {
	T* rawPtr = new T(p0, p1);
	TakeOwn<T, std::is_base_of<IUnknown, T>::value>()(rawPtr);
	return MakeShared<T, std::is_base_of<IUnknown, T>::value>()(rawPtr);
}
template<class T, class P0, class P1, class P2> std::shared_ptr<T> MakePtr(P0 p0, P1 p1, P2 p2) {
	T* rawPtr = new T(p0, p1, p2);
	TakeOwn<T, std::is_base_of<IUnknown, T>::value>()(rawPtr);
	return MakeShared<T, std::is_base_of<IUnknown, T>::value>()(rawPtr);
}
template<class T, class P0, class P1, class P2, class P3> std::shared_ptr<T> MakePtr(P0 p0, P1 p1, P2 p2, P3 p3) { 
	T* rawPtr = new T(p0, p1, p2, p3);
	TakeOwn<T, std::is_base_of<IUnknown, T>::value>()(rawPtr);
	return MakeShared<T, std::is_base_of<IUnknown, T>::value>()(rawPtr);
}
#endif


#define INHERIT_COM(X) //DECLSPEC_UUID(X)