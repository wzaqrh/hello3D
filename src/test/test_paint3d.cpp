#include "test/test_case.h"
#include "test/app.h"
#include "core/renderable/sprite.h"
#include "core/renderable/paint3d.h"

using namespace mir;
using namespace mir::rend;

class TestPaint3D : public App
{
protected:
	CoTask<bool> OnInitScene() override;
	void OnInitCamera() override {}
};

CoTask<bool> TestPaint3D::OnInitScene()
{
	CameraPtr camera = mScneMng->CreateCameraNode(kCameraPerspective);

	rend::Paint3DPtr paint = mScneMng->AddRendAsNode(CoAwait mRendFac->CreatePaint3DT());
	paint->SetColor(0xFFD3D3D3);//gray
	paint->DrawLine(Eigen::Vector3f(-10, -10, 0), Eigen::Vector3f(10, 10, 5));

	//五角星
	paint->SetColor(0xFFFFFF00);//yellow
	Eigen::Vector3f pts[6] = {
		Eigen::Vector3f(-2, -2, 0),
		Eigen::Vector3f(0, 2, 0),
		Eigen::Vector3f(2, -2, 0),
		Eigen::Vector3f(-2, 0, 0),
		Eigen::Vector3f(2, 0, 0),
		Eigen::Vector3f(-2, -2, 0),
	};
	paint->DrawPolygon(pts, 6);

	//正方形
	paint->SetColor(0xFFA020F0);//purple
	paint->DrawRectEdge(Eigen::Vector3f(3,-4,0), Eigen::Vector3f(3,4,4), Eigen::Vector3f(1,0,0));

	//AABB
	paint->SetColor(0xFF0000FF);//blue
	Eigen::AlignedBox3f bounds;
	bounds.extend(Eigen::Vector3f(-4,-4,0));
	bounds.extend(Eigen::Vector3f(4,4,4));
	paint->DrawAABBEdge(bounds);

	//cube
	paint->SetColor(0xFFFF0000);//red
	paint->DrawCube(Eigen::Vector3f(0,0,0), Eigen::Vector3f(3,4,-2), Eigen::Vector3f(-1,0,0));

	CoReturn true;
}

auto reg = AppRegister<TestPaint3D>("test_paint3d");