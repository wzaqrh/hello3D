#pragma once
#include "core/mir_export.h"
#include "core/base/launch.h"
#include "core/resource/assimp_resource.h"
#include "core/renderable/renderable.h"

namespace mir {

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
private:
	const std::vector<aiMatrix4x4>& GetBoneMatrices(const aiNode* pNode, size_t pMeshIndex);
	void DoDraw(const aiNode* node, RenderOperationQueue& opList);
private:
	std::map<const aiNode*, AiNodeInfo> mNodeInfos;
	std::shared_ptr<AiScene> mAiScene;
	int mCurrentAnimIndex = -1;

	std::vector<aiMatrix4x4> mTransforms;
	float mElapse = 0.0f;
private:
	ResourceManager& mResourceMng;
	Launch mLaunchMode;
	MaterialPtr mMaterial;
	TransformPtr mTransform;
};

}