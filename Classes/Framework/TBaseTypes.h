#pragma once
#include "std.h"
#include "TPredefine.h"

template<class T>
T clamp(T minVal, T maxVal, T v) {
	return min(max(v, minVal),maxVal);
}

struct TINT4 {
	int x, y, z, w;
};

struct TData {
	void* data;
	unsigned int dataSize;
	TData(void* __data, unsigned int __dataSize);
};
template <class T>
inline TData make_data(const T& v) {
	return TData((void*)&v, sizeof(v));
}

__declspec(align(16)) struct TCameraBase {
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
public:
	void* operator new(size_t i){
		return _mm_malloc(i,16);
	}
	void operator delete(void* p) {
		_mm_free(p);
	}
	TCameraBase();
public:
	XMFLOAT3 CalNDC(XMFLOAT3 pos);
	XMFLOAT4 CalNDC(XMFLOAT4 pos);
};

struct TCamera : public TCameraBase {
public:
	bool mIsPespective = true;
	float mEyeDistance;
	XMFLOAT3 mEye, mAt, mUp;
	int mWidth, mHeight;
	double mFOV, mFar;
public:
	void* operator new(size_t i){
		return _mm_malloc(i,16);
	}
	void operator delete(void* p) {
		_mm_free(p);
	}
	TCamera() {}
	TCamera(const TCamera& other);
	void SetLookAt(XMFLOAT3 eye, XMFLOAT3 at);
	void SetPerspectiveProj(int width, int height, double fov, double far1);
	void SetOthogonalProj(int width, int height, double far1);
public:
	static std::shared_ptr<TCamera> CreatePerspective(int width, int height, double fov = 45.0, int eyeDistance = 10, double far1 = 100);
	static std::shared_ptr<TCamera> CreateOthogonal(int width, int height, double far1 = 100);
};
typedef std::shared_ptr<TCamera> TCameraPtr;

enum enLightType {
	E_LIGHT_DIRECT,
	E_LIGHT_POINT,
	E_LIGHT_SPOT
};

class TDirectLight {
public:
	XMFLOAT4 LightPos;//world space
	XMFLOAT4 DiffuseColor;
	XMFLOAT4 SpecularColorPower;
public:
	TDirectLight();
	void SetDirection(float x, float y, float z);
	void SetDiffuseColor(float r, float g, float b, float a);
	void SetSpecularColor(float r, float g, float b, float a);
	void SetSpecularPower(float power);
public:
	TCameraBase GetLightCamera(TCamera& otherCam);
	static TConstBufferDecl& GetDesc();
};
typedef std::shared_ptr<TDirectLight> TDirectLightPtr;

class TPointLight : public TDirectLight {
public:
	XMFLOAT4 Attenuation;
public:
	TPointLight();
	void SetPosition(float x, float y, float z);
	void SetAttenuation(float a, float b, float c);
public:
	static TConstBufferDecl& GetDesc();
};
typedef std::shared_ptr<TPointLight> TPointLightPtr;

class TSpotLight : public TPointLight {
public:
	XMFLOAT4 DirectionCutOff;
public:
	TSpotLight();
	void SetDirection(float x, float y, float z);
	void SetCutOff(float cutoff);
	void SetAngle(float radian);
public:
	static TConstBufferDecl& GetDesc();
};
typedef std::shared_ptr<TSpotLight> TSpotLightPtr;

struct TBlendFunc {
	D3D11_BLEND src, dst;
	TBlendFunc() :src(D3D11_BLEND_ONE), dst(D3D11_BLEND_INV_SRC_ALPHA) {};
	TBlendFunc(D3D11_BLEND __src, D3D11_BLEND __dst);
};

struct TDepthState {
	BOOL depthEnable;
	D3D11_COMPARISON_FUNC depthFunc;
	D3D11_DEPTH_WRITE_MASK depthWriteMask;
	TDepthState() :depthEnable(false), depthFunc(D3D11_COMPARISON_LESS), depthWriteMask(D3D11_DEPTH_WRITE_MASK_ALL) {}
	TDepthState(bool __depthEnable, D3D11_COMPARISON_FUNC __depthFunc = D3D11_COMPARISON_LESS, D3D11_DEPTH_WRITE_MASK __depthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL);
};

enum EConstBufferElementType {
	E_CONSTBUF_ELEM_BOOL,
	E_CONSTBUF_ELEM_INT,
	E_CONSTBUF_ELEM_FLOAT,
	E_CONSTBUF_ELEM_FLOAT4,
	E_CONSTBUF_ELEM_MATRIX,
	E_CONSTBUF_ELEM_STRUCT,
	E_CONSTBUF_ELEM_MAX
};
template<class T> struct CBETypeTrait {};
#define DECLRAE_CBET(E,CLS) template<> struct CBETypeTrait<CLS> { enum {value = E }; };
DECLRAE_CBET(E_CONSTBUF_ELEM_INT, unsigned int);
DECLRAE_CBET(E_CONSTBUF_ELEM_FLOAT, float);
DECLRAE_CBET(E_CONSTBUF_ELEM_FLOAT4, XMFLOAT4);
DECLRAE_CBET(E_CONSTBUF_ELEM_MATRIX, XMMATRIX);
DECLRAE_CBET(E_CONSTBUF_ELEM_STRUCT, TDirectLight);
DECLRAE_CBET(E_CONSTBUF_ELEM_STRUCT, TPointLight);
DECLRAE_CBET(E_CONSTBUF_ELEM_STRUCT, TSpotLight);
template<class T> inline EConstBufferElementType GetCBEType(const T& Value) { return static_cast<EConstBufferElementType>(CBETypeTrait<T>::value); }
template<class T> inline TConstBufferDecl& GetDecl(const T& Value) { return T::GetDesc(); }

struct TConstBufferDeclElement {
	size_t offset;
	size_t size;
	size_t count;
	std::string name;
	EConstBufferElementType type;
public:
	TConstBufferDeclElement(const char* __name, EConstBufferElementType __type, size_t __size, size_t __count = 0, size_t __offset = 0);
};
#define CBELEMNT(CLS) TConstBufferDeclElement(#CLS, GetCBEType(cb.CLS), sizeof(cb.CLS))
#define BUILD_ADD(CLS) builder.Add(CBELEMNT(CLS));

#define BUILD_ADDSUB(CLS) builder.Add(CBELEMNT(CLS), GetDecl(cb.CLS));

#define CBELEMNTS(CLS, N) TConstBufferDeclElement(#CLS, GetCBEType(cb.CLS[0]), sizeof(cb.CLS), ARRAYSIZE(cb.CLS))
#define BUILD_ADDS(CLS) builder.Add(CBELEMNTS(CLS));

struct TConstBufferDecl {
	std::vector<TConstBufferDeclElement> elements;
	std::map<std::string, TConstBufferDecl> subDecls;
	size_t bufferSize = 0;
public:
	TConstBufferDeclElement& Add(const TConstBufferDeclElement& elem);
	TConstBufferDeclElement& Add(const TConstBufferDeclElement& elem, const TConstBufferDecl& subDecl);
	TConstBufferDeclElement& Last();
};
typedef std::shared_ptr<TConstBufferDecl> TConstBufferDeclPtr;

#include "TPredefine.h"
//#include "TInterfaceType.h"
//#include "TMaterial.h"