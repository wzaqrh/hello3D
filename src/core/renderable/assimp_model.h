#pragma once
#include <assimp/cimport.h>
#include <assimp/Importer.hpp>
#include <assimp/ai_assert.h>
#include <assimp/cfileio.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/IOSystem.hpp>
#include <assimp/IOStream.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/DefaultLogger.hpp>
#include "core/mir_export.h"
#include "core/base/launch.h"
#include "core/renderable/renderable.h"
#include "core/renderable/assimp_mesh.h"

namespace mir {

struct AiNodeInfo {
	aiMatrix4x4 mLocalTransform;
	aiMatrix4x4 mGlobalTransform;
	AssimpMeshPtrVector meshes;
	int channelIndex;
public:
	AiNodeInfo() {
		channelIndex = -1;
	}
	AssimpMeshPtr operator[](int pos) {
		return meshes[pos];
	}
	size_t MeshCount() const {
		return meshes.size();
	}
	void AddMesh(AssimpMeshPtr mesh) {
		meshes.push_back(mesh);
	}
};

struct AiScene {
	AssimpMeshPtrVector mMeshes;
	std::map<std::string, const aiNode*> mBoneNodesByName;
	std::map<const aiNode*, AiNodeInfo> mNodeInfos;
	std::map<std::string, ITexturePtr> mLoadedTexture;
	aiNode* mRootNode = nullptr;
	const aiScene* mScene = nullptr;
};

class MIR_CORE_API AssimpModel : public IRenderable 
{
	friend class RenderableFactory;
	DECLARE_STATIC_CREATE_CONSTRUCTOR(AssimpModel);
	AssimpModel(Launch launchMode, ResourceManager& resourceMng, TransformPtr pMove, const std::string& matType);
public:
	~AssimpModel();
	void LoadModel(const std::string& imgPath, const std::string& redirectResource = "");
	void PlayAnim(int Index);

	void Update(float dt);
	int GenRenderOperation(RenderOperationQueue& opList) override;
private:
	const std::vector<aiMatrix4x4>& GetBoneMatrices(const aiNode* pNode, size_t pMeshIndex);
	void DoDraw(aiNode* node, RenderOperationQueue& opList);
	void processNode(aiNode * node, const aiScene * scene);
	AssimpMeshPtr processMesh(aiMesh * mesh, const aiScene * scene);
	std::vector<ITexturePtr> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const aiScene* scene);
private:
	AssimpMeshPtrVector mMeshes;
	std::map<std::string, const aiNode*> mBoneNodesByName;
	std::map<const aiNode*, AiNodeInfo> mNodeInfos;
	std::map<std::string, ITexturePtr> mLoadedTexture;
	aiNode* mRootNode = nullptr;
	const aiScene* mScene = nullptr;
	std::shared_ptr<AiScene> mAiScene;
	int mCurrentAnimIndex = -1;
private:
	std::vector<aiMatrix4x4> mTransforms;
	float mElapse = 0.0f;
	Assimp::Importer* mImporter = nullptr;
	std::string mRedirectResourceDir, mRedirectResourceExt;
	ResourceManager& mResourceMng;
	Launch mLaunchMode;
	MaterialPtr mMaterial;
	TransformPtr mTransform;
};

}