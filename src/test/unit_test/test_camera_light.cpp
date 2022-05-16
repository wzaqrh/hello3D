#include <catch.hpp>
#include <iostream>
#include <boost/format.hpp>
#include "test/unit_test/unit_test.h"
#include "core/mir.h"
#include "core/scene/camera.h"
#include "core/scene/light.h"

using namespace mir;

TEST_CASE("calculate direct-light's view projection", "DirectLight_CalculateLightingViewProjection")
{
	auto view = mir::math::cam::MakeLookAtLH(Eigen::Vector3f(616, 616, -616), Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(0, 1, 0));
	auto proj =  mir::math::cam::MakeOrthographicOffCenterLH(0, 1024, 0, 768, -1067, 10);

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

#if 0
TEST_CASE("mir_light bug 1201.308", "mir_light_bug1201.308")
{
	auto engine = std::make_shared<Mir>(__LaunchSync__);
	
	auto dir_light = std::make_shared<DirectLight>();
	dir_light->SetDirection(Eigen::Vector3f(0, 0, 1));

	CameraPtr camera = Camera::CreatePerspective(*engine->ResourceMng(), Eigen::Vector2i(1024, 768), Eigen::Vector3f(0,0,-1500), 3000, 30);
	
	{
		Eigen::Matrix4f camera_view = camera->GetView(), camera_proj = camera->GetProjection();
		test::TestViewProjectionWithCases(camera_view, camera_proj);
	}

	{
		Eigen::Matrix4f light_view, light_proj;
		dir_light->CalculateLightingViewProjection(*camera, light_view, light_proj);
		test::TestViewProjectionWithCases(light_view, light_proj);
	}
}

TEST_CASE("mir_camera ProjectPoint", "mir_camera_ProjectPoint")
{
	auto engine = std::make_shared<Mir>(__LaunchSync__);
	auto sceneMng = engine->SceneMng();
	
	sceneMng->RemoveAllLights();
	auto dir_light = sceneMng->AddDirectLight();
	dir_light->SetDirection(Eigen::Vector3f(0, 0, 1));

	sceneMng->RemoveAllCameras();
	auto camera = sceneMng->CreateCameraNode(kCameraOthogonal, Eigen::Vector3f(0, 0, -1500), 3000);
	camera->GetTransform()->SetPosition(Eigen::Vector3f(0, 0, 0));

	auto ndc = camera->ProjectPoint(Eigen::Vector3f(100, 100, 10));
	CHECK(test::IsEqual(ndc, Eigen::Vector3f(-0.8046875, -0.739583313, 0.503331661)));
}

TEST_CASE("mir_camera ProjectPoint 2", "mir_camera_ProjectPoint2")
{
	auto engine = std::make_shared<Mir>(__LaunchSync__);
	auto sceneMng = engine->SceneMng();
	auto rendFac = engine->RenderableFac();
	auto size = engine->ResourceMng()->WinSize() / 2;

	sceneMng->RemoveAllLights();
	auto dir_light = sceneMng->AddDirectLight();
	dir_light->SetDirection(Eigen::Vector3f(0, 0, 1));

	sceneMng->RemoveAllCameras();
	auto camera = sceneMng->CreateCameraNode(kCameraOthogonal, Eigen::Vector3f(0, 0, -1500), 3000); 
	camera->GetTransform()->SetPosition(Eigen::Vector3f(0, 0, 0));

	test::CompareLightCameraByViewProjection(*dir_light, *camera, {});
}
#endif