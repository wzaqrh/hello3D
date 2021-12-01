#pragma once
#include "core/base/math.h"
#include "core/predeclare.h"
#include "core/mir_export.h"

namespace mir {
namespace test {

MIR_CORE_API bool IsEqual(float l, float r);
MIR_CORE_API bool IsEqual(Eigen::Vector3f l, Eigen::Vector3f r);
MIR_CORE_API bool IsEqual(Eigen::Vector4f l, Eigen::Vector4f r);

MIR_CORE_API bool CheckInNDC(Eigen::Vector4f p);

MIR_CORE_API void TestViewProjectionWithCases(Eigen::Matrix4f view, Eigen::Matrix4f proj);
MIR_CORE_API void CompareLightCameraByViewProjection(const ILight& dir_light, const Camera& camera, std::vector<Eigen::Vector4f> positions);

}
}
