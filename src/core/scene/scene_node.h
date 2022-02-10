#pragma once
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#include "core/predeclare.h"

namespace mir {

class MIR_CORE_API SceneNode : std::enable_shared_from_this<SceneNode>, boost::noncopyable {
public:
	SceneNode();
	void SetRenderable(const RenderablePtr& rend);
	void SetLight(const scene::LightPtr& light);
	void SetCamera(const scene::CameraPtr& camera);
public:
	const TransformPtr& GetTransform() const { return mTransform; }
	const RenderablePtr& GetRenderable() const { return mRenderable; }
	const scene::LightPtr& GetLight() const { return mLight; }
	const scene::CameraPtr& GetCamera() const { return mCamera; }
	
	template<typename T> const std::shared_ptr<T>& GetComponent() const {}
	template<> const TransformPtr& GetComponent<Transform>() const { return mTransform; }
	template<> const RenderablePtr& GetComponent<Renderable>() const { return mRenderable; }
	template<> const scene::LightPtr& GetComponent<scene::Light>() const { return mLight; }
	template<> const scene::CameraPtr& GetComponent<scene::Camera>() const { return mCamera; }
private:
	TransformPtr mTransform;
	RenderablePtr mRenderable;
	scene::LightPtr mLight;
	scene::CameraPtr mCamera;
};

}