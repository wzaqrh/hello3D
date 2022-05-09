#pragma once
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#include "core/predeclare.h"
#include "core/base/base_type.h"
#include "core/base/declare_macros.h"
#include "core/scene/transform.h"
#include "core/scene/component.h"

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

class MIR_CORE_API Camera : public Component
{
	friend class CameraFactory;
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	Camera(ResourceManager& resMng);

	void SetLookAt(const Eigen::Vector3f& eye, const Eigen::Vector3f& at);
	void SetForward(const Eigen::Vector3f& forward);
	void SetClippingPlane(const Eigen::Vector2f& zRange);
	const Eigen::Vector2f& GetClippingPlane() const { return mClipPlane; }
	void SetAspect(float aspect);
	float GetAspect() const { return mAspect; }
	void SetOrthographicSize(float size);
	void SetFov(float fov);
	float GetFov() const { return mFov; }

	void SetCullingMask(unsigned mask) { mCameraMask = mask; }
	unsigned GetCullingMask() const { return mCameraMask; }

	void SetDepth(unsigned depth) { mDepth = depth; }
	unsigned GetDepth() const { return mDepth; }

	void SetRenderingPath(RenderingPath path) { mRenderPath = path; }
	RenderingPath GetRenderingPath() const { return mRenderPath; }

	void SetSkyBox(const rend::SkyBoxPtr& skybox);
	void AddPostProcessEffect(const rend::PostProcessPtr& postEffect);
	IFrameBufferPtr FetchOutput2PostProcess(std::vector<ResourceFormat> formats = { kFormatR8G8B8A8UNorm,kFormatD24UNormS8UInt });
	IFrameBufferPtr SetOutput(float scale = 1, std::vector<ResourceFormat> formats = { kFormatR8G8B8A8UNorm,kFormatD24UNormS8UInt });
	IFrameBufferPtr SetOutput(IFrameBufferPtr output);
private:
	void SetType(CameraType type);
	void RecalculateProjection() const;
	void RecalculateView() const;
public:
	const TransformPtr& GetTransform() const;
	const rend::SkyBoxPtr& GetSkyBox() const { return mSkyBox; }
	const std::vector<rend::PostProcessPtr>& GetPostProcessEffects() const { return mPostProcessEffects; }
	const IFrameBufferPtr& GetPostProcessInput() const { return mPostProcessInput; }
	const IFrameBufferPtr& GetOutput() const { return mOutput; }
	
	const Eigen::Matrix4f& GetView() const;
	const Eigen::Matrix4f& GetProjection() const;
	Eigen::Vector3f ProjectPoint(const Eigen::Vector3f& worldpos) const;//world -> ndc
	Eigen::Vector4f ProjectPoint(const Eigen::Vector4f& worldpos) const;
	math::Frustum GetFrustum() const;

	CameraType GetType() const { return mType; }
	Eigen::Vector2f GetOthoWinSize() const;
public:
	CoTask<void> UpdateFrame(float dt);
private:
	ResourceManager& mResMng;
	TransformPtr mTransform;
	rend::SkyBoxPtr mSkyBox;
	std::vector<rend::PostProcessPtr> mPostProcessEffects;
	IFrameBufferPtr mPostProcessInput, mOutput;
	
	RenderingPath mRenderPath;
	unsigned mCameraMask;
	unsigned mDepth;
private:
	CameraType mType;
	Eigen::Vector2f mClipPlane;//near, far
	float mFov, mOrthoSize;
	float mAspect;

	mutable Eigen::Matrix4f mView, mProjection, mWorldView;
	mutable bool mViewDirty, mProjectionDirty;
};

class MIR_CORE_API CameraFactory : boost::noncopyable {
public:
	CameraFactory(ResourceManager& resMng) :mResMng(resMng) {}
	void SetPixelPerUnit(float ppu) { mPixelPerUnit = ppu; }

	CameraPtr CreatePerspective(float aspect = 1.0,
		const Eigen::Vector3f& eyePos = math::cam::DefEye(),
		const Eigen::Vector3f& length_forward = math::vec::Forward(),
		const Eigen::Vector2f& clipPlane = math::cam::DefClippingPlane(),
		float fov = math::cam::DefFov(),
		unsigned cameraMask = -1);
	CameraPtr CreateOthogonal(float aspect = 1.0,
		const Eigen::Vector3f& eyePos = math::cam::DefEye(),
		const Eigen::Vector3f& length_forward = math::vec::Forward(),
		const Eigen::Vector2f& clipPlane = math::cam::DefClippingPlane(),
		float othoSize = math::cam::DefOthoSize(),
		unsigned cameraMask = -1);
	template <CameraType CamType, typename... T> CameraPtr CreateCamera(T &&...args) {
		return CreateCameraFunctor<CamType>()(std::forward<T>(args)...);
	}

	CameraPtr CreateDefPerspective(const Eigen::Vector3f& eyePos = math::cam::DefEye());
	CameraPtr CreateDefOthogonal(const Eigen::Vector3f& eyePos = math::cam::DefEye());
	TemplateArgs CameraPtr CreateDefCamera(CameraType camType, T &&...args) {
		return (camType == kCameraPerspective) ? CreateDefPerspective(std::forward<T>(args)...) : CreateDefOthogonal(std::forward<T>(args)...);
	}
private:
	template<CameraType CamType> struct CreateCameraFunctor {};
	template<> struct CreateCameraFunctor<kCameraPerspective> {
		TemplateArgs CameraPtr operator()(CameraFactory& __this, T &&...args) const { return __this.CreatePerspective(std::forward<T>(args)...); }
	};
	template<> struct CreateCameraFunctor<kCameraOthogonal> {
		TemplateArgs CameraPtr operator()(CameraFactory& __this, T &&...args) const { return __this.CreateOthogonal(std::forward<T>(args)...); }
	};
private:
	float mPixelPerUnit = 100;
	ResourceManager& mResMng;
};

}
}