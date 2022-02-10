#pragma once
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#include "core/predeclare.h"
#include "core/base/base_type.h"
#include "core/scene/transform.h"
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

namespace scene {

class MIR_CORE_API Camera : boost::noncopyable
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	TemplateArgs static CameraPtr CreatePerspective(ResourceManager& resMng, T &&...args) {
		CameraPtr camera = CreateInstance<Camera>(resMng);
		auto size = resMng.WinSize();
		camera->InitAsPerspective(1.0f*size.x()/size.y(), std::forward<T>(args)...);
		return camera;
	}
	TemplateArgs static CameraPtr CreateOthogonal(ResourceManager& resMng, T &&...args) {
		CameraPtr camera = CreateInstance<Camera>(resMng);
		auto size = resMng.WinSize();
		camera->InitAsOthogonal(1.0f*size.x()/size.y(), std::forward<T>(args)...);
		return camera;
	}
	Camera(ResourceManager& resMng);

	void SetLookAt(const Eigen::Vector3f& eye, const Eigen::Vector3f& at);
	void SetForward(const Eigen::Vector3f& forward);
	void SetClippingPlane(const Eigen::Vector2f& zRange);
	void SetAspect(float aspect);
	void SetOrthographicSize(float size);
	void SetFov(float fov);

	void SetCullingMask(unsigned mask) { mCameraMask = mask; }
	unsigned GetCullingMask() const { return mCameraMask; }

	void SetDepth(unsigned depth) { mDepth = depth; }
	unsigned GetDepth() const { return mDepth; }

	void SetRenderingPath(RenderingPath path) { mRenderPath = path; }
	RenderingPath GetRenderingPath() const { return mRenderPath; }

	void SetSkyBox(const rend::SkyBoxPtr& skybox);
	void AddPostProcessEffect(const rend::PostProcessPtr& postEffect);
	IFrameBufferPtr FetchOutput2PostProcess(ResourceFormat format = kFormatR8G8B8A8UNorm);
	IFrameBufferPtr SetOutput(float scale = 1, 
		std::vector<ResourceFormat> formats = { kFormatR8G8B8A8UNorm,kFormatD24UNormS8UInt });
	IFrameBufferPtr SetOutput(IFrameBufferPtr output);
public:
	const TransformPtr& GetTransform() const;
	const rend::SkyBoxPtr& GetSkyBox() const { return mSkyBox; }
	const std::vector<rend::PostProcessPtr>& GetPostProcessEffects() const { return mPostProcessEffects; }
	const IFrameBufferPtr& GetOutput2PostProcess() const { return mPostProcessInput; }
	const IFrameBufferPtr& GetOutput() const { return mOutput; }
	
	const Eigen::Matrix4f& GetView() const;
	const Eigen::Matrix4f& GetProjection() const;
	Eigen::Vector3f ProjectPoint(const Eigen::Vector3f& worldpos) const;//world -> ndc
	Eigen::Vector4f ProjectPoint(const Eigen::Vector4f& worldpos) const;

	CameraType GetType() const { return mType; }
	Eigen::Vector3f GetLookAt() const;
	Eigen::Vector3f GetForward() const;
	float GetForwardLength() const { return mForwardLength; }
	Eigen::Vector3f GetUp() const;
	Eigen::Vector2f GetOthoWinSize() const;
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
	rend::SkyBoxPtr mSkyBox;
	std::vector<rend::PostProcessPtr> mPostProcessEffects;
	IFrameBufferPtr mPostProcessInput, mOutput;
	
	RenderingPath mRenderPath;
	unsigned mCameraMask;
	unsigned mDepth;
private:
	CameraType mType;
	Eigen::Vector2f mClipPlane;
	float mFov, mOrthoSize;
	float mAspect, mForwardLength;

	mutable Eigen::Matrix4f mView, mProjection, mWorldView;
	mutable bool mViewDirty, mProjectionDirty;
};

}
}