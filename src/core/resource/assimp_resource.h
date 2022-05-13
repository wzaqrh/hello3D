#pragma once
#include "core/base/cppcoro.h"
#include "core/rendersys/predeclare.h"
#include "core/resource/predeclare.h"
#include "core/resource/resource.h"
#include "core/resource/assimp_mesh.h"

struct aiAnimation;

namespace mir {
namespace res {

struct AiNode : public std::enable_shared_from_this<AiNode>
{
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	AiNode(const size_t serializeIndex) :SerilizeIndex(serializeIndex) {}
	void AddMesh(const AssimpMeshPtr& mesh) {
		Meshes.push_back(mesh);

		Transform3fAffine t(mGlobalTransform);
		mAABB.extend(mesh->GetAABB().transformed(t));
	}
	void AddChild(const AiNodePtr& child) {
		Children.push_back(child);
		child->Parent = this->shared_from_this();

		child->mGlobalTransform = mGlobalTransform * child->GetLocalTransform();
		mAABB.extend(child->UpdateAABB());
	}

	const Eigen::AlignedBox3f& UpdateAABB() {
		mAABB = Eigen::AlignedBox3f();
		
		Transform3fAffine t(mGlobalTransform);
		for (auto& mesh : Meshes) 
			mAABB.extend(mesh->GetAABB().transformed(t));

		for (auto& child : Children) {
			child->mGlobalTransform = mGlobalTransform * child->GetLocalTransform();
			mAABB.extend(child->UpdateAABB());
		}
		return mAABB;
	}
public:
	const Eigen::AlignedBox3f& GetAABB() const { return mAABB; }
	const Eigen::Matrix4f& GetLocalTransform() const { return mLocalTransform; }

	const AiNodePtr& GetChild(size_t pos) const { return Children[pos]; }
	size_t ChildCount() const { return Children.size(); }
	const std::vector<AiNodePtr>& GetChildren() const { return Children; }
	
	const AssimpMeshPtr& operator[](size_t index) const { return Meshes[index]; }
	size_t MeshCount() const { return Meshes.size(); }
	std::vector<AssimpMeshPtr>& GetMeshes() { return Meshes; }
public:
	const size_t SerilizeIndex;
	std::string mName;
	std::weak_ptr<AiNode> Parent;
	std::vector<AiNodePtr> Children;
	std::vector<AssimpMeshPtr> Meshes;
	Eigen::Matrix4f mLocalTransform, mGlobalTransform;
	Eigen::AlignedBox3f mAABB;
};

struct AiScene : public ImplementResource<IResource>
{
	friend class AiSceneLoader;
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	TemplateArgs AiNodePtr AddNode(T &&...args) {
		AiNodePtr newNode = std::make_shared<AiNode>(std::forward<T>(args)..., mNodes.size());
		mNodes.push_back(newNode);
		return newNode;
	}
	AssimpMeshPtr AddMesh() {
		AssimpMeshPtr mesh = std::make_shared<AssimpMesh>();
		mMeshes.push_back(mesh);
		return mesh;
	}
	const std::vector<AiNodePtr>& GetNodes() const { return mNodes; }
	AiNodePtr FindNodeByName(const std::string& name) const {
		auto find_iter = std::find_if(mNodes.begin(), mNodes.end(), [&name](const AiNodePtr& nnode) { 
			return nnode->mName == name;
		});
		return (find_iter != mNodes.end()) ? *find_iter : nullptr;
	}
	const std::vector<AssimpMeshPtr>& GetMeshes() const { return mMeshes; }
	const Eigen::AlignedBox3f& GetAABB() const { return mRootNode->GetAABB(); }
public:
	AiNodePtr mRootNode;
	std::vector<const aiAnimation*> mAnimations;
	std::vector<AssimpMeshPtr> mMeshes;
	std::vector<AiNodePtr> mNodes;
};

class AiResourceFactory {
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	CoTask<bool> CreateAiScene(Launch launchMode, AiScenePtr& aiScene, ResourceManager& resourceMng, std::string assetPath, std::string redirectRes) ThreadSafe;
};

}
}