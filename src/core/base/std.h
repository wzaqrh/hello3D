#pragma once
#include <assimp/cimport.h>
#include <assimp/Importer.hpp>
#include <assimp/ai_assert.h>
#include <assimp/cfileio.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/IOSystem.hpp>
#include <assimp/IOStream.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/DefaultLogger.hpp>

#include <stdio.h>
#include <string>
#include <vector>
#include <assert.h>
#include <type_traits>
#include <map>
#include <memory>
#include <queue>
#include <functional>

#include "stddx9.h"
#include "stddx11.h"
#include "ComBase.h"

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