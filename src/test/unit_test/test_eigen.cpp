#include <windows.h>
#include <xnamath.h>
#define EIGEN_DONT_ALIGN_STATICALLY
#include <Eigen/Core>
#include <Eigen/LU>
#include <Eigen/SVD>
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include "catch.hpp"


typedef Eigen::Transform<float, 3, Eigen::Affine> Transform3fAffine;
typedef Eigen::Transform<float, 3, Eigen::Projective> Transform3Projective;
#define AS_REF(TYPE, V) *(TYPE*)(&V)
#define AS_CONST_REF(TYPE, V) *(const TYPE*)(&V)

static bool CHECK_EQUAL(const Eigen::Matrix4f& m0, const XMMATRIX& m1) {
	float* arr0 = (float*)&m0;
	float* arr1 = (float*)&m1;
	for (size_t i = 0; i < 16; ++i) {
		if (std::abs<float>(arr0[i] - arr1[1]) >= 0.000001)
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
	CHECK_EQUAL(m0, m1);
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

namespace mir {
Eigen::Matrix4f MakeLookAt(const Eigen::Vector3f& eye,
	const Eigen::Vector3f& target,
	const Eigen::Vector3f& up);
}

TEST_CASE("mir camera functions", "mir_camera_functions") {
	Eigen::Vector3f eye(0, 0, -10);
	Eigen::Vector3f target(0, 0, 0);
	Eigen::Vector3f up(0, 1, 0);
	Eigen::Matrix4f m0 = mir::MakeLookAt(eye, target, up);

	XMVECTOR Eye = XMVectorSet(eye.x(), eye.y(), eye.z(), 0.0f);
	XMVECTOR At = XMVectorSet(target.x(), target.y(), target.z(), 0.0f);
	XMVECTOR Up = XMVectorSet(up.x(), up.y(), up.z(), 0.0f);
	XMMATRIX m1 = XMMatrixLookAtLH(Eye, At, Up);

	CHECK_EQUAL(m0, m1);
}