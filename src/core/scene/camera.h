#pragma once
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#include "core/predeclare.h"
#include "core/base/base_type.h"
#include "core/base/transform.h"
#include "core/base/declare_macros.h"

namespace mir {

enum RenderingPath {
	kRenderPathForward,
	kRenderPathDeffered
};

enum CameraType {
	kCameraPerspective,
	kCameraOthogonal
};

class MIR_CORE_API Camera : boost::noncopyable
{
public:
	TemplateArgs static CameraPtr CreatePerspective(ResourceManager& resMng, T &&...args) {
		CameraPtr camera = std::make_shared<Camera>(resMng);
		auto size = resMng.WinSize();
		camera->InitAsPerspective(1.0f*size.x()/size.y(), std::forward<T>(args)...);
		return camera;
	}
	TemplateArgs static CameraPtr CreateOthogonal(ResourceManager& resMng, T &&...args) {
		CameraPtr camera = std::make_shared<Camera>(resMng);
		auto size = resMng.WinSize();
		camera->InitAsOthogonal(1.0f*size.x()/size.y(), std::forward<T>(args)...);
		return camera;
	}
	Camera(ResourceManager& resMng);

	void SetLookAt(const Eigen::Vector3f& eye, const Eigen::Vector3f& at, const Eigen::Vector3f& up = math::vec::Up());
	void SetForwardLength(float length);
	void SetClippingPlane(const Eigen::Vector2f& zRange);
	void SetAspect(float aspect);
	void SetOrthographicSize(float size);
	void SetFov(float fov);
	void SetYFlipped(bool flip);

	void SetCameraMask(unsigned mask) { mCameraMask = mask; }
	unsigned GetCameraMask() const { return mCameraMask; }

	void SetDepth(unsigned depth) { mDepth = depth; }
	unsigned GetDepth() const { return mDepth; }

	void SetRenderingPath(RenderingPath path) { mRenderPath = path; }
	RenderingPath GetRenderingPath() const { return mRenderPath; }

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
	const IFrameBufferPtr& GetOutput2PostProcess() const { return mPostProcessInput; }
	const IFrameBufferPtr& GetOutput() const { return mOutput; }
	
	const Eigen::Matrix4f& GetView() const;
	const Eigen::Matrix4f& GetProjection() const;
	
	CameraType GetType() const { return mType; }
	Eigen::Vector3f GetEye() const;
	Eigen::Vector3f GetLookAt() const;
	Eigen::Vector3f GetForward() const;
	float GetForwardLength() const;

	Eigen::Vector2f GetOthoWinSize() const;
	Eigen::Vector3f ProjectPoint(const Eigen::Vector3f& worldpos) const;//world -> ndc
	Eigen::Vector4f ProjectPoint(const Eigen::Vector4f& worldpos) const;
private:
	void InitAsPerspective(float aspect = 1.0,
		const Eigen::Vector3f& eyePos = math::cam::DefEye(), 
		const Eigen::Vector3f& length_forward = math::vec::Forward(), 
		const Eigen::Vector2f& clipPlane = math::cam::DefClippingPlane(),
		float fov = math::cam::DefFov(),
		unsigned cameraMask = -1);
	void InitAsOthogonal(float aspect = 1.0,
		const Eigen::Vector3f& eyePos = math::cam::DefEye(), 
		const Eigen::Vector3f& length_forward = math::vec::Forward(), 
		const Eigen::Vector2f& clipPlane = math::cam::DefClippingPlane(),
		float othoSize = math::cam::DefOthoSize(),
		unsigned cameraMask = -1);
	void RecalculateProjection() const;
	void RecalculateView() const;
private:
	ResourceManager& mResourceMng;
	TransformPtr mTransform;
	SkyBoxPtr mSkyBox;
	std::vector<PostProcessPtr> mPostProcessEffects;
	IFrameBufferPtr mPostProcessInput, mOutput;
	
	RenderingPath mRenderPath = kRenderPathForward;
	unsigned mCameraMask = -1;
	unsigned mDepth = 0;
private:
	CameraType mType;
	//Eigen::Vector2i mScreenSize, mSize;
	Eigen::Vector3f mUpVector;
	float mForwardLength;
	Eigen::Vector2f mZRange;
	float mFov, mOrthoSize, mAspect;
	bool mFlipY;

	mutable Eigen::Matrix4f mView, mProjection, mWorldView;
	mutable bool mViewDirty, mProjectionDirty;
};

}