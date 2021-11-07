#pragma once
//INCLUDE_PREDEFINE_H
#include "core/renderable/assimp_mesh.h"
#include "core/renderable/renderable.h"

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

typedef std::shared_ptr<struct IRenderSystem> IRenderSystemPtr;
typedef std::shared_ptr<struct Movable> MovablePtr;
class AssimpModel : public IRenderable {
public:
	AssimpModel(IRenderSystem& RenderSys, MovablePtr pMove, const std::string& matType);
	~AssimpModel();
public:
	void LoadModel(const std::string& imgPath);
	void PlayAnim(int Index);

	void Update(float dt);
	void Draw();
	virtual int GenRenderOperation(RenderOperationQueue& opList) override;
private:
	const std::vector<aiMatrix4x4>& GetBoneMatrices(const aiNode* pNode, size_t pMeshIndex);
	void DoDraw(aiNode* node, RenderOperationQueue& opList);
	void LoadMaterial(const std::string& matType);
	void processNode(aiNode * node, const aiScene * scene);
	AssimpMeshPtr processMesh(aiMesh * mesh, const aiScene * scene);
	std::vector<ITexturePtr> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const aiScene* scene);
private:
	MaterialPtr mMaterial;
	MovablePtr mMove;
public:
	AssimpMeshPtrVector mMeshes;
	std::map<std::string, const aiNode*> mBoneNodesByName;
	std::map<const aiNode*, AiNodeInfo> mNodeInfos;
	std::map<std::string, ITexturePtr> mLoadedTexture;
	aiNode* mRootNode = nullptr;
	const aiScene* mScene = nullptr;
	int mCurrentAnimIndex = -1;
	int mDrawCount;
private:
	std::vector<aiMatrix4x4> mTransforms;
	float mElapse = 0.0f;
	Assimp::Importer* mImporter = nullptr;
	IRenderSystem& mRenderSys;
};

}