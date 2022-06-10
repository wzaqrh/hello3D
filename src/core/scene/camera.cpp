#include <boost/math/constants/constants.hpp>
#include "core/resource/resource_manager.h"
#include "core/renderable/post_process.h"
#include "core/renderable/skybox.h"
#include "core/scene/camera.h"
#include "core/base/debug.h"
#include "core/base/math.h"

namespace mir {
namespace scene {

Camera::Camera(ResourceManager& resMng)
: mResMng(resMng)
{
	DEBUG_MEM_ALLOC_TAG(camera); 
}
Camera::~Camera()
{
	DEBUG_MEM_DEALLOC_TAG(camera);
}

void Camera::PostSetComponent()
{
	if (auto transform = GetTransform())
		transform->GetSignal().Connect(mTransformSlot);
}

void Camera::SetType(CameraType type)
{
	mType = type;
	mProjSignal();
}

void Camera::SetClippingPlane(const Eigen::Vector2f& zRange)
{
	mClipPlane = zRange;
	mProjSignal();
}
void Camera::SetAspect(float aspect)
{
	mAspect = aspect;
	mProjSignal();
}
void Camera::SetFov(float fov)
{
	mFov = fov / boost::math::constants::radian<float>();
	mProjSignal();
}

void Camera::SetReverseZ(bool reverseZ)
{
	mReverseZ = reverseZ;
	mProjSignal();
}

void Camera::SetBackgroundColor(const Eigen::Vector4f& color)
{
	mBackgroundColor = color;
}

void Camera::SetOrthographicSize(float size)
{
	mOrthoSize = size;
	mProjSignal();
}
void Camera::SetLookAt(const Eigen::Vector3f& eye, const Eigen::Vector3f& at)
{
	GetTransform()->SetPosition(eye);
	GetTransform()->LookAt(at);
	mProjSignal();
}
void Camera::SetForward(const Eigen::Vector3f& forward)
{
	GetTransform()->LookForward(forward);
	mProjSignal();
}

CoTask<void> Camera::UpdateFrame(float dt)
{
	for (auto& effect : mPostProcessEffects)
		CoAwait effect->UpdateFrame(dt);
#if MIR_MATERIAL_HOTLOAD
	if (mSkyBox) CoAwait mSkyBox->UpdateFrame(dt);
#endif
	CoReturn;
}

/********** query **********/
Eigen::Vector2f Camera::GetOthoWinSize() const
{
	BOOST_ASSERT(mType == kCameraOthogonal);
	float winHeight = mOrthoSize * 2;
	return Eigen::Vector2f(winHeight * mAspect, winHeight);
}

//#define OTHO_PROJ_CENTER_NOT_ZERO
void Camera::RecalculateProjection() const
{
	switch (mType) {
	case kCameraPerspective:
		if (mReverseZ) mProjection = math::cam::MakePerspectiveFovLHReversZ(mFov, mAspect, mClipPlane[0], mClipPlane[1]);
		else mProjection = math::cam::MakePerspectiveFovLH(mFov, 1.0, mClipPlane[0], mClipPlane[1]);
		break;
	case kCameraOthogonal: {
	#if defined OTHO_PROJ_CENTER_NOT_ZERO
		Eigen::Vector2f othoWin = GetOthoWinSize();
		mProjection = math::cam::MakeOrthographicOffCenterLH(0, othoWin.x(),
			0, othoWin.y(),
			mClipPlane.x(), mClipPlane.y());

		auto lb = Transform3Projective(mProjection) * Eigen::Vector4f(0, 0, 0, 1); lb /= lb.w();
		BOOST_ASSERT(test::IsEqual(lb.head<2>(), Eigen::Vector2f(-1, -1)));
		auto rt = Transform3Projective(mProjection) * Eigen::Vector4f(othoWin.x(), othoWin.y(), 0, 1); rt /= rt.w();
		BOOST_ASSERT(test::IsEqual(rt.head<2>(), Eigen::Vector2f(1, 1)));
	#else
		Eigen::Vector2f othoWin = GetOthoWinSize() / 2;
		mProjection = math::cam::MakeOrthographicOffCenterLH(-othoWin.x(), othoWin.x(),
			-othoWin.y(), othoWin.y(),
			mClipPlane.x(), mClipPlane.y());

		/*auto lb = Transform3Projective(mProjection) * Eigen::Vector4f(-othoWin.x(), -othoWin.y(), 0, 1); lb /= lb.w();
		BOOST_ASSERT(test::IsEqual(lb.head<2>(), Eigen::Vector2f(-1, -1)));
		auto rt = Transform3Projective(mProjection) * Eigen::Vector4f(othoWin.x(), othoWin.y(), 0,1); rt /= rt.w();
		BOOST_ASSERT(test::IsEqual(rt.head<2>(), Eigen::Vector2f(1, 1)));*/
	#endif
	}break;
	default:
		break;
	}

#if 0
	if (mFlipY) {
		mProjection = Transform3Projective(mProjection)
			.prescale(Eigen::Vector3f(1, -1, 1))
			.matrix();
	}
#endif
}
const Eigen::Matrix4f& Camera::GetProjection() const
{
	if (mProjSignal.Slot.AcquireSignal()) {
		RecalculateProjection();
	}
	return mProjection;
}

void Camera::RecalculateView() const
{
	auto transform = GetTransform();
	auto eyePos = transform->GetLocalPosition();
	auto forward = transform->GetForward();
	auto up = transform->GetUp();
	mView = math::cam::MakeLookForwardLH(eyePos, forward, up);

	Transform3fAffine t(Transform3fAffine::Identity());
	{
		auto s = transform->GetLocalScale();
		t.prescale(Eigen::Vector3f(1 / s.x(), 1 / s.y(), 1 / s.z()));

	#if defined OTHO_PROJ_CENTER_NOT_ZERO
		if (mType == kCameraOthogonal) {
			Eigen::Vector2f othoWin = GetOthoWinSize();
			Eigen::Vector3f center(othoWin.x() / 2, othoWin.y() / 2, 0);
			t.pretranslate(center);//[-screen.hw, screen.hw] -> [0->sreen.w]
		}
	#endif
	}
	mWorldView = t.matrix() * mView;

	/*auto lb = Transform3Projective(mWorldView) * Eigen::Vector4f(-5, -5, 0, 1); lb /= lb.w();
	auto rt = Transform3Projective(mWorldView) * Eigen::Vector4f(5, 5, 0, 1); rt /= rt.w();*/
}
const Eigen::Matrix4f& Camera::GetView() const
{
	if (mTransformSlot.AcquireSignal()) {
		RecalculateView();
	}
	return mWorldView;
}

Eigen::Vector4f Camera::ProjectPoint(const Eigen::Vector4f& pos) const
{
	Eigen::Vector4f perspective = Transform3Projective(GetProjection() * GetView()) * pos;

	if (perspective.w() != 0) {
		perspective.head<3>() /= perspective.w();
		perspective.w() = 1;
	}
	else {
		perspective = Eigen::Vector4f(0, 0, 0, 0);
	}
	return perspective;
}
Eigen::Vector3f Camera::ProjectPoint(const Eigen::Vector3f& pos) const
{
	return ProjectPoint(Eigen::Vector4f(pos.x(), pos.y(), pos.z(), 1.0)).head<3>();
}

math::Frustum Camera::GetFrustum() const
{
	math::Frustum ft;
	float tan_hfov = tanf(mFov * 0.5f);
	float n  = mClipPlane[0];
	float nh = tan_hfov * n;
	float nw = nh * mAspect;
	ft.FarPlane[0] = Eigen::Vector3f(-nw,-nh,n);
	ft.FarPlane[1] = Eigen::Vector3f( nw,-nh,n);
	ft.FarPlane[2] = Eigen::Vector3f( nw, nh,n);
	ft.FarPlane[3] = Eigen::Vector3f(-nw, nh,n);

	float f  = mClipPlane[1];
	float fh = tan_hfov * f;
	float fw = fh * mAspect;
	ft.FarPlane[0] = Eigen::Vector3f(-nw, -nh, f);
	ft.FarPlane[1] = Eigen::Vector3f(nw, -nh, f);
	ft.FarPlane[2] = Eigen::Vector3f(nw, nh, f);
	ft.FarPlane[3] = Eigen::Vector3f(-nw, nh, f);

	return ft;
}

/********** components **********/
void Camera::SetSkyBox(const rend::SkyBoxPtr& skybox)
{
	mSkyBox = skybox;
}

void Camera::AddPostProcessEffect(const rend::PostProcessPtr& postEffect)
{
	if (mPostProcessEffects.empty() && mPostProcessInput == NULL)
		FetchOutput2PostProcess();

	mPostProcessEffects.push_back(postEffect);
}
IFrameBufferPtr Camera::FetchOutput2PostProcess(std::vector<ResourceFormat> formats)
{
	if (mPostProcessInput == nullptr) {
		mPostProcessInput = mResMng.CreateFrameBuffer(__LaunchSync__, Eigen::Vector3i(mResMng.WinWidth(), mResMng.WinHeight(), 1), formats);
		DEBUG_SET_PRIV_DATA(mPostProcessInput, "camera.output_to_post_process");
	}
	return mPostProcessInput;
}

IFrameBufferPtr Camera::SetOutput(IFrameBufferPtr output)
{
	mOutput = output;
	DEBUG_SET_PRIV_DATA(mOutput, "camera.output");
	return mOutput;
}
IFrameBufferPtr Camera::SetOutput(float scale, std::vector<ResourceFormat> formats)
{
	return SetOutput(mResMng.CreateFrameBuffer(__LaunchSync__, Eigen::Vector3i(mResMng.WinWidth() * scale, mResMng.WinHeight() * scale, 1), formats));
}

/********** CameraFactory **********/
CameraPtr CameraFactory::CreatePerspective(SceneNodePtr node)
{
	CameraPtr camera = DoCreateAddCamera(node);

	camera->SetType(kCameraPerspective);
	camera->SetFov(mDefault.Fov);
	camera->SetAspect(mDefault.Aspect);
	camera->SetReverseZ(mDefault.ReverseZ);
	camera->SetClippingPlane(mDefault.ClipPlane);
	camera->SetLookAt(mDefault.EyePos, mDefault.EyePos + mDefault.LengthForward);
	camera->SetCullingMask(mDefault.CameraMask);

	return camera;
}

CameraPtr CameraFactory::CreateOthogonal(SceneNodePtr node)
{
	CameraPtr camera = DoCreateAddCamera(node);

	camera->SetType(kCameraOthogonal);
	camera->SetOrthographicSize(mDefault.OthoSize);
	camera->SetAspect(mDefault.Aspect);
	camera->SetClippingPlane(mDefault.ClipPlane);
	camera->SetLookAt(mDefault.EyePos, mDefault.EyePos + mDefault.LengthForward);
	camera->SetCullingMask(mDefault.CameraMask);

	return camera;
}

CameraPtr CameraFactory::DoCreateAddCamera(SceneNodePtr node)
{
	CameraPtr camera = CreateInstance<Camera>(mResMng);
	node->SetCamera(camera);

#if CAMERA_FAC_CACHE
	mCameras.push_back(camera);
	mSignal();
#endif

	return camera;
}

#if CAMERA_FAC_CACHE
const std::vector<mir::scene::CameraPtr>& CameraFactory::GetCameras() const
{
	ResortCameras();
	return mCameras;
}

void CameraFactory::ResortCameras() const
{
	if (mSignal.Slot.AcquireSignal()) {
		struct CompCameraByDepth {
			bool operator()(const CameraPtr& l, const CameraPtr& r) const {
				return l->GetDepth() < r->GetDepth();
			}
		};
		std::stable_sort(mCameras.begin(), mCameras.end(), CompCameraByDepth());
	}
}
#endif

}
}