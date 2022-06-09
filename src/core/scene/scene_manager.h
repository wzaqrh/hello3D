#pragma once
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#include "core/predeclare.h"
#include "core/base/deffered_signal.h"
#include "core/gui/gui_manager.h"

namespace mir {

class MIR_CORE_API SceneManager : boost::noncopyable
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	SceneManager(ResourceManager& resMng, RenderableFactoryPtr rendFac, GuiManagerPtr guiMng, const Configure& cfg);
	void SetPixelPerUnit(float ppu);

	SceneNodePtr AddNode();

	template <typename CameraType, typename... T> scene::CameraPtr CreateCameraNode(CameraType cameraType, T &&...args) {
		return mCameraFac->CreateCameraByType(AddNode(), cameraType, std::forward<T>(args)...);
	}

	scene::LightPtr AddLightAsNode(scene::LightPtr light) {
		AddNode()->SetLight(light);
		return light;
	}
	template <typename LightType, typename... T> scene::LightPtr CreateLightNode(LightType type, T &&...args) {
		return AddLightAsNode(mLightFac->CreateLightByType(type, std::forward<T>(args)...));
	}
	template<typename LightClass, typename... T> std::shared_ptr<LightClass> CreateLightNode(T &&...args) {
		auto light = mLightFac->CreateLight<LightClass>(std::forward<T>(args)...);
		AddLightAsNode(light);
		return light;
	}

	gui::GuiCanvasPtr CreateGuiCanvasNode();

	template<typename RendClass> std::shared_ptr<RendClass> AddRendAsNode(std::shared_ptr<RendClass> rend) { AddRendAsNode(std::static_pointer_cast<Renderable>(rend)); return std::static_pointer_cast<RendClass>(rend); }
	RenderablePtr AddRendAsNode(RenderablePtr rend);
public:
	Eigen::AlignedBox3f GetWorldAABB() const;
	
	const std::vector<scene::CameraPtr>& GetCameras() const;
	scene::CameraPtr GetDefCamera() const { return GetCameras().size() ? GetCameras()[0] : nullptr; }

	const std::vector<scene::LightPtr>& GetLights() const;
	scene::LightPtr GetDefLight() const { return GetLights().size() ? GetLights()[0] : nullptr; }

	gui::GuiCanvasPtr GetGuiCanvas() const { return mGuiMng->GetCanvas(); }

	const scene::CameraFactoryPtr& GetCameraFac() const { return mCameraFac; }
	const scene::LightFactoryPtr& GetLightFac() const { return mLightFac; }
	const SceneNodeFactoryPtr& GetNodeFac() const { return mNodeFac; }
	const GuiManagerPtr& GetGuiMng() const { return mGuiMng; }
public:
	CoTask<void> UpdateFrame(float dt);
	void GetRenderables(RenderableCollection& rends);
private:
	ResourceManager& mResMng;

	scene::LightFactoryPtr mLightFac;
	scene::CameraFactoryPtr mCameraFac;
	SceneNodeFactoryPtr mNodeFac;
	RenderableFactoryPtr mRendFac;
	GuiManagerPtr mGuiMng;

	std::vector<SceneNodePtr> mNodes;
	DefferedSignal mNodesSignal;

	mutable std::vector<scene::LightPtr> mLights;
	mutable std::vector<scene::CameraPtr> mCameras;
	mutable DefferedSlot mLightsSlot, mCamerasSlot;
#if MIR_GRAPHICS_DEBUG
	rend::Paint3DPtr mDebugPaint;
#endif
};

};