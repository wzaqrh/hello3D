#include "core/scene/scene_node.h"
#include "core/scene/transform.h"
#include "core/renderable/renderable.h"
#include "core/scene/light.h"
#include "core/scene/camera.h"

namespace mir {

SceneNode::SceneNode()
{}

void SceneNode::SetTransform(const TransformPtr& transform)
{
	mTransform = CreateInstance<Transform>();
	mTransform->AttachSceneNode(shared_from_this());
}

void SceneNode::SetRenderable(const RenderablePtr& rend)
{
	mRenderable = rend;
	mRenderable->AttachSceneNode(shared_from_this());
}
void SceneNode::SetLight(const scene::LightPtr& light)
{
	mLight = light;
	mLight->AttachSceneNode(shared_from_this());
}
void SceneNode::SetCamera(const scene::CameraPtr& camera)
{
	mCamera = camera;
	mCamera->AttachSceneNode(shared_from_this());
}

SceneNodePtr SceneNodeFactory::CreateNode()
{
	SceneNodePtr node = CreateInstance<SceneNode>();
	node->SetTransform(CreateInstance<Transform>());
	return node;
}

}