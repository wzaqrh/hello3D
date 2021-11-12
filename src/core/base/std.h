#pragma once
#include <assert.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <queue>
#include <functional>
#include <type_traits>

#define EIGEN_DONT_ALIGN_STATICALLY
#include <Eigen/Core>
#include <Eigen/LU>
#include <Eigen/SVD>
#include <Eigen/Dense>
#include <Eigen/Geometry>

#define AS_REF(TYPE, V) *(TYPE*)(&V)
#define AS_CONST_REF(TYPE, V) *(const TYPE*)(&V)

typedef Eigen::Transform<float, 3, Eigen::Affine> Transform3fAffine;
typedef Eigen::Transform<float, 3, Eigen::Projective> Transform3Projective;

#include "core/rendersys/d3d9/stddx9.h"
#include "core/rendersys/d3d11/stddx11.h"

#include "wrl/client.h"
using Microsoft::WRL::ComPtr;
#define MakePtr std::make_shared
#define PtrRaw(T) T.get()

typedef struct _XMINT4 {
	int x, y, z, w;
#ifdef __cplusplus
	_XMINT4() {};
	_XMINT4(int _x, int _y, int _z, int _w) : x(_x), y(_y), z(_z), w(_w) {};
#endif // __cplusplus
} XMINT4;

typedef struct _XMINT2 {
	int x, y;
#ifdef __cplusplus
	_XMINT2() {};
	_XMINT2(int _x, int _y) : x(_x), y(_y) {};
#endif // __cplusplus
} XMINT2;

#define USE_ONLY_PNG
#define USE_RENDER_OP
#define D3D11_DEBUG
//#define PRELOAD_SHADER