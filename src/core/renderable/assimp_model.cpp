#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/assert.hpp>
#include "core/renderable/assimp_model.h"
#include "core/resource/material.h"
#include "core/resource/material_cb.h"
#include "core/resource/material_factory.h"
#include "core/resource/resource_manager.h"
#include "core/rendersys/interface_type.h"
#include "core/base/transform.h"
#include "core/base/debug.h"

namespace mir {

#define AS_CONST_REF(TYPE, V) *(const TYPE*)(&V)

struct EvaluateTransforms
{
public:
	std::vector<aiMatrix4x4> mTransforms;
	const aiAnimation* mAnim;
public:
	EvaluateTransforms(const aiAnimation* Anim) :mAnim(Anim) {}
	void Update(float pTime) 
	{
		double ticksPerSecond = mAnim->mTicksPerSecond != 0.0 ? mAnim->mTicksPerSecond : 25.0;
		pTime *= ticksPerSecond;

		// map into anim's duration
		double time = 0.0f;
		if (mAnim->mDuration > 0.0) {
			time = fmod(pTime, mAnim->mDuration);
		}

		mTransforms.resize(mAnim->mNumChannels);

		for (unsigned int a = 0; a < mAnim->mNumChannels; ++a) {
			const aiNodeAnim* channel = mAnim->mChannels[a];

			/*std::string name = "IK_Auge_L";
			if (name == channel->mNodeName.C_Str()) {
			channel = channel;
			}*/

			// ******** Position *****
			aiVector3D presentPosition(0, 0, 0);
			if (channel->mNumPositionKeys > 0) {
				// Look for present frame number. Search from last position if time is after the last time, else from beginning
				// Should be much quicker than always looking from start for the average use case.
				unsigned int frame = 0;
				while (frame < channel->mNumPositionKeys - 1) {
					if (time < channel->mPositionKeys[frame + 1].mTime) {
						break;
					}
					++frame;
				}

				// interpolate between this frame's value and next frame's value
				unsigned int nextFrame = (frame + 1) % channel->mNumPositionKeys;
				const aiVectorKey& key = channel->mPositionKeys[frame];
				const aiVectorKey& nextKey = channel->mPositionKeys[nextFrame];
				double diffTime = nextKey.mTime - key.mTime;
				if (diffTime < 0.0) {
					diffTime += mAnim->mDuration;
				}
				if (diffTime > 0) {
					float factor = float((time - key.mTime) / diffTime);
					presentPosition = key.mValue + (nextKey.mValue - key.mValue) * factor;
				}
				else {
					presentPosition = key.mValue;
				}
			}

			// ******** Rotation *********
			aiQuaternion presentRotation(1, 0, 0, 0);
			if (channel->mNumRotationKeys > 0) {
				unsigned int frame = 0;
				while (frame < channel->mNumRotationKeys - 1) {
					if (time < channel->mRotationKeys[frame + 1].mTime) {
						break;
					}
					++frame;
				}

				// interpolate between this frame's value and next frame's value
				unsigned int nextFrame = (frame + 1) % channel->mNumRotationKeys;
				const aiQuatKey& key = channel->mRotationKeys[frame];
				const aiQuatKey& nextKey = channel->mRotationKeys[nextFrame];
				double diffTime = nextKey.mTime - key.mTime;
				if (diffTime < 0.0) {
					diffTime += mAnim->mDuration;
				}
				if (diffTime > 0) {
					float factor = float((time - key.mTime) / diffTime);
					aiQuaternion::Interpolate(presentRotation, key.mValue, nextKey.mValue, factor);
				}
				else {
					presentRotation = key.mValue;
				}
			}

			// ******** Scaling **********
			aiVector3D presentScaling(1, 1, 1);
			if (channel->mNumScalingKeys > 0) {
				unsigned int frame = 0;
				while (frame < channel->mNumScalingKeys - 1) {
					if (time < channel->mScalingKeys[frame + 1].mTime) {
						break;
					}
					++frame;
				}

				// TODO: (thom) interpolation maybe? This time maybe even logarithmic, not linear
				presentScaling = channel->mScalingKeys[frame].mValue;
			}

			// build a transformation matrix from it
			aiMatrix4x4& mat = mTransforms[a];
			mat = aiMatrix4x4(presentRotation.GetMatrix());
			mat.a1 *= presentScaling.x; mat.b1 *= presentScaling.x; mat.c1 *= presentScaling.x;
			mat.a2 *= presentScaling.y; mat.b2 *= presentScaling.y; mat.c2 *= presentScaling.y;
			mat.a3 *= presentScaling.z; mat.b3 *= presentScaling.z; mat.c3 *= presentScaling.z;
			mat.a4 = presentPosition.x; mat.b4 = presentPosition.y; mat.c4 = presentPosition.z;
		}
	}
};

/********** AssimpModel **********/
AssimpModel::AssimpModel(Launch launchMode, ResourceManager& resourceMng, TransformPtr pMove, const std::string& matType)
	:mLaunchMode(launchMode), mResourceMng(resourceMng)
{
	mTransform = pMove ? pMove : std::make_shared<Transform>();
	mMaterial = resourceMng.CreateMaterial(launchMode, matType);
}

AssimpModel::~AssimpModel()
{}

void AssimpModel::LoadModel(const std::string& assetPath, const std::string& redirectResource)
{
	mAiScene = mResourceMng.CreateAiScene(mLaunchMode, mMaterial, assetPath, redirectResource);
	mNodeInfos = mAiScene->mNodeInfos;
	Update(0);
}

const std::vector<aiMatrix4x4>& AssimpModel::GetBoneMatrices(const aiNode* pNode, size_t pMeshIndex)
{
	BOOST_ASSERT(pMeshIndex < pNode->mNumMeshes);
	size_t meshIndex = pNode->mMeshes[pMeshIndex];

	BOOST_ASSERT(meshIndex < mAiScene->mScene->mNumMeshes);
	const aiMesh* mesh = mAiScene->mScene->mMeshes[meshIndex];

	mTransforms.resize(mesh->mNumBones, aiMatrix4x4());

	aiMatrix4x4 globalInverseMeshTransform = mNodeInfos[pNode].mGlobalTransform;
	globalInverseMeshTransform.Inverse();

	for (size_t a = 0; a < mesh->mNumBones; ++a) {
		const aiBone* bone = mesh->mBones[a];
		const aiNode* boneNode = mAiScene->mBoneNodesByName[bone->mName.data];
		auto& boneInfo = mNodeInfos[boneNode];
		aiMatrix4x4 currentGlobalTransform = boneInfo.mGlobalTransform;
		mTransforms[a] = globalInverseMeshTransform * currentGlobalTransform * bone->mOffsetMatrix;
	}
	return mTransforms;
}

void VisitNode(const aiNode* cur, 
	std::map<const aiNode*, AiNodeInfo>& mNodeInfos, 
	std::vector<const aiNode*>& vec, 
	EvaluateTransforms& eval) 
{
	vec.push_back(cur);

	auto& nodeInfo = mNodeInfos[cur];
	if (nodeInfo.channelIndex >= 0 && nodeInfo.channelIndex < eval.mTransforms.size()) {
		nodeInfo.mLocalTransform = (eval.mTransforms[nodeInfo.channelIndex]);
	}
	else {
		nodeInfo.mLocalTransform = (cur->mTransformation);
	}

	for (int i = 0; i < cur->mNumChildren; ++i) {
		VisitNode(cur->mChildren[i], mNodeInfos, vec, eval);
	}
}

void AssimpModel::Update(float dt)
{
	std::vector<const aiNode*> vec;
	if (mCurrentAnimIndex < 0 || mCurrentAnimIndex >= mAiScene->mScene->mNumAnimations) {
		std::queue<const aiNode*> que;
		que.push(mAiScene->mRootNode);
		while (!que.empty()) {
			const aiNode* cur = que.front();
			que.pop();
			vec.push_back(cur);
			for (int i = 0; i < cur->mNumChildren; ++i)
				que.push(cur->mChildren[i]);
		}
	}
	else {
		aiAnimation* aiAnim = mAiScene->mScene->mAnimations[mCurrentAnimIndex];
		EvaluateTransforms eval(aiAnim);

		mElapse += dt;
		eval.Update(mElapse);

		VisitNode(mAiScene->mRootNode, mNodeInfos, vec, eval);
	}

	for (int i = 0; i < vec.size(); ++i) {
		const aiNode* cur = vec[i];
		auto& nodeInfo = mNodeInfos[cur];

		nodeInfo.mGlobalTransform = (nodeInfo.mLocalTransform);
		const aiNode* iter = cur->mParent;
		while (iter) {
			nodeInfo.mGlobalTransform = mNodeInfos[iter].mLocalTransform * nodeInfo.mGlobalTransform;
			iter = iter->mParent;
		}
	}
}

void AssimpModel::PlayAnim(int Index)
{
	mCurrentAnimIndex = Index;
	mElapse = 0;
	if (mCurrentAnimIndex < 0 || mCurrentAnimIndex >= mAiScene->mScene->mNumAnimations) return;

	const aiAnimation* currentAnim = mAiScene->mScene->mAnimations[mCurrentAnimIndex];

	for (auto& iter : mNodeInfos) {
		std::string iterName = iter.first->mName.C_Str();
		for (unsigned int a = 0; a < currentAnim->mNumChannels; a++) {
			if (iterName == currentAnim->mChannels[a]->mNodeName.C_Str()) {
				iter.second.channelIndex = a;
				break;
			}
		}
	}
}

void AssimpModel::DoDraw(const aiNode* node, RenderOperationQueue& opList)
{
	const auto& meshArr = mNodeInfos[node];
	if (meshArr.MeshCount() > 0) {
		cbWeightedSkin weightedSkin = {};
		weightedSkin.Model = AS_CONST_REF(Eigen::Matrix4f, mNodeInfos[node].mGlobalTransform);

		for (int i = 0; i < meshArr.MeshCount(); i++) {
			AssimpMeshPtr mesh = meshArr[i];
			if (mesh->GetAiMesh()->HasBones()) {
				const std::vector<aiMatrix4x4>& boneMatArr = GetBoneMatrices(node, i);
				size_t boneSize = boneMatArr.size(); 
				//BOOST_ASSERT(boneSize <= MAX_MATRICES);
				for (int j = 0; j < std::min<int>(MAX_MATRICES, boneSize); ++j)
					weightedSkin.Models[j] = AS_CONST_REF(Eigen::Matrix4f, boneMatArr[j]);
			}
			else {
				weightedSkin.Models[0] = Eigen::Matrix4f::Identity();
			}

			weightedSkin.hasNormal = mesh->HasTexture(kTexturePbrNormal);
			weightedSkin.hasMetalness = mesh->HasTexture(kTexturePbrMetalness);
			weightedSkin.hasRoughness = mesh->HasTexture(kTexturePbrRoughness);
			weightedSkin.hasAO = mesh->HasTexture(kTexturePbrAo);

			if (mesh->IsLoaded()) {
				RenderOperation op = {};
				op.mMaterial = mMaterial;
				op.mIndexBuffer = mesh->GetIndexBuffer();
				op.mVertexBuffer = mesh->GetVertexBuffer();
				op.mTextures = *mesh->GetTextures();
				op.SetConstantBufferBytes(MAKE_CBNAME(cbWeightedSkin), weightedSkin);
				opList.AddOP(op);
			}
		}
	}

	for (int i = 0; i < node->mNumChildren; i++)
		DoDraw(node->mChildren[i], opList);
}

int AssimpModel::GenRenderOperation(RenderOperationQueue& opList)
{
	if (!mMaterial->IsLoaded() || !mAiScene->IsLoaded())
		return 0;

	int count = opList.Count();
	DoDraw(mAiScene->mRootNode, opList);

	Eigen::Matrix4f world = mTransform->GetMatrix();
	for (int i = count; i < opList.Count(); ++i)
		opList[i].mWorldTransform = world;

	return opList.Count() - count;
}

}