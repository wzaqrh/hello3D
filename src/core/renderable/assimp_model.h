#pragma once
#include "core/mir_export.h"
#include "core/base/launch.h"
#include "core/base/uniform_struct.h"
#include "core/resource/assimp_resource.h"
#include "core/renderable/renderable_base.h"

namespace mir {
namespace rend {

struct AiAnimeNode 
{
public:
	void Init(size_t serializeIndex, const res::AiNodePtr& node) {
		SerilizeIndex = serializeIndex;
		ChannelIndex = -1;
		LocalTransform = GlobalTransform = node->GetLocalTransform();
	}
public:
	size_t SerilizeIndex;
	int ChannelIndex;
	Eigen::Matrix4f LocalTransform, GlobalTransform;
};
struct AiAnimeTree 
{
public:
	void Init(const std::vector<res::AiNodePtr>& serializeNodes) {
		mNodeBySerializeIndex.assign(serializeNodes.begin(), serializeNodes.end());
		mAnimeNodes.resize(serializeNodes.size());
		for (size_t i = 0; i < serializeNodes.size(); ++i)
			mAnimeNodes[i].Init(i, serializeNodes[i]);
	}
	AiAnimeNode& GetNode(const res::AiNodePtr& node) const  {
		return mAnimeNodes[node->SerilizeIndex];
	}
	AiAnimeNode& GetParent(AiAnimeNode& anode) const  {
		size_t parentIndex = mNodeBySerializeIndex[anode.SerilizeIndex]->SerilizeIndex;
		return mAnimeNodes[parentIndex];
	}
	AiAnimeNode& GetChild(AiAnimeNode& anode, size_t index) const  {
		res::AiNodePtr child = mNodeBySerializeIndex[anode.SerilizeIndex]->Children[index];
		return mAnimeNodes[child->SerilizeIndex];
	}
	size_t GetChildCount(AiAnimeNode& node) const {
		return mNodeBySerializeIndex[node.SerilizeIndex]->ChildCount();
	}
	bool IsInited() const {
		return ! mNodeBySerializeIndex.empty();
	}
public:
	mutable std::vector<AiAnimeNode> mAnimeNodes;
	std::vector<res::AiNodePtr> mNodeBySerializeIndex;
};

class MIR_CORE_API AssimpModel : public RenderableSingleRenderOp 
{
	typedef RenderableSingleRenderOp Super;
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	AssimpModel(Launch launchMode, ResourceManager& resMng, const res::MaterialInstance& matInst) :Super(launchMode, resMng, matInst) {}
	
	CoTask<bool> LoadModel(std::string assetPath, std::string redirectResource = "");
	void PlayAnim(int Index);

	CoTask<void> UpdateFrame(float dt) override;
	void GenRenderOperation(RenderOperationQueue& opList) override;
private:
	const std::vector<Eigen::Matrix4f>& GetBoneMatrices(const res::AiNodePtr& node, const res::AssimpMeshPtr& mesh);
	void DoDraw(const res::AiNodePtr& node, RenderOperationQueue& opList);
	bool IsMaterialEnabled() const override { return false; }
private:
	res::AiScenePtr mAiScene;
	AiAnimeTree mAnimeTree;

	int mCurrentAnimIndex = -1;
	float mElapse = 0.0f;
	std::vector<Eigen::Matrix4f> mTempBoneMatrices;
};

}
}