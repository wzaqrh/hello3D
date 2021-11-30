#include <windows.h>
#include <xnamath.h>
#include <catch.hpp>
#include "core/base/math.h"

#define AS_CONST_REF(TYPE, V) *(const TYPE*)(&V)

static bool IsEqual(float l, float r) {
	return std::abs<float>(l - r) < 0.00001;
}
static bool IsEqual(const Eigen::Matrix4f& m0, const XMMATRIX& m1) {
	float* arr0 = (float*)&m0;
	float* arr1 = (float*)&m1;
	for (size_t i = 0; i < 16; ++i) {
		if (std::abs<float>(arr0[i] - arr1[i]) >= 0.00001)
			return false;
	}
	return true;
}

static_assert(sizeof(Eigen::Matrix4f) == sizeof(XMMATRIX), "");
Eigen::Matrix4f SetMatrixSRT0(Eigen::Vector3f mScale,
	Eigen::Vector3f mPosition,
	Eigen::Vector3f mEuler,
	Eigen::Vector3f mFlip)
{
	Transform3fAffine srt = Transform3fAffine::Identity();
	srt.prescale(mScale);
	if (mFlip.x() != 1 || mFlip.y() != 1 || mFlip.z() != 1)
		srt.prescale(mFlip);
	if (mEuler.x() != 0 || mEuler.x() != 0 || mEuler.z() != 0) {
		auto euler = Eigen::AngleAxisf(mEuler.z(), Eigen::Vector3f::UnitZ())
			* Eigen::AngleAxisf(mEuler.x(), Eigen::Vector3f::UnitX())
			* Eigen::AngleAxisf(mEuler.y(), Eigen::Vector3f::UnitY());
		srt.prerotate(euler);
	}
	if (mPosition.x() != 0 || mPosition.y() != 0 || mPosition.z() != 0)
		srt.pretranslate(mPosition);
	return srt.matrix();
}
XMMATRIX GetMatrixSRT1(Eigen::Vector3f mScale,
	Eigen::Vector3f mPosition,
	Eigen::Vector3f mEuler,
	Eigen::Vector3f mFlip)
{
	XMMATRIX mMatrix = XMMatrixScaling(mScale.x(), mScale.y(), mScale.z());
	if (mFlip.x() != 1 || mFlip.y() != 1 || mFlip.z() != 1)
		mMatrix *= XMMatrixScaling(mFlip.x(), mFlip.y(), mFlip.z());
	if (mEuler.x() != 0 || mEuler.x() != 0 || mEuler.z() != 0)
		mMatrix *= XMMatrixRotationZ(mEuler.z()) 
			* XMMatrixRotationX(mEuler.x()) 
			* XMMatrixRotationY(mEuler.y());
	if (mPosition.x() != 0 || mPosition.y() != 0 || mPosition.z() != 0)
		mMatrix *= XMMatrixTranslation(mPosition.x(), mPosition.y(), mPosition.z());
	return mMatrix;
}
TEST_CASE("eigen is right hand", "[eigen_is_right_hand]") 
{
	Eigen::Vector3f mScale(1,0.07,0.5);
	Eigen::Vector3f mPosition(0,-5,0);
	Eigen::Vector3f mEuler(0,0,0);
	Eigen::Vector3f mFlip(1,-1,1);
	Eigen::Matrix4f m0 = SetMatrixSRT0(mScale, mPosition, mEuler, mFlip);
	XMMATRIX m1 = GetMatrixSRT1(mScale, mPosition, mEuler, mFlip);
	CHECK(IsEqual(m0, m1));

	/*Eigen::Matrix4f p0 = mir::math::MakePerspectiveFovLH(60 / 180.0 * XM_PI, 1280.0f/720, 0.01, 10);
	m0 *= p0;
	XMMATRIX p1 = XMMatrixPerspectiveFovLH(50 / 180.0 * XM_PI, 1280.0f/720, 0.01, 10);
	m1 *= p1;
	CHECK(IsEqual(m0, m1));*/
}

TEST_CASE("eigen matrix translate", "eigen_matrix_translate")
{
	Transform3Projective t0 = Transform3Projective::Identity();
	t0.pretranslate(Eigen::Vector3f(100, 0, 0));

	Eigen::Matrix4f m0 = t0.matrix();
	CHECK(m0(0,3) == 100);

	XMMATRIX m1 = AS_CONST_REF(XMMATRIX, m0);
	CHECK(m1._41 == 100);
}

TEST_CASE("eigen vector length", "eigen_vector_length")
{
	Eigen::Vector3f vec = Eigen::Vector3f(3, 4, 0);
	CHECK(IsEqual(vec.norm(), 5));
}

TEST_CASE("eigen vector block", "eigen_vector_block")
{
	Eigen::Vector4f vec;
	vec.head<3>() = Eigen::Vector3f(0, 0, 0);
}

TEST_CASE("eigen matrix block", "eigen_matrix_block")
{
	Eigen::Matrix4f m0 = Eigen::Matrix4f::Identity();

	{
		m0.topLeftCorner<3, 1>() = Eigen::Vector3f(10, 20, 30);//m[(0,0):(0,3)]
		XMMATRIX m1 = AS_CONST_REF(XMMATRIX, m0);
		CHECK(m1._11 == 10);
		CHECK(m1._12 == 20);
		CHECK(m1._13 == 30);
	}

	{
		m0 = Eigen::Matrix4f::Identity();
		m0.col(0) = Eigen::Vector4f(10, 20, 30, 40);
		XMMATRIX m1 = AS_CONST_REF(XMMATRIX, m0);
		CHECK(m1._11 == 10);
		CHECK(m1._12 == 20);
		CHECK(m1._13 == 30);
		CHECK(m1._14 == 40);
	}

	{
		m0 = Eigen::Matrix4f::Identity();
		m0.block<3, 1>(0, 1) = Eigen::Vector3f(10, 20, 30);//m[(1,0):(1,3)]
		XMMATRIX m1 = AS_CONST_REF(XMMATRIX, m0);
		CHECK(m1._21 == 10);
		CHECK(m1._22 == 20);
		CHECK(m1._23 == 30);
	}
}

TEST_CASE("mir matrix MakeLookAtLH", "mir_matrix_MakeLookAtLH") {
	Eigen::Vector3f eye(0, 0, -10);
	Eigen::Vector3f target(0, 0, 0);
	Eigen::Vector3f up(0, 1, 0);
	Eigen::Matrix4f m0 = mir::math::MakeLookAtLH(eye, target, up);

	XMVECTOR Eye = XMVectorSet(eye.x(), eye.y(), eye.z(), 0.0f);
	XMVECTOR At = XMVectorSet(target.x(), target.y(), target.z(), 0.0f);
	XMVECTOR Up = XMVectorSet(up.x(), up.y(), up.z(), 0.0f);
	XMMATRIX m1 = XMMatrixLookAtLH(Eye, At, Up);

	CHECK(IsEqual(m0, m1));
}

TEST_CASE("mir matrix MakePerspectiveFovLH", "mir_matrix_MakePerspectiveFovLH") {
	float fov = 60 / 180.0 * XM_PI;
	float width = 1280;
	float height = 720;
	float aspect = width / height;
	float zNear = 0.01;
	float zFar = 10;
	Eigen::Matrix4f m0 = mir::math::MakePerspectiveFovLH(fov, aspect, zNear, zFar);
	XMMATRIX m1 = XMMatrixPerspectiveFovLH(fov, aspect, zNear, zFar);
	CHECK(IsEqual(m0, m1));
}

#include <future>
#include <iostream>
TEST_CASE("mir matrix MakeOrthographicOffCenterLH", "mir_matrix_MakeOrthographicOffCenterLH") {
	float width = 1280;
	float height = 720;
	float zNear = 0.01;
	float zFar = 10;
	Eigen::Matrix4f m0 = mir::math::MakeOrthographicOffCenterLH(0, width, 0, height, zNear, zFar);
	XMMATRIX m1 = XMMatrixOrthographicOffCenterLH(0, width, 0, height, zNear, zFar);
	CHECK(IsEqual(m0, m1));
}

TEST_CASE("std_future", "std_future") {
#if 0
	std::future<int> result = std::async([]()->int {
		std::this_thread::sleep_for(std::chrono::seconds(5));
		std::cout << "packaged_task";
		return 7;
});
#else
	std::packaged_task<int()> task([]()->int {
		std::this_thread::sleep_for(std::chrono::seconds(5));
		std::cout << "packaged_task finished";
		return 7;
	});
	std::future<int> result = task.get_future();
#endif
	{
		std::future<int> shared_result = std::move(result);
		std::cout << "continue before packaged_task";
	}
	std::future<int> shared_result = std::move(result);
	CHECK(!result.valid());

	task();
	int a = shared_result.get();
	CHECK(!shared_result.valid());
}