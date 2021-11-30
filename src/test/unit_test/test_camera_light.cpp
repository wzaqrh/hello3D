#include <catch.hpp>
#include <iostream>
#include <boost/format.hpp>
#include "core/mir.h"
#include "core/base/math.h"
#include "core/scene/camera.h"
#include "core/scene/light.h"

using namespace mir;

TEST_CASE("calculate direct-light's view projection", "DirectLight_CalculateLightingViewProjection")
{
	auto view = mir::math::MakeLookAtLH(Eigen::Vector3f(616, 616, -616), Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(0, 1, 0));
	auto proj =  mir::math::MakeOrthographicOffCenterLH(0, 1024, 0, 768, -1067, 10);

	Eigen::Vector4f posArr[] = {
		Eigen::Vector4f(5, 5, -5, 1),
		Eigen::Vector4f(10, 0, -199, 1),
		Eigen::Vector4f(10, 0, -200, 1),
		Eigen::Vector4f(10, 0, -1, 1)
	};
	for (size_t i = 0; i < sizeof(posArr) / sizeof(posArr[0]); ++i) {
		auto pos = posArr[i];
		Eigen::Vector4f perspective = Transform3Projective(view * proj) * pos;
		Eigen::Vector4f world = Transform3Projective(view) * pos;
		Eigen::Vector4f perspective2 = Transform3Projective(proj) * pos;
		boost::format fmt("light(%1%): pos(%2%) perspective(%3%) world(%4%) perspective2(%5%)");
		fmt % i %pos %perspective %world %perspective2;
		std::cout << fmt.str() << std::endl;
	}
}

bool checkInNDC(Eigen::Vector4f p) {
	if (p.x() < -1 || p.x() > 1) return false;
	if (p.y() < -1 || p.y() > 1) return false;
	if (p.z() < 0 || p.z() > 1) return false;
	return true;
}
void testViewProjection(Eigen::Matrix4f view, Eigen::Matrix4f proj)
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
TEST_CASE("mir_light bug 1201.308", "mir_light_bug1201.308")
{
	auto engine = std::make_shared<Mir>(__LaunchSync__);
	
	auto dir_light = std::make_shared<DirectLight>();
	dir_light->SetDirection(Eigen::Vector3f(0, 0, 1));

	CameraPtr camera = Camera::CreatePerspective(*engine->ResourceMng(), Eigen::Vector2i(1024, 768), Eigen::Vector3f(0,0,-1500), 3000, 30);
	
	{
		Eigen::Matrix4f camera_view = camera->GetView(), camera_proj = camera->GetProjection();
		testViewProjection(camera_view, camera_proj);
	}

	{
		Eigen::Matrix4f light_view, light_proj;
		dir_light->CalculateLightingViewProjection(*camera, light_view, light_proj);
		testViewProjection(light_view, light_proj);
	}
}