#include "core/base/debug.h"
#include "core/scene/camera.h"
#include "core/scene/light.h"
#include "core/scene/transform.h"
#include "core/scene/scene_manager.h"
#include "core/resource/resource_manager.h"
#include "core/renderable/paint3d.h"
#include "core/renderable/skybox.h"
#include "core/renderable/post_process.h"
#include "core/renderable/renderable_factory.h"
#include "core/rendersys/render_pipeline.h"

using namespace mir::scene;

namespace mir {

SceneManager::SceneManager(ResourceManager& resMng, RenderableFactoryPtr rendFac, const Configure& cfg)
	: mResMng(resMng)
	, mRendFac(rendFac)
{
	mLightFac = CreateInstance<LightFactory>();
	mCameraFac = CreateInstance<CameraFactory>(resMng);
	mCameraFac->SetReverseZ(cfg.IsReverseZ());
	mCameraFac->SetAspect(1.0f * mResMng.WinWidth() / mResMng.WinHeight());
	mNodeFac = CreateInstance<SceneNodeFactory>();

	mNodesSignal.Connect(mCamerasSlot);
	mNodesSignal.Connect(mLightsSlot);
}

void SceneManager::SetPixelPerUnit(float ppu)
{
	mCameraFac->SetPixelPerUnit(ppu);
}

SceneNodePtr SceneManager::AddNode()
{
	SceneNodePtr node = mNodeFac->CreateNode();
	node->SetTransform(CreateInstance<Transform>());
	
	mNodes.push_back(node);
	mNodesSignal();

	return node;
}

RenderablePtr SceneManager::AddRendAsNode(RenderablePtr rend)
{
	BOOST_ASSERT(rend);
	auto node = AddNode();
	node->SetRenderable(rend);
	return rend;
}

CoTask<void> SceneManager::UpdateFrame(float dt)
{
	Eigen::AlignedBox3f aabb = this->GetWorldAABB();
	for (auto& node : mNodes) {
		if (RenderablePtr rend = node->GetRenderable()) 
			CoAwait rend->UpdateFrame(dt);

		if (CameraPtr camera = node->GetCamera())
			CoAwait camera->UpdateFrame(dt);

		if (scene::LightPtr light = node->GetLight())
			light->UpdateLightCamera(aabb);
	}

#if MIR_GRAPHICS_DEBUG
	if (mDebugPaint == nullptr) 
		mDebugPaint = CoAwait mRendFac->CreatePaint3DT();
	COROUTINE_VARIABLES;
	mDebugPaint->SetColor(0xFF00FF00);
	mDebugPaint->Clear();
	mDebugPaint->DrawAABBEdge(aabb);
#endif
}

void SceneManager::GetRenderables(RenderableCollection& rends)
{
	for (auto& node : mNodes) {
		if (RenderablePtr rend = node->GetComponent<Renderable>()) {
			rends.AddRenderable(rend);
		}
	}
#if MIR_GRAPHICS_DEBUG
	if (mDebugPaint) rends.AddRenderable(mDebugPaint);
#endif
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

const std::vector<mir::scene::CameraPtr>& SceneManager::GetCameras() const
{
	if (mCamerasSlot.AcquireSignal()) {
		mCameras.clear();
		for (auto& node : mNodes) {
			if (auto camera = node->GetCamera())
				mCameras.push_back(camera);
		}

		struct CompCameraByDepth {
			bool operator()(const CameraPtr& l, const CameraPtr& r) const {
				return l->GetDepth() < r->GetDepth();
			}
		};
		std::stable_sort(mCameras.begin(), mCameras.end(), CompCameraByDepth());
	}
	return mCameras;
}

const std::vector<mir::scene::LightPtr>& SceneManager::GetLights() const
{
	if (mLightsSlot.AcquireSignal()) {
		mLights.clear();
		for (auto& node : mNodes) {
			if (auto light = node->GetLight())
				mLights.push_back(light);
		}

		struct CompLightByType {
			bool operator()(const LightPtr& l, const LightPtr& r) const {
				return l->GetType() < r->GetType();
			}
		};
		std::stable_sort(mLights.begin(), mLights.end(), CompLightByType());
	}
	return mLights;
}

}