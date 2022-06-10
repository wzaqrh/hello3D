#pragma once
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#include "core/predeclare.h"
#include "core/base/math.h"

namespace mir {

class MIR_CORE_API SceneNode : public std::enable_shared_from_this<SceneNode>, boost::noncopyable {
public:
	SceneNode();
	~SceneNode();
	void SetTransform(const TransformPtr& transform);
	void SetRenderable(const RenderablePtr& rend);
	void SetLight(const scene::LightPtr& light);
	void SetCamera(const scene::CameraPtr& camera);

	template<typename T> void SetComponent(const std::shared_ptr<T>& component) {}
	template<> void SetComponent<Transform>(const TransformPtr& component) { SetTransform(component); }
	template<> void SetComponent<Renderable>(const RenderablePtr& component) { SetRenderable(component); }
	template<> void SetComponent<scene::Light>(const scene::LightPtr& component) { SetLight(component); }
	template<> void SetComponent<scene::Camera>(const scene::CameraPtr& component) { SetCamera(component); }
public:
	const TransformPtr& GetTransform() const { return mTransform; }
	const RenderablePtr& GetRenderable() const { return mRenderable; }
	const scene::LightPtr& GetLight() const { return mLight; }
	const scene::CameraPtr& GetCamera() const { return mCamera; }
	const Eigen::AlignedBox3f GetAABB() const;

	template<typename T> const std::shared_ptr<T>& GetComponent() const {}
	template<> const TransformPtr& GetComponent<Transform>() const { return mTransform; }
	template<> const RenderablePtr& GetComponent<Renderable>() const { return mRenderable; }
	template<> const scene::LightPtr& GetComponent<scene::Light>() const { return mLight; }
	template<> const scene::CameraPtr& GetComponent<scene::Camera>() const { return mCamera; }
private:
	void FireSetComponenteEvent();
private:
	TransformPtr mTransform;
	RenderablePtr mRenderable;
	scene::LightPtr mLight;
	scene::CameraPtr mCamera;
};

class SceneNodeFactory : boost::noncopyable {
public:
	SceneNodePtr CreateNode();
};

}