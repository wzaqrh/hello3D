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

typedef Eigen::Transform<float, 3, Eigen::Affine> Transform3fAffine;
typedef Eigen::Transform<float, 3, Eigen::Projective> Transform3Projective;

//#include "core/rendersys/d3d9/stddx9.h"
//#include "core/rendersys/d3d11/stddx11.h"

#define MakePtr std::make_shared
#define PtrRaw(T) T.get()

#define USE_ONLY_PNG
#define USE_RENDER_OP
#define D3D11_DEBUG
//#define PRELOAD_SHADER

#define interface struct