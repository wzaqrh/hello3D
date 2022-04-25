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
	BOOST_ASSERT(transform);
	mTransform = CreateInstance<Transform>();
	mTransform->AttachSceneNode(shared_from_this());
}

void SceneNode::SetRenderable(const RenderablePtr& rend)
{
	mRenderable = rend;
	if (mRenderable) 
		mRenderable->AttachSceneNode(shared_from_this());
}
void SceneNode::SetLight(const scene::LightPtr& light)
{
	mLight = light;
	if (mLight) 
		mLight->AttachSceneNode(shared_from_this());
}
void SceneNode::SetCamera(const scene::CameraPtr& camera)
{
	mCamera = camera;
	if (mCamera) 
		mCamera->AttachSceneNode(shared_from_this());
}

const Eigen::AlignedBox3f SceneNode::GetAABB() const
{
	return mRenderable ? mRenderable->GetWorldAABB() : Eigen::AlignedBox3f();
}

SceneNodePtr SceneNodeFactory::CreateNode()
{
	SceneNodePtr node = CreateInstance<SceneNode>();
	node->SetTransform(CreateInstance<Transform>());
	return node;
}

}