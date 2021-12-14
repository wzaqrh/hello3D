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
	SceneManager(ResourceManager& resMng);
	void SetPixelPerUnit(float ppu) { mPixelPerUnit = ppu; }

	void RemoveAllCameras();
	CameraPtr AddOthogonalCamera(const Eigen::Vector3f& eyePos = math::cam::DefEye(), unsigned camMask = -1);
	CameraPtr AddPerspectiveCamera(const Eigen::Vector3f& eyePos = math::cam::DefEye(), unsigned camMask = -1);

	void RemoveAllLights();
	SpotLightPtr AddSpotLight(unsigned camMask = -1);
	PointLightPtr AddPointLight(unsigned camMask = -1);
	DirectLightPtr AddDirectLight(unsigned camMask = -1);
public:
	const std::vector<CameraPtr>& GetCameras() const;
	size_t GetCameraCount() const { return mCameras.size(); }
	CameraPtr GetCamera(size_t index) const { return GetCameras()[index];  }
	CameraPtr GetDefCamera() const { return GetCameraCount() ? GetCamera(0) : nullptr;  }

	const std::vector<ILightPtr>& GetLights() const;
	size_t GetLightCount() const { return mLights.size(); }
	ILightPtr GetLight(size_t index) const { return GetLights()[index]; }
	ILightPtr GetDefLight() const { return GetLightCount() ? GetLight(0) : nullptr; }
private:
	void ResortLights() const;
	void ResortCameras() const;
private:
	ResourceManager& mResMng;
	float mPixelPerUnit = 100;

	mutable std::vector<CameraPtr> mCameras;
	mutable std::vector<ILightPtr> mLights;
	mutable bool mCamerasDirty, mLightsDirty;
};

};