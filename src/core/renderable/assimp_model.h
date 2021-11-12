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

class AssimpModel : public IRenderable {
public:
	AssimpModel(IRenderSystem& renderSys, MaterialFactory& matFac, TransformPtr pMove, const std::string& matType);
	~AssimpModel();
public:
	void LoadModel(const std::string& imgPath);
	void PlayAnim(int Index);

	void Update(float dt);
	virtual int GenRenderOperation(RenderOperationQueue& opList) override;
private:
	const std::vector<aiMatrix4x4>& GetBoneMatrices(const aiNode* pNode, size_t pMeshIndex);
	void DoDraw(aiNode* node, RenderOperationQueue& opList);
	void processNode(aiNode * node, const aiScene * scene);
	AssimpMeshPtr processMesh(aiMesh * mesh, const aiScene * scene);
	std::vector<ITexturePtr> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const aiScene* scene);
private:
	MaterialPtr mMaterial;
	TransformPtr mTransform;
public:
	AssimpMeshPtrVector mMeshes;
	std::map<std::string, const aiNode*> mBoneNodesByName;
	std::map<const aiNode*, AiNodeInfo> mNodeInfos;
	std::map<std::string, ITexturePtr> mLoadedTexture;
	aiNode* mRootNode = nullptr;
	const aiScene* mScene = nullptr;
	int mCurrentAnimIndex = -1;
private:
	std::vector<aiMatrix4x4> mTransforms;
	float mElapse = 0.0f;
	Assimp::Importer* mImporter = nullptr;
	IRenderSystem& mRenderSys;
};

}