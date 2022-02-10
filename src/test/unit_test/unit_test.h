#pragma once
#include "core/mir_config.h"
#include "core/base/math.h"
#include "core/predeclare.h"
#include "core/mir_export.h"

//using namespace mir;
//using namespace mir::scene;
//using namespace mir::rend;

namespace mir {
namespace test {

MIR_CORE_API bool IsEqual(float l, float r);
MIR_CORE_API bool IsEqual(const Eigen::Vector2f& l, const Eigen::Vector2f& r);
MIR_CORE_API bool IsEqual(const Eigen::Vector3f& l, const Eigen::Vector3f& r);
MIR_CORE_API bool IsEqual(const Eigen::Vector4f& l, const Eigen::Vector4f& r);

MIR_CORE_API bool CheckInNDC(const Eigen::Vector4f& p);

MIR_CORE_API void TestViewProjectionWithCases(const Eigen::Matrix4f& view, const Eigen::Matrix4f& proj);
MIR_CORE_API void CompareLightCameraByViewProjection(const scene::Light& dir_light, const scene::Camera& camera, Eigen::Vector2i size, std::vector<Eigen::Vector4f> positions);

}
}

#if defined MIR_TEST
#define MIR_TEST_CASE(STATEMENT) do { using namespace mir::test; STATEMENT; } while(0)
#else
#define MIR_TEST_CASE(STATEMENT)
#endif