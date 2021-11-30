#pragma once
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#include "core/predeclare.h"
#include "core/rendersys/base_type.h"
#include "core/scene/camera.h"
#include "core/scene/light.h"

namespace mir {

class MIR_CORE_API SceneManager : boost::noncopyable 
{
public:
	SceneManager(ResourceManager& resMng, const Eigen::Vector2i& screenSize, CameraPtr defCamera);
	
	void RemoveAllCameras();
	CameraPtr AddOthogonalCamera(const Eigen::Vector3f& eyePos, double far1, unsigned camMask = -1);
	CameraPtr AddPerspectiveCamera(const Eigen::Vector3f& eyePos, double far1, double fov, unsigned camMask = -1);

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
	size_t GetLightCount() const { return mLightsByOrder.size(); }
	ILightPtr GetLight(size_t index) const { return GetLights()[index]; }
	ILightPtr GetDefLight() const { return GetLightCount() ? GetLight(0) : nullptr; }
private:
	void ResortLights() const;
	void ResortCameras() const;
private:
	ResourceManager& mResMng;
	Eigen::Vector2i mScreenSize;

	mutable std::vector<CameraPtr> mCameras;
	mutable bool mCamerasDirty;

	mutable std::vector<ILightPtr> mLightsByOrder;
};

};