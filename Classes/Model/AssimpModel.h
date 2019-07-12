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

#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <assert.h>

#include "TMesh.h"

XMMATRIX ToXM(const aiMatrix4x4& m);
aiMatrix4x4 FromXM(const XMMATRIX& m);

struct AiNodeInfo {
	aiMatrix4x4 mLocalTransform;
	aiMatrix4x4 mGlobalTransform;
	TMeshSharedPtrVector meshes;
	int channelIndex;
public:
	AiNodeInfo();
	TMeshSharedPtr operator[](int pos);
	int size() const;
	void push_back(TMeshSharedPtr mesh);
};

class TRenderSystem;
class AssimpModel
{
public:
	AssimpModel(TRenderSystem* RenderSys);
	~AssimpModel();
public:
	void LoadModel(const std::string& imgPath);
	void Update(float dt);
	void PlayAnim(int Index);
	const std::vector<aiMatrix4x4>& GetBoneMatrices(const aiNode* pNode, size_t pMeshIndex);
private:
	void processNode(aiNode * node, const aiScene * scene);
	TMeshSharedPtr processMesh(aiMesh * mesh, const aiScene * scene);
	std::vector<TextureInfo> loadMaterialTextures(aiMaterial* mat, 
		aiTextureType type, std::string typeName, const aiScene* scene);
public:
	TMeshSharedPtrVector mMeshes;
	std::map<std::string, const aiNode*> mBoneNodesByName;
	std::map<const aiNode*, AiNodeInfo> mNodeInfos;
	std::map<std::string, TextureInfo> mLoadedTexture;
	aiNode* mRootNode = nullptr;
	const aiScene* mScene = nullptr;
	int mCurrentAnimIndex = -1;
private:
	std::vector<aiMatrix4x4> mTransforms;
	float mElapse = 0.0f;
	Assimp::Importer* mImporter = nullptr;
	TRenderSystem* mRenderSys;
};

