#include <windows.h>
#include <xnamath.h>
#define EIGEN_DONT_ALIGN_STATICALLY
#include <Eigen/Core>
#include <Eigen/LU>
#include <Eigen/SVD>
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include "catch.hpp"

static_assert(sizeof(Eigen::Matrix4f) == sizeof(XMMATRIX), "");

#define CHECK_EQUAL(L, R) CHECK(sizeof(L) == sizeof(R)); CHECK(memcmp(&L, &R, sizeof(L)) == 0);

Eigen::Matrix4f SetMatrixSRT0(Eigen::Vector3f mScale,
	Eigen::Vector3f mPosition,
	Eigen::Vector3f mEuler,
	Eigen::Vector3f mFlip)
{
	typedef Eigen::Transform<float, 3, Eigen::Affine> Transform3fAffine;
	Transform3fAffine srt = Transform3fAffine::Identity();
	srt.scale(mScale);
	if (mFlip.x() != 1 || mFlip.y() != 1 || mFlip.z() != 1)
		srt.scale(mFlip);
	if (mEuler.x() != 0 || mEuler.x() != 0 || mEuler.z() != 0) {
		auto euler = Eigen::AngleAxisf(mEuler.z(), Eigen::Vector3f::UnitZ())
			* Eigen::AngleAxisf(mEuler.x(), Eigen::Vector3f::UnitX())
			* Eigen::AngleAxisf(mEuler.y(), Eigen::Vector3f::UnitY());
		srt.rotate(euler);
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
	CHECK_EQUAL(SetMatrixSRT0(mScale, mPosition, mEuler, mFlip), 
			    GetMatrixSRT1(mScale, mPosition, mEuler, mFlip));
}