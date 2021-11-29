#pragma once
#include "core/mir_export.h"
#include "core/base/launch.h"
#include "core/resource/assimp_resource.h"
#include "core/renderable/renderable.h"

namespace mir {

constexpr int MAX_MATRICES = 56;
struct cbWeightedSkin
{
	Eigen::Matrix4f Model;
	Eigen::Matrix4f Models[MAX_MATRICES];
	unsigned int hasNormal;
	unsigned int hasMetalness;
	unsigned int hasRoughness;
	unsigned int hasAO;
};

struct cbUnityMaterial
{
	cbUnityMaterial() {
		_SpecColor = Eigen::Vector4f(1, 1, 1, 1);
		_Color = Eigen::Vector4f(1, 1, 1, 1);
		_GlossMapScale = 1;
		_OcclusionStrength = 1;
		_SpecLightOff = 0;
	}
	Eigen::Vector4f _SpecColor;
	Eigen::Vector4f _Color;
	float _GlossMapScale;
	float _OcclusionStrength;
	unsigned int _SpecLightOff;
};

struct cbUnityGlobal
{
	cbUnityGlobal() {
		_Unity_IndirectSpecColor = Eigen::Vector4f(0, 0, 0, 0);
		_AmbientOrLightmapUV = Eigen::Vector4f(0.01, 0.01, 0.01, 1);
		_Unity_SpecCube0_HDR = Eigen::Vector4f(0.5, 1, 0, 0);
	}
	Eigen::Vector4f _Unity_IndirectSpecColor;
	Eigen::Vector4f _AmbientOrLightmapUV;
	Eigen::Vector4f _Unity_SpecCube0_HDR;
};

struct AiAnimeNode {
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
struct AiAnimeTree {
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

class MIR_CORE_API AssimpModel : public IRenderable 
{
	friend class RenderableFactory;
	DECLARE_STATIC_CREATE_CONSTRUCTOR(AssimpModel);
	AssimpModel(Launch launchMode, ResourceManager& resourceMng, TransformPtr pMove, const std::string& matType);
public:
	~AssimpModel();
	void LoadModel(const std::string& assetPath, const std::string& redirectResource = "");
	void PlayAnim(int Index);
	
	void Update(float dt);
	int GenRenderOperation(RenderOperationQueue& opList) override;

	const TransformPtr& GetTransform() const { return mTransform; }
private:
	const std::vector<aiMatrix4x4>& GetBoneMatrices(const AiNodePtr& node, size_t meshIndexIndex);
	void DoDraw(const AiNodePtr& node, RenderOperationQueue& opList);
private:
	const Launch mLaunchMode;
	ResourceManager& mResourceMng;
	MaterialPtr mMaterial;
	TransformPtr mTransform;
	AiScenePtr mAiScene;
	AiAnimeTree mAnimeTree;
	std::function<void()> mInitAnimeTreeTask, mPlayAnimTask;

	int mCurrentAnimIndex = -1;
	float mElapse = 0.0f;
	std::vector<aiMatrix4x4> mTempBoneMatrices;
};

}