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
#include <map>
#include <memory>
#include <queue>
#include <functional>

#include "stddx9.h"
#include "stddx11.h"

typedef struct _XMINT4 {
	int x, y, z, w;
} XMINT4;