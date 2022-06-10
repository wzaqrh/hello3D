#include "core/base/debug.h"
#include "core/scene/scene_node.h"
#include "core/scene/transform.h"
#include "core/scene/light.h"
#include "core/scene/camera.h"
#include "core/renderable/renderable.h"

namespace mir {

#define SAFE_CALL(V, FUNC) if (V) V->FUNC

SceneNode::SceneNode()
{
	DEBUG_MEM_ALLOC_TAG(scene_node);
}

SceneNode::~SceneNode()
{
	DEBUG_MEM_DEALLOC_TAG(scene_node);
}

void SceneNode::SetTransform(const TransformPtr& transform)
{
	BOOST_ASSERT(transform);
	SAFE_CALL(mTransform, DettachSceneNode());
	mTransform = CreateInstance<Transform>();
	SAFE_CALL(mTransform, AttachSceneNode(shared_from_this()));

	FireSetComponenteEvent();
}

void SceneNode::SetRenderable(const RenderablePtr& rend)
{
	SAFE_CALL(mRenderable, DettachSceneNode());
	mRenderable = rend;
	SAFE_CALL(mRenderable, AttachSceneNode(shared_from_this()));

	FireSetComponenteEvent();
}
void SceneNode::SetLight(const scene::LightPtr& light)
{
	SAFE_CALL(mLight, DettachSceneNode());
	mLight = light;
	SAFE_CALL(mLight, AttachSceneNode(shared_from_this()));

	FireSetComponenteEvent();
}
void SceneNode::SetCamera(const scene::CameraPtr& camera)
{
	SAFE_CALL(mCamera, DettachSceneNode());
	mCamera = camera;
	SAFE_CALL(mCamera, AttachSceneNode(shared_from_this()));

	FireSetComponenteEvent();
}

const Eigen::AlignedBox3f SceneNode::GetAABB() const
{
	return mRenderable ? mRenderable->GetWorldAABB() : Eigen::AlignedBox3f();
}

void SceneNode::FireSetComponenteEvent()
{
	if (mTransform) mTransform->PostSetComponent();
	if (mRenderable) mRenderable->PostSetComponent();
	if (mLight) mLight->PostSetComponent();
	if (mCamera) mCamera->PostSetComponent();
}

/********** SceneNodeFactory **********/
SceneNodePtr SceneNodeFactory::CreateNode()
{
	SceneNodePtr node = CreateInstance<SceneNode>();
	node->SetTransform(CreateInstance<Transform>());
	return node;
}

}