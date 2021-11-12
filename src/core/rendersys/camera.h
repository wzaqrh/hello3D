#pragma once
#include <boost/noncopyable.hpp>
#include "core/renderable/predeclare.h"
#include "core/rendersys/predeclare.h"
#include "core/rendersys/base_type.h"
#include "core/base/transform.h"

namespace mir {

class __declspec(align(16)) Camera : boost::noncopyable
{
public:
	static CameraPtr CreatePerspective(RenderSystem& renderSys,int width, int height, 
		Eigen::Vector3f eyePos = Eigen::Vector3f(0,0,-10), 
		double far1 = 100, double fov = 45.0);
	static CameraPtr CreateOthogonal(RenderSystem& renderSys, int width, int height, 
		Eigen::Vector3f eyePos = Eigen::Vector3f(0,0,-10),
		double far1 = 100);
	Camera(RenderSystem& renderSys);

	void SetLookAt(const Eigen::Vector3f& eye, const Eigen::Vector3f& at);
	void SetPerspectiveProj(int width, int height, double fov, double zFar);
	void SetOthogonalProj(int width, int height, double zFar);
	
	void SetFlipY(bool flip);
	void SetSkyBox(const SkyBoxPtr& skybox);
	void AddPostProcessEffect(const PostProcessPtr& postEffect);
	IRenderTexturePtr FetchPostProcessInput();
public:
	const TransformPtr& GetTransform() const;
	const SkyBoxPtr& SkyBox() const { return mSkyBox; }
	const std::vector<PostProcessPtr>& PostProcessEffects() const { return mPostProcessEffects; }

	const XMMATRIX& GetView() const;
	const XMMATRIX& GetProjection() const  { return AS_CONST_REF(XMMATRIX, mProjection); }
	
	int GetWidth() const { return mWidth; }
	int GetHeight() const  { return mHeight; }
	
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
	IRenderTexturePtr mPostProcessInput;

	int mWidth, mHeight;
	Eigen::Vector3f mEyePos, mLookAtPos, mUpVector;

	bool mIsPespective;
	double mFov, mZFar;
};

}