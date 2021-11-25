#pragma once
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#include "core/renderable/predeclare.h"
#include "core/rendersys/predeclare.h"
#include "core/rendersys/base_type.h"
#include "core/base/transform.h"

namespace mir {

class MIR_CORE_API Camera : boost::noncopyable
{
public:
	static CameraPtr CreatePerspective(RenderSystem& renderSys, const Eigen::Vector2i& size, 
		Eigen::Vector3f eyePos = Eigen::Vector3f(0,0,-10), double far1 = 100, double fov = 45.0);
	static CameraPtr CreateOthogonal(RenderSystem& renderSys, const Eigen::Vector2i& size, 
		Eigen::Vector3f eyePos = Eigen::Vector3f(0,0,-10), double far1 = 100);
	Camera(RenderSystem& renderSys);

	void SetLookAt(const Eigen::Vector3f& eye, const Eigen::Vector3f& at);
	void SetPerspectiveProj(const Eigen::Vector2i& size, double fov, double zFar);
	void SetOthogonalProj(const Eigen::Vector2i& size, double zFar);
	
	void SetFlipY(bool flip);
	void SetSkyBox(const SkyBoxPtr& skybox);
	void AddPostProcessEffect(const PostProcessPtr& postEffect);
	IRenderTargetPtr FetchPostProcessInput();
public:
	const TransformPtr& GetTransform() const;
	const SkyBoxPtr& SkyBox() const { return mSkyBox; }
	const std::vector<PostProcessPtr>& PostProcessEffects() const { return mPostProcessEffects; }

	const Eigen::Matrix4f& GetView() const;
	const Eigen::Matrix4f& GetProjection() const  { return mProjection; }
	
	int GetWidth() const { return mSize.x(); }
	int GetHeight() const  { return mSize.y(); }
	
	Eigen::Vector3f ProjectPoint(const Eigen::Vector3f& worldpos) const;//world -> ndc
	Eigen::Vector4f ProjectPoint(const Eigen::Vector4f& worldpos) const;
private:
	RenderSystem& mRenderSys;

	SkyBoxPtr mSkyBox;
	std::vector<PostProcessPtr> mPostProcessEffects;

	TransformPtr mTransform;
	mutable bool mTransformDirty;

	bool mFlipY;
	mutable Eigen::Matrix4f mView, mProjection, mWorldView;
public:
	IRenderTargetPtr mPostProcessInput;

	Eigen::Vector2i mSize;
	Eigen::Vector3f mEyePos, mLookAtPos, mUpVector;

	bool mIsPespective;
	double mFov, mZFar;
};

}