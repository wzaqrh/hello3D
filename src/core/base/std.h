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

#define MakePtr std::make_shared
#define PtrRaw(T) T.get()

#define interface struct

#define DECLARE_STATIC_CREATE_CONSTRUCTOR(CLASS) \
	template <typename... T> static std::shared_ptr<CLASS> \
	Create(T &&...args) { return std::shared_ptr<CLASS>(new CLASS(std::forward<T>(args)...)); }