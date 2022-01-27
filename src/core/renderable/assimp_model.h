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
	void Init(size_t serializeIndex, const res::AiNodePtr& node) {
		SerilizeIndex = serializeIndex;
		ChannelIndex = -1;
		LocalTransform = GlobalTransform = node->RawNode->mTransformation;
	}
public:
	size_t SerilizeIndex;
	int ChannelIndex;
	aiMatrix4x4 LocalTransform;
	aiMatrix4x4 GlobalTransform;
};
struct AiAnimeTree 
{
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
	INHERIT_RENDERABLE_SINGLE_OP_CONSTRUCTOR(AssimpModel);
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	cppcoro::shared_task<bool> LoadModel(const std::string& assetPath, const std::string& redirectResource = "");
	void PlayAnim(int Index);

	void Update(float dt);
	void GenRenderOperation(RenderOperationQueue& opList) override;
private:
	const std::vector<aiMatrix4x4>& GetBoneMatrices(const res::AiNodePtr& node, size_t meshIndexIndex);
	void DoDraw(const res::AiNodePtr& node, RenderOperationQueue& opList);
private:
	res::AiScenePtr mAiScene;
	AiAnimeTree mAnimeTree;
	std::function<void()> mInitAnimeTreeTask, mPlayAnimTask;

	int mCurrentAnimIndex = -1;
	float mElapse = 0.0f;
	std::vector<aiMatrix4x4> mTempBoneMatrices;
};

}
}