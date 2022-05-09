#pragma once
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#include "core/predeclare.h"
#include "core/base/base_type.h"
#include "core/scene/camera.h"
#include "core/scene/light.h"
#include "core/renderable/renderable_factory.h"

namespace mir {

class MIR_CORE_API SceneManager : boost::noncopyable
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	SceneManager(ResourceManager& resMng, RenderableFactoryPtr rendFac);
	void SetPixelPerUnit(float ppu);

	SceneNodePtr AddNode();

	template<typename... T> scene::CameraPtr CreateCamera(CameraType CamType, T &&...args) {
		return mCameraFac->CreateDefCamera(CamType, std::forward<T>(args)...);
	}
	template<typename... T> scene::CameraPtr CreateAddCameraNode(CameraType CamType, T &&...args) {
		scene::CameraPtr camera = CreateCamera(CamType, std::forward<T>(args)...);
		CreateAddCameraNode(camera);
		return camera;
	}
	scene::CameraPtr CreateAddCameraNode(scene::CameraPtr camera);
	void RemoveAllCameras();

	template<typename LightClass, typename... T> std::shared_ptr<LightClass> CreateLight(T &&...args) {
		return mLightFac->CreateLight<LightClass>(std::forward<T>(args)...);
	}
	template<typename LightClass, typename... T> std::shared_ptr<LightClass> CreateAddLightNode(T &&...args) {
		std::shared_ptr<LightClass> light = CreateLight<LightClass>(std::forward<T>(args)...);
		AddLightNode(light);
		return light;
	}
	template<typename LightClass> const std::shared_ptr<LightClass>& AddLightNode(const std::shared_ptr<LightClass>& light) { AddLightNode((scene::LightPtr)light); return light; }
	scene::LightPtr AddLightNode(scene::LightPtr light);
	void RemoveAllLights();

	template<typename RendClass, typename... T> std::shared_ptr<RendClass> CreateRend(T &&...args) {
		return mRendFac->CreateRend<RendClass>(std::forward<T>(args)...);
	}
	template<typename RendClass, typename... T> std::shared_ptr<RendClass> CreateAddRendNode(T &&...args) {
		std::shared_ptr<RendClass> rend = CreateRend<RendClass>(std::forward<T>(args)...);
		AddRendNode(rend);
		return rend;
	}
	template<typename RendClass> const std::shared_ptr<RendClass>& AddRendNode(std::shared_ptr<RendClass> rend) { AddRendNode(std::static_pointer_cast<Renderable>(rend)); return std::static_pointer_cast<RendClass>(rend); }
	RenderablePtr AddRendNode(RenderablePtr rend);
public:
	Eigen::AlignedBox3f GetWorldAABB() const;

	const std::vector<scene::CameraPtr>& GetCameras() const { return mCameras; }
	scene::CameraPtr GetDefCamera() const { return GetCameras().size() ? GetCameras()[0] : nullptr; }

	const std::vector<scene::LightPtr>& GetLights() const { return mLights; }
	scene::LightPtr GetDefLight() const { return GetLights().size() ? GetLights()[0] : nullptr; }

	const scene::LightFactoryPtr& GetLightFac() const { return mLightFac; }
	const scene::CameraFactoryPtr& GetCameraFac() const { return mCameraFac; }
	const SceneNodeFactoryPtr& GetNodeFac() const { return mNodeFac; }
private:
	void ResortLights();
	void ResortCameras();
public:
	CoTask<void> UpdateFrame(float dt);
	void GenRenderOperation(RenderOperationQueue& opQue);
private:
	ResourceManager& mResMng;

	scene::LightFactoryPtr mLightFac;
	scene::CameraFactoryPtr mCameraFac;
	SceneNodeFactoryPtr mNodeFac;
	RenderableFactoryPtr mRendFac;

	std::vector<SceneNodePtr> mNodes;
	std::vector<RenderablePtr> mRends;
	std::vector<scene::CameraPtr> mCameras;
	std::vector<scene::LightPtr> mLights;
	bool mRendsDirty, mCamerasDirty, mLightsDirty;
#if MIR_GRAPHICS_DEBUG
	rend::Paint3DPtr mDebugPaint;
#endif
};

};