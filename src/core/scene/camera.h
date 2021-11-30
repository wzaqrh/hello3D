#pragma once
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#include "core/predeclare.h"
#include "core/rendersys/base_type.h"
#include "core/base/transform.h"

namespace mir {

enum CameraType {
	kCameraPerspective,
	kCameraOthogonal
};

class MIR_CORE_API Camera : boost::noncopyable
{
public:
	static CameraPtr CreatePerspective(ResourceManager& resMng, const Eigen::Vector2i& size, 
		Eigen::Vector3f eyePos = Eigen::Vector3f(0,0,-10), double far1 = 100, double fov = 45.0);
	static CameraPtr CreateOthogonal(ResourceManager& resMng, const Eigen::Vector2i& size, 
		Eigen::Vector3f eyePos = Eigen::Vector3f(0,0,-10), double far1 = 100);
	Camera(ResourceManager& resMng);

	void SetCameraMask(unsigned mask) { mCameraMask = mask; }
	unsigned GetCameraMask() const { return mCameraMask; }

	void SetDepth(unsigned depth) { mDepth = depth; }
	unsigned GetDepth() const { return mDepth; }

	void SetLookAt(const Eigen::Vector3f& eye, const Eigen::Vector3f& at);
	void SetFlipY(bool flip);

	void SetSkyBox(const SkyBoxPtr& skybox);
	void AddPostProcessEffect(const PostProcessPtr& postEffect);
	IFrameBufferPtr FetchPostProcessInput(ResourceFormat format = kFormatR8G8B8A8UNorm);
	IFrameBufferPtr FetchOutput(ResourceFormat format = kFormatR8G8B8A8UNorm);
private:
	void SetPerspectiveProj(const Eigen::Vector2i& size, double fov, double zFar);
	void SetOthogonalProj(const Eigen::Vector2i& size, double zFar);
public:
	const TransformPtr& GetTransform() const;
	const SkyBoxPtr& GetSkyBox() const { return mSkyBox; }
	const std::vector<PostProcessPtr>& GetPostProcessEffects() const { return mPostProcessEffects; }
	const IFrameBufferPtr& GetPostProcessInput() const { return mPostProcessInput; }
	const IFrameBufferPtr& GetOutput() const { return mOutput; }
	
	const Eigen::Matrix4f& GetView() const;
	const Eigen::Matrix4f& GetProjection() const  { return mProjection; }
	
	CameraType GetType() const { return mType; }
	const Eigen::Vector2i& GetSize() const { return mSize; }
	const Eigen::Vector3f& GetLookAtPos() const { return mLookAtPos; }
	const Eigen::Vector3f& GetEyePos() const { return mEyePos; }
	float GetZFar() const { return mZFar; }

	Eigen::Vector3f ProjectPoint(const Eigen::Vector3f& worldpos) const;//world -> ndc
	Eigen::Vector4f ProjectPoint(const Eigen::Vector4f& worldpos) const;
private:
	ResourceManager& mResourceMng;

	SkyBoxPtr mSkyBox;
	std::vector<PostProcessPtr> mPostProcessEffects;

	TransformPtr mTransform;
	mutable bool mTransformDirty;

	bool mFlipY;
	mutable Eigen::Matrix4f mView, mProjection, mWorldView;

	unsigned mCameraMask = -1;
	unsigned mDepth = 0;
private:
	IFrameBufferPtr mPostProcessInput, mOutput;

	Eigen::Vector2i mSize;
	Eigen::Vector3f mEyePos, mLookAtPos, mUpVector;

	CameraType mType;
	double mFov, mZFar;
};

}