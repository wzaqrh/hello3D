#include <catch.hpp>
#include "test/unit_test/unit_test.h"
#include "core/mir.h"
#include "core/scene/camera.h"
#include "core/scene/light.h"

namespace mir {
namespace test {

bool IsEqual(float l, float r) {
	return std::abs<float>(l - r) < 0.00001;
}
bool IsEqual(Eigen::Vector3f l, Eigen::Vector3f r) {
	return IsEqual(l.x(), r.x())
		&& IsEqual(l.y(), r.y())
		&& IsEqual(l.z(), r.z());
}
bool IsEqual(Eigen::Vector4f l, Eigen::Vector4f r) {
	return IsEqual(l.x(), r.x())
		&& IsEqual(l.y(), r.y())
		&& IsEqual(l.z(), r.z())
		&& IsEqual(l.w(), r.w());
}

bool CheckInNDC(Eigen::Vector4f p)
{
	p.head<3>() /= p.w();
	p.w() = 1;
	if (p.x() < -1 || p.x() > 1) return false;
	if (p.y() < -1 || p.y() > 1) return false;
	if (p.z() < 0 || p.z() > 1) return false;
	return true;
}
void TestViewProjectionWithCases(Eigen::Matrix4f view, Eigen::Matrix4f proj)
{
	//view = math::MakeLookAtLH(Eigen::Vector3f(0.57, 0.57, -0.57) * 1024, Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(0, 1, 0));
	//proj = math::MakeOrthographicOffCenterLH(0, 1024, 0, 768, -10, 1024);

	Eigen::Vector4f posArr[] = {
		//Eigen::Vector4f(0, 0,  0, 1),
		//Eigen::Vector4f(5, 5, -5, 1),
		Eigen::Vector4f(10, 10, 0, 1),
		Eigen::Vector4f(10, 10, -150, 1),
		//Eigen::Vector4f(10, 0, -1, 1)
	};
	for (size_t i = 0; i < sizeof(posArr) / sizeof(posArr[0]); ++i) {
		auto pos = posArr[i];
		Eigen::Vector4f perspective = Transform3Projective(proj * view) * pos; perspective /= perspective.w();
		Eigen::Vector4f world = Transform3Projective(view) * pos; world /= world.w();
		//Eigen::Vector4f perspective2 = Transform3Projective(proj) * pos; perspective2 /= perspective2.w();
		//BOOST_ASSERT(checkInNDC(perspective));
		//BOOST_ASSERT(checkInNDC(perspective2));
		pos = posArr[i];
	}
}
void CompareLightCameraByViewProjection(const ILight& dir_light, const Camera& camera, std::vector<Eigen::Vector4f> positions)
{
	static std::vector<Eigen::Vector4f> local_positions = {
		Eigen::Vector4f(100, 0, -100, 1.0),
		Eigen::Vector4f(-1024/2, 0, 200, 1.0),
		Eigen::Vector4f(1024/2, 0, 200, 1.0),
	};
	positions.insert(positions.end(), local_positions.begin(), local_positions.end());

	Eigen::Vector4f light_ndc, camera_ndc, camera_ndc1;
	for (size_t i = 0; i < positions.size(); ++i) {
		auto cube_position = positions[i];
		Eigen::Matrix4f light_view, light_proj;
		dir_light.CalculateLightingViewProjection(camera, light_view, light_proj);
		light_ndc = Transform3Projective(light_view) * cube_position;
		light_ndc = Transform3Projective(light_proj) * cube_position;
		light_ndc = Transform3Projective(light_proj * light_view) * cube_position;

		Eigen::Matrix4f camera_view = camera.GetView(), camera_proj = camera.GetProjection();
		camera_ndc = Transform3Projective(camera_proj * camera_view) * cube_position;
		camera_ndc.head<3>() /= camera_ndc.w();
		camera_ndc.w() = 1;

		camera_ndc1 = camera.ProjectPoint(cube_position);
		BOOST_ASSERT(IsEqual(camera_ndc, camera_ndc1));
	}
}

}
}