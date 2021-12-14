#include <catch.hpp>
#include "test/unit_test/unit_test.h"
#include "core/mir.h"
#include "core/scene/camera.h"
#include "core/scene/light.h"

namespace mir {
namespace test {

#define EPS 0.0001
bool IsEqual(float l, float r) {
	return std::abs<float>(l - r) < EPS;
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
	p /= p.w();
	BOOST_ASSERT(p.x() >= -1-EPS && p.x() <= 1+EPS);
	BOOST_ASSERT(p.y() >= -1-EPS && p.y() <= 1+EPS);
	BOOST_ASSERT(p.z() >= 0-EPS && p.z() <= 1+EPS);
	if (p.x() < -1-EPS || p.x() > 1+EPS) return false;
	if (p.y() < -1-EPS || p.y() > 1+EPS) return false;
	if (p.z() < 0-EPS || p.z() > 1+EPS) return false;
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
void CompareLightCameraByViewProjection(const ILight& dir_light, const Camera& camera, Eigen::Vector2i size, std::vector<Eigen::Vector4f> positions)
{
	constexpr bool castShadow = true;
//#define ONLY_CHECK_PROJ
	static std::vector<Eigen::Vector4f> local_positions = {
		Eigen::Vector4f(0, 0, 0, 1),
	};
	positions.insert(positions.end(), local_positions.begin(), local_positions.end());

	Eigen::Vector4f light_ndc, camera_ndc, camera_ndc1;
	for (size_t i = 0; i < positions.size(); ++i) {
		auto cube_position = positions[i];
		Eigen::Matrix4f light_view, light_proj;
		dir_light.CalculateLightingViewProjection(camera, size, castShadow, light_view, light_proj);
		light_ndc = Transform3Projective(light_view) * cube_position;
		light_ndc = Transform3Projective(light_proj) * cube_position;
	#if !defined ONLY_CHECK_PROJ
		light_ndc = Transform3Projective(light_proj * light_view) * cube_position;
	#endif
		if (!castShadow) {
			light_ndc /= light_ndc.w();
			light_ndc.head<2>() = (light_ndc.head<2>() - Eigen::Vector2f(0.5, 0.5)) * 2;
			light_ndc.y() = -light_ndc.y();
		}

		Eigen::Matrix4f camera_view = camera.GetView(), camera_proj = camera.GetProjection();
		camera_ndc = Transform3Projective(camera_proj) * cube_position;
	#if !defined ONLY_CHECK_PROJ
		camera_ndc = Transform3Projective(camera_proj * camera_view) * cube_position;
	#endif
		camera_ndc /= camera_ndc.w();

		camera_ndc1 = camera.ProjectPoint(cube_position);

		if (i < positions.size() - local_positions.size()) {
			BOOST_ASSERT(IsEqual(camera_ndc.x(), light_ndc.x()));
			BOOST_ASSERT(IsEqual(camera_ndc.y(), light_ndc.y()));
		}
	#if !defined ONLY_CHECK_PROJ
		if (i < positions.size() - local_positions.size()) {
			BOOST_ASSERT(CheckInNDC(light_ndc));
			BOOST_ASSERT(CheckInNDC(camera_ndc));
			BOOST_ASSERT(CheckInNDC(camera_ndc1));
		}
		BOOST_ASSERT(IsEqual(camera_ndc, camera_ndc1));
	#endif
	}
}

}
}