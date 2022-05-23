#pragma once

namespace mir {

enum CubeFace 
{
	kCubeFacePosX,
	kCubeFaceNegX,
	kCubeFacePosY,
	kCubeFaceNegY,
	kCubeFacePosZ,
	kCubeFaceNegZ,
	kCubeFaceCount
};

enum CubeCorner 
{
	kCubeConerFrontLeftBottom = 0,
	kCubeConerFrontLeftTop = 1,
	kCubeConerFrontRightTop = 2,
	kCubeConerFrontRightBottom = 3,
	kCubeConerFrontCount,
	kCubeConerBackLeftBottom = 4,
	kCubeConerBackLeftTop = 5,
	kCubeConerBackRightTop = 6,
	kCubeConerBackRightBottom = 7,
	kCubeConerCount,
};

enum CubeConerMask 
{
	kCubeConerBottom = 0U,
	kCubeConerLeft = 0U,
	kCubeConerFront = 0U,
	kCubeConerTop = 1 << 0,
	kCubeConerRight = 1 << 1,
	kCubeConerBack = 1 << 2
};
#define IS_CORNER_TOP(V)   ((V + 1) & (kCubeConerTop << 1))
#define IS_CORNER_RIGHT(V) (V & kCubeConerRight)
#define IS_CORNER_BACK(V)  (V & kCubeConerBack)
#define MAKE_CORNER(MASK_BACK, MASK_RIGHT, MASK_TOP) (MASK_BACK | MASK_RIGHT | ((MASK_RIGHT >> 1) ^ MASK_TOP))
static_assert(MAKE_CORNER(kCubeConerFront, kCubeConerLeft, kCubeConerTop) == kCubeConerFrontLeftTop, "");
static_assert(MAKE_CORNER(kCubeConerFront, kCubeConerRight, kCubeConerTop) == kCubeConerFrontRightTop, "");

}