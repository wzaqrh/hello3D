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
#include "core/rendersys/predeclare.h"
#include "core/resource/resource.h"
#include "core/resource/assimp_mesh.h"

namespace mir {

struct AiNodeInfo {
	aiMatrix4x4 mLocalTransform;
	aiMatrix4x4 mGlobalTransform;
	std::vector<AssimpMeshPtr> meshes;
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

class AiScene : public ImplementResource<IResource> 
{
public:
	Assimp::Importer* mImporter = nullptr;
	const aiNode* mRootNode = nullptr;
	const aiScene* mScene = nullptr;
	std::vector<AssimpMeshPtr> mMeshes;
	std::map<std::string, const aiNode*> mBoneNodesByName;
	std::map<std::string, ITexturePtr> mLoadedTexture;
	std::map<const aiNode*, AiNodeInfo> mNodeInfos;
};

class AiResourceFactory {
public:
	AiScenePtr CreateAiScene(Launch launchMode, ResourceManager& resourceMng,
		const Material& material, const std::string& assetPath, const std::string& redirectRes);
};

}