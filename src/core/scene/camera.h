#pragma once
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#include "core/predeclare.h"
#include "core/base/base_type.h"
#include "core/base/declare_macros.h"
#include "core/base/deffered_signal.h"
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
	typedef Component Super;
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	Camera(ResourceManager& resMng);

	void SetLookAt(const Eigen::Vector3f& eye, const Eigen::Vector3f& at);
	void SetForward(const Eigen::Vector3f& forward);
	void SetOrthographicSize(float size);
	void SetClippingPlane(const Eigen::Vector2f& zRange);
	void SetAspect(float aspect);
	void SetFov(float fov);
	
	void SetCullingMask(unsigned mask) { mCameraMask = mask; }
	void SetDepth(unsigned depth) { mDepth = depth; }
	void SetRenderingPath(RenderingPath path) { mRenderPath = path; }

	CameraType GetType() const { return mType; }
	Eigen::Vector2f GetOthoWinSize() const;
	const Eigen::Vector2f& GetClippingPlane() const { return mClipPlane; }
	float GetAspect() const { return mAspect; }
	float GetFov() const { return mFov; }
	unsigned GetCullingMask() const { return mCameraMask; }
	unsigned GetDepth() const { return mDepth; }
	RenderingPath GetRenderingPath() const { return mRenderPath; }

	void SetSkyBox(const rend::SkyBoxPtr& skybox);
	void AddPostProcessEffect(const rend::PostProcessPtr& postEffect);
	IFrameBufferPtr FetchOutput2PostProcess(std::vector<ResourceFormat> formats = { kFormatR8G8B8A8UNorm,kFormatD24UNormS8UInt });
	IFrameBufferPtr SetOutput(float scale = 1, std::vector<ResourceFormat> formats = { kFormatR8G8B8A8UNorm,kFormatD24UNormS8UInt });
	IFrameBufferPtr SetOutput(IFrameBufferPtr output);

	const rend::SkyBoxPtr& GetSkyBox() const { return mSkyBox; }
	const std::vector<rend::PostProcessPtr>& GetPostProcessEffects() const { return mPostProcessEffects; }
	const IFrameBufferPtr& GetPostProcessInput() const { return mPostProcessInput; }
	const IFrameBufferPtr& GetOutput() const { return mOutput; }
	
	const Eigen::Matrix4f& GetView() const;
	const Eigen::Matrix4f& GetProjection() const;
	Eigen::Vector3f ProjectPoint(const Eigen::Vector3f& worldpos) const;//world -> ndc
	Eigen::Vector4f ProjectPoint(const Eigen::Vector4f& worldpos) const;
	math::Frustum GetFrustum() const;
public:
	CoTask<void> UpdateFrame(float dt);
	void PostSetComponent() override;
private:
	void SetType(CameraType type);
	void RecalculateProjection() const;
	void RecalculateView() const;
private:
	ResourceManager& mResMng;
	rend::SkyBoxPtr mSkyBox;
	std::vector<rend::PostProcessPtr> mPostProcessEffects;
	IFrameBufferPtr mPostProcessInput, mOutput;
	
	RenderingPath mRenderPath = kRenderPathForward;
	unsigned mCameraMask = (unsigned)-1;
	unsigned mDepth = 0;
private:
	CameraType mType = kCameraPerspective;
	float mOrthoSize = 5;
	float mAspect = 1;
	float mFov = 60;
	Eigen::Vector2f mClipPlane = Eigen::Vector2f(0.3f, 1000);//near, far
	
	mutable Eigen::Matrix4f mView, mProjection, mWorldView;
	mutable DefferedConnctedSignal mProjSignal;
	mutable DefferedSlot mTransformSlot;
};

class MIR_CORE_API CameraFactory : boost::noncopyable {
public:
	CameraFactory(ResourceManager& resMng) :mResMng(resMng) {}
#if CAMERA_FAC_CACHE
	const DefferedSignal& GetSignal() const { return mSignal; }
#endif

	void SetPixelPerUnit(float ppu) { mDefault.PixelPerUnit = ppu; }
	void SetOthographicSize(float othoSize) { mDefault.OthoSize = othoSize; }
	void SetAspect(float aspect) { mDefault.Aspect = aspect; }
	void SetFov(float fov) { mDefault.Fov = fov; }
	void SetClipPlane(Eigen::Vector2f clipPlane) { mDefault.ClipPlane = clipPlane; }
	void SetEyePos(Eigen::Vector3f eyePos) { mDefault.EyePos = eyePos; }
	void SetForward(Eigen::Vector3f forward) { mDefault.LengthForward = forward; }
	void SetForward(unsigned cameraMask) { mDefault.CameraMask = cameraMask; }

	CameraPtr CreatePerspective(SceneNodePtr node);
	CameraPtr CreateOthogonal(SceneNodePtr node);
	TemplateArgs CameraPtr CreateCameraByType(SceneNodePtr node, CameraType camType, T &&...args) {
		return (camType == kCameraPerspective)
			? CreatePerspective(node, std::forward<T>(args)...)
			: CreateOthogonal(node, std::forward<T>(args)...);
	}

#if CAMERA_FAC_CACHE
	const std::vector<scene::CameraPtr>& GetCameras() const;
	scene::CameraPtr GetDefCamera() const { return GetCameras().size() ? GetCameras()[0] : nullptr; }
#endif
private:
	CameraPtr DoCreateAddCamera(SceneNodePtr node);
	void ResortCameras() const;
private:
	struct CameraDefault {
		float PixelPerUnit = 100;//othographic
		float OthoSize = math::cam::DefOthoSize();//othographic
		float Aspect = 1.0;//perspective
		float Fov = 60.0f;//perspective
		Eigen::Vector2f ClipPlane = math::cam::DefClippingPlane();
		Eigen::Vector3f EyePos = math::cam::DefEye();
		Eigen::Vector3f LengthForward = math::vec::Forward();
		unsigned CameraMask = -1;
	} mDefault;
	ResourceManager& mResMng;
#if CAMERA_FAC_CACHE
	mutable std::vector<scene::CameraPtr> mCameras;
	mutable DefferedConnctedSignal mSignal;
#endif
};

}
}