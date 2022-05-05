#include "core/scene/scene_manager.h"
#include "core/scene/camera.h"
#include "core/scene/light.h"
#include "core/scene/transform.h"
#include "core/resource/resource_manager.h"
#include "core/renderable/skybox.h"
#include "core/renderable/post_process.h"
#include "core/rendersys/render_pipeline.h"

using namespace mir::scene;

namespace mir {

SceneManager::SceneManager(ResourceManager& resMng, RenderableFactoryPtr rendFac)
	: mResMng(resMng)
	, mRendFac(rendFac)
	, mCamerasDirty(false)
	, mLightsDirty(false)
{
	mLightFac = CreateInstance<LightFactory>();
	mCameraFac = CreateInstance<CameraFactory>(resMng);
	mNodeFac = CreateInstance<SceneNodeFactory>();
}

void SceneManager::SetPixelPerUnit(float ppu)
{
	mCameraFac->SetPixelPerUnit(ppu);
}

SceneNodePtr SceneManager::AddNode()
{
	SceneNodePtr node = mNodeFac->CreateNode();
	mNodes.push_back(node);
	return node;
}

scene::CameraPtr SceneManager::CreateAddCameraNode(scene::CameraPtr camera)
{
	BOOST_ASSERT(camera);
	auto node = AddNode();
	node->SetCamera(camera);

	mCameras.push_back(camera);
	mCamerasDirty = true;
	return camera;
}
void SceneManager::RemoveAllCameras()
{
	mCameras.clear();
}
void SceneManager::ResortCameras() 
{
	if (mCamerasDirty) {
		mCamerasDirty = false;

		struct CompCameraByDepth {
			bool operator()(const CameraPtr& l, const CameraPtr& r) const {
				return l->GetDepth() < r->GetDepth();
			}
		};
		std::stable_sort(mCameras.begin(), mCameras.end(), CompCameraByDepth());
	}
}

scene::LightPtr SceneManager::AddLightNode(scene::LightPtr light)
{
	BOOST_ASSERT(light);
	auto node = AddNode();
	node->SetLight(light);

	mLights.push_back(light);
	mLightsDirty = true;
	return light;
}
void SceneManager::RemoveAllLights()
{
	mLights.clear();
}
void SceneManager::ResortLights() 
{
	if (mLightsDirty) {
		mLightsDirty = false;

		struct CompLightByType {
			bool operator()(const LightPtr& l, const LightPtr& r) const {
				return l->GetType() < r->GetType();
			}
		};
		std::sort(mLights.begin(), mLights.end(), CompLightByType());
	}
}

RenderablePtr SceneManager::AddRendNode(RenderablePtr rend)
{
	BOOST_ASSERT(rend);
	auto node = AddNode();
	node->SetRenderable(rend);

	mRends.push_back(rend);
	mRendsDirty = true;
	return rend;
}

CoTask<void> SceneManager::UpdateFrame(float dt)
{
	for (auto& node : mNodes) {
		if (RenderablePtr rend = node->GetComponent<Renderable>()) 
			CoAwait rend->UpdateFrame(dt);
		if (CameraPtr camera = node->GetComponent<Camera>())
			CoAwait camera->UpdateFrame(dt);
	}

	Eigen::AlignedBox3f aabb = this->GetWorldAABB();
	for (auto& light : mLights)
		light->UpdateLightCamera(aabb);
}

void SceneManager::GenRenderOperation(RenderOperationQueue& opQue)
{
	for (auto& node : mNodes) {
		if (RenderablePtr rend = node->GetComponent<Renderable>())
			rend->GenRenderOperation(opQue);
	}
}

Eigen::AlignedBox3f SceneManager::GetWorldAABB() const
{
	Eigen::AlignedBox3f aabb;
	for (size_t i = 0; i < mNodes.size(); ++i) {
		auto node = mNodes[i];
		aabb.extend(node->GetAABB());
	}
	return aabb;
}

}