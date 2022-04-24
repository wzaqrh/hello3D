#pragma once
#include <assimp/cimport.h>
#include <assimp/Importer.hpp>
#include <assimp/ai_assert.h>
#include <assimp/cfileio.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/pbrmaterial.h>
#include <assimp/IOSystem.hpp>
#include <assimp/IOStream.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/DefaultLogger.hpp>
#include "core/base/cppcoro.h"
#include "core/rendersys/predeclare.h"
#include "core/resource/resource.h"
#include "core/resource/assimp_mesh.h"

namespace mir {
namespace res {

struct AiNode : public std::enable_shared_from_this<AiNode>
{
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	AiNode(const aiNode* rawNode, const size_t serializeIndex): RawNode(rawNode), SerilizeIndex(serializeIndex) {}
	void AddChild(const AiNodePtr& child) {
		Children.push_back(child);
		child->Parent = this->shared_from_this();
		mAABB.extend(child->mAABB);
	}
	void AddMesh(const AssimpMeshPtr& mesh) {
		mAABB.extend(mesh->GetAABB());
		Meshes.push_back(mesh);
	}

	size_t ChildCount() const { return Children.size(); }
	const AssimpMeshPtr& operator[](size_t index) const { return Meshes[index]; }
	size_t MeshCount() const { return Meshes.size(); }
	const Eigen::AlignedBox3f& GetAABB() const { return mAABB; }
public:
	const size_t SerilizeIndex;
	const aiNode* RawNode;
	std::weak_ptr<AiNode> Parent;
	std::vector<AiNodePtr> Children;
	std::vector<AssimpMeshPtr> Meshes;
	Eigen::AlignedBox3f mAABB;
};

struct AiScene : public ImplementResource<IResource>
{
	friend class AiSceneLoader;
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	AiNodePtr AddNode(const aiNode* rawNode) {
		AiNodePtr newNode = std::make_shared<AiNode>(rawNode, mNodeBySerializeIndex.size());
		mNodeBySerializeIndex.push_back(newNode);
		return newNode;
	}
	const std::vector<AiNodePtr>& GetSerializeNodes() const { return mNodeBySerializeIndex; }
	std::vector<AiNodePtr>::const_iterator begin() const { return mNodeBySerializeIndex.begin(); }
	std::vector<AiNodePtr>::const_iterator end() const { return mNodeBySerializeIndex.end(); }
	const Eigen::AlignedBox3f& GetAABB() const { return mRootNode->GetAABB(); }
public:
	const aiScene* mScene = nullptr;
	AiNodePtr mRootNode;
	std::vector<AiNodePtr> mNodeBySerializeIndex;
	std::map<std::string, AiNodePtr> mBoneNodesByName;
	std::map<std::string, ITexturePtr> mLoadedTexture;
private:
	const Assimp::Importer* mImporter = nullptr;
};

class AiResourceFactory {
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	CoTask<bool> CreateAiScene(Launch launchMode, AiScenePtr& aiScene, ResourceManager& resourceMng, std::string assetPath, std::string redirectRes) ThreadSafe;
};

}
}