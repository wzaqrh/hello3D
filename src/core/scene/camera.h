#pragma once
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#include "core/predeclare.h"
#include "core/base/base_type.h"
#include "core/base/transform.h"

#if defined TRANSFORM_QUATERNION
#define CAMERA_TRANSFORM
#endif

namespace mir {

enum CameraType {
	kCameraPerspective,
	kCameraOthogonal
};

class MIR_CORE_API Camera : boost::noncopyable
{
public:
	static CameraPtr CreatePerspective(ResourceManager& resMng, const Eigen::Vector3f& eyePos = math::cam::DefEye(), 
		float zfar = 100, float fov = 45.0);
	static CameraPtr CreateOthogonal(ResourceManager& resMng, const Eigen::Vector3f& eyePos = math::cam::DefEye(), 
		float zfar = 100);
	Camera(ResourceManager& resMng);

	void SetLookAt(const Eigen::Vector3f& eye, const Eigen::Vector3f& at, const Eigen::Vector3f& up = math::vec::Up());
	void SetForwardLength(float length);
	void SetZRange(const Eigen::Vector2f& zRange);
	void SetFov(float fov);
	void SetYFlipped(bool flip);

	void SetCameraMask(unsigned mask) { mCameraMask = mask; }
	unsigned GetCameraMask() const { return mCameraMask; }

	void SetDepth(unsigned depth) { mDepth = depth; }
	unsigned GetDepth() const { return mDepth; }

	void SetSkyBox(const SkyBoxPtr& skybox);
	void AddPostProcessEffect(const PostProcessPtr& postEffect);
	IFrameBufferPtr FetchOutput2PostProcess(ResourceFormat format = kFormatR8G8B8A8UNorm);
	IFrameBufferPtr SetOutput(float scale = 1, 
		std::vector<ResourceFormat> formats = { kFormatR8G8B8A8UNorm,kFormatD24UNormS8UInt });
	IFrameBufferPtr SetOutput(IFrameBufferPtr output);
public:
	const TransformPtr& GetTransform() const;
	const SkyBoxPtr& GetSkyBox() const { return mSkyBox; }
	const std::vector<PostProcessPtr>& GetPostProcessEffects() const { return mPostProcessEffects; }
	const IFrameBufferPtr& GetPostProcessInput() const { return mPostProcessInput; }
	const IFrameBufferPtr& GetOutput() const { return mOutput; }
	
	const Eigen::Matrix4f& GetView() const;
	const Eigen::Matrix4f& GetProjection() const;
	
	CameraType GetType() const { return mType; }
	const Eigen::Vector2i& GetWinSize() const { return mScreenSize; }

	Eigen::Vector3f GetEye() const;
	Eigen::Vector3f GetLookAt() const;
	Eigen::Vector3f GetForward() const;
	float GetForwardLength() const;

	Eigen::Vector3f ProjectPoint(const Eigen::Vector3f& worldpos) const;//world -> ndc
	Eigen::Vector4f ProjectPoint(const Eigen::Vector4f& worldpos) const;
private:
	const Eigen::Vector2i& GetSize() const { return mSize; }
	void InitAsPerspective(const Eigen::Vector2i& screensize, const Eigen::Vector3f& eyePos, float zfar, float fov);
	void InitAsOthogonal(const Eigen::Vector2i& screensize, const Eigen::Vector3f& eyePos, float zfar);
	void RecalculateProjection() const;
	void RecalculateView() const;
private:
	ResourceManager& mResourceMng;
	TransformPtr mTransform;
	SkyBoxPtr mSkyBox;
	std::vector<PostProcessPtr> mPostProcessEffects;
	IFrameBufferPtr mPostProcessInput, mOutput;
	
	unsigned mCameraMask = -1;
	unsigned mDepth = 0;
private:
	CameraType mType;
	Eigen::Vector2i mScreenSize, mSize;
#if defined CAMERA_TRANSFORM
	Eigen::Vector3f mUpVector;
	float mForwardLength;
#else
	Eigen::Vector3f mEyePos, mForwardVector, mUpVector;
#endif
	Eigen::Vector2f mZRange;
	float mFov;
	bool mFlipY;

	mutable Eigen::Matrix4f mView, mProjection, mWorldView;
	mutable bool mViewDirty, mProjectionDirty;
};

}