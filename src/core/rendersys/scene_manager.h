#pragma once
#include <boost/noncopyable.hpp>
#include "core/rendersys/predeclare.h"
#include "core/renderable/predeclare.h"
#include "core/rendersys/base_type.h"
#include "core/rendersys/material_cb.h"
#include "core/rendersys/camera.h"

namespace mir {

class SceneManager : boost::noncopyable 
{
public:
	RenderSystem& mRenderSys;
	MaterialFactory& mMaterialFac;
	int mScreenWidth, mScreenHeight;
	
	std::vector<CameraPtr> mCameras;

	std::vector<cbDirectLightPtr> mDirectLights;
	std::vector<cbPointLightPtr> mPointLights;
	std::vector<cbSpotLightPtr> mSpotLights;
	std::vector<std::pair<cbDirectLight*, LightType>> mLightsByOrder;
public:
	SceneManager(RenderSystem& renderSys, MaterialFactory& matFac, XMINT2 screenSize, CameraPtr defCamera);

	void RemoveAllCameras();
	CameraPtr AddOthogonalCamera(const XMFLOAT3& eyePos, double far1);
	CameraPtr AddPerspectiveCamera(const XMFLOAT3& eyePos, double far1, double fov);
	CameraPtr GetDefCamera() const;

	cbSpotLightPtr AddSpotLight();
	cbPointLightPtr AddPointLight();
	cbDirectLightPtr AddDirectLight();
};

};