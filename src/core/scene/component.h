#pragma once
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#include "core/predeclare.h"
#include "core/scene/scene_node.h"

namespace mir {

class MIR_CORE_API Component : boost::noncopyable {
	friend class SceneNode;
public:
	~Component() {}

	template<typename T> void SetComponent(const std::shared_ptr<T>& component) {
		if (auto node = mNode.lock()) 
			node->SetComponent<T>(component);
	}

	template<typename T> std::shared_ptr<T> GetComponent() const {
		auto node = mNode.lock();
		return node ? node->GetComponent<T>() : nullptr;
	}
	TransformPtr GetTransform() const { return GetComponent<Transform>(); }
	RenderablePtr GetRenderable() const { return GetComponent<Renderable>(); }
	scene::LightPtr GetLight() const { return GetComponent<scene::Light>(); }
	scene::CameraPtr GetCamera() const { return GetComponent<scene::Camera>(); }

	SceneNodePtr GetNode() { return mNode.lock(); }
private:
	void AttachSceneNode(const SceneNodePtr& node) { 
		mNode = node; 
		PostAttachSceneNode();
	}
	void DettachSceneNode() {
		PreDettachSceneNode();
		mNode.reset();
	}

	virtual void PostAttachSceneNode() {}
	virtual void PreDettachSceneNode() {}
	virtual void PostSetComponent() {}
private:
	SceneNodeWeakPtr mNode;
};

}