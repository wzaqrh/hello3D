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
	template<typename T> std::shared_ptr<T> GetComponent() const { 
		auto node = mNode.lock();
		return node ? node->GetComponent<T>() : nullptr; 
	}
	template<typename T> void SetComponent(const std::shared_ptr<T>& component) {
		if (auto node = mNode.lock()) 
			node->SetComponent<T>(component);
	}
	SceneNodePtr GetNode() { return mNode.lock(); }
private:
	void AttachSceneNode(const SceneNodePtr& node) {
		mNode = node;
	}
private:
	SceneNodeWeakPtr mNode;
};

}