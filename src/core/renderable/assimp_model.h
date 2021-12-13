#pragma once
#include "core/mir_export.h"
#include "core/base/launch.h"
#include "core/base/uniform_struct.h"
#include "core/resource/assimp_resource.h"
#include "core/renderable/renderable_base.h"

namespace mir {

struct AiAnimeNode 
{
	void Init(size_t serializeIndex, const AiNodePtr& node) {
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
	void Init(const std::vector<AiNodePtr>& serializeNodes) {
		mNodeBySerializeIndex.assign(serializeNodes.begin(), serializeNodes.end());
		mAnimeNodes.resize(serializeNodes.size());
		for (size_t i = 0; i < serializeNodes.size(); ++i)
			mAnimeNodes[i].Init(i, serializeNodes[i]);
	}
	AiAnimeNode& GetNode(const AiNodePtr& node) const  {
		return mAnimeNodes[node->SerilizeIndex];
	}
	AiAnimeNode& GetParent(AiAnimeNode& anode) const  {
		size_t parentIndex = mNodeBySerializeIndex[anode.SerilizeIndex]->SerilizeIndex;
		return mAnimeNodes[parentIndex];
	}
	AiAnimeNode& GetChild(AiAnimeNode& anode, size_t index) const  {
		AiNodePtr child = mNodeBySerializeIndex[anode.SerilizeIndex]->Children[index];
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
	std::vector<AiNodePtr> mNodeBySerializeIndex;
};

class MIR_CORE_API AssimpModel : public RenderableSingleRenderOp 
{
	typedef RenderableSingleRenderOp Super;
	friend class RenderableFactory;
	DECLARE_STATIC_CREATE_CONSTRUCTOR(AssimpModel);
	AssimpModel(Launch launchMode, ResourceManager& resourceMng, const MaterialLoadParam& matType);
public:
	void LoadModel(const std::string& assetPath, const std::string& redirectResource = "");
	void PlayAnim(int Index);

	void Update(float dt);
	void GenRenderOperation(RenderOperationQueue& opList) override;
private:
	const std::vector<aiMatrix4x4>& GetBoneMatrices(const AiNodePtr& node, size_t meshIndexIndex);
	void DoDraw(const AiNodePtr& node, RenderOperationQueue& opList);
private:
	AiScenePtr mAiScene;
	AiAnimeTree mAnimeTree;
	std::function<void()> mInitAnimeTreeTask, mPlayAnimTask;

	int mCurrentAnimIndex = -1;
	float mElapse = 0.0f;
	std::vector<aiMatrix4x4> mTempBoneMatrices;
};

}