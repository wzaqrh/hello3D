#pragma once
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#include "core/predeclare.h"
#include "core/base/base_type.h"
#include "core/scene/camera.h"
#include "core/scene/light.h"

namespace mir {

class MIR_CORE_API SceneManager : boost::noncopyable 
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	SceneManager(ResourceManager& resMng);
	void SetPixelPerUnit(float ppu) { mPixelPerUnit = ppu; }

	void RemoveAllCameras();
	scene::CameraPtr AddPerspectiveCamera(const Eigen::Vector3f& eyePos = math::cam::DefEye(), unsigned camMask = -1);
	scene::CameraPtr AddOthogonalCamera(const Eigen::Vector3f& eyePos = math::cam::DefEye(), unsigned camMask = -1);
	TemplateArgs scene::CameraPtr AddCameraByType(CameraType camType, T &&...args) {
		return (camType == kCameraPerspective) ? AddPerspectiveCamera(std::forward<T>(args)...) : AddOthogonalCamera(std::forward<T>(args)...);
	}

	void RemoveAllLights();
	scene::SpotLightPtr AddSpotLight(unsigned camMask = -1);
	scene::PointLightPtr AddPointLight(unsigned camMask = -1);
	scene::DirectLightPtr AddDirectLight(unsigned camMask = -1);
public:
	const std::vector<scene::CameraPtr>& GetCameras() const;
	size_t GetCameraCount() const { return mCameras.size(); }
	scene::CameraPtr GetCamera(size_t index) const { return GetCameras()[index];  }
	scene::CameraPtr GetDefCamera() const { return GetCameraCount() ? GetCamera(0) : nullptr;  }

	const std::vector<scene::ILightPtr>& GetLights() const;
	size_t GetLightCount() const { return mLights.size(); }
	scene::ILightPtr GetLight(size_t index) const { return GetLights()[index]; }
	scene::ILightPtr GetDefLight() const { return GetLightCount() ? GetLight(0) : nullptr; }
private:
	void ResortLights() const;
	void ResortCameras() const;
private:
	ResourceManager& mResMng;
	float mPixelPerUnit = 100;

	mutable std::vector<scene::CameraPtr> mCameras;
	mutable std::vector<scene::ILightPtr> mLights;
	mutable bool mCamerasDirty, mLightsDirty;
};

};