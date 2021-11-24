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
	RenderSystem& mRenderSys;
	MaterialFactory& mMaterialFac;
	Eigen::Vector2i mScreenSize;
	
	std::vector<CameraPtr> mCameras;

	std::vector<DirectLightPtr> mDirectLights;
	std::vector<PointLightPtr> mPointLights;
	std::vector<SpotLightPtr> mSpotLights;
	typedef std::vector<ILightPtr> LightsByOrder;
	LightsByOrder mLightsByOrder;
public:
	SceneManager(RenderSystem& renderSys, MaterialFactory& matFac, const Eigen::Vector2i& screenSize, CameraPtr defCamera);

	void RemoveAllCameras();
	CameraPtr AddOthogonalCamera(const Eigen::Vector3f& eyePos, double far1);
	CameraPtr AddPerspectiveCamera(const Eigen::Vector3f& eyePos, double far1, double fov);
	CameraPtr GetDefCamera() const;

	SpotLightPtr AddSpotLight();
	PointLightPtr AddPointLight();
	DirectLightPtr AddDirectLight();
};

};