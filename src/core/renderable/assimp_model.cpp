#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/assert.hpp>
#include "core/renderable/assimp_model.h"
#include "core/resource/resource_manager.h"
#include "core/base/transform.h"

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
AssimpModel::AssimpModel(Launch launchMode, ResourceManager& resourceMng, const MaterialLoadParam& matName)
	:Super(launchMode, resourceMng, matName)
{}

void AssimpModel::LoadModel(const std::string& assetPath, const std::string& redirectResource)
{
	mAiScene = mResourceMng.CreateAiScene(mLaunchMode, assetPath, redirectResource);
	
	if (mAiScene->IsLoaded()) {
		mAnimeTree.Init(mAiScene->GetSerializeNodes());
		Update(0);
	}
	else {
		mInitAnimeTreeTask = [this]() {
			mAnimeTree.Init(mAiScene->GetSerializeNodes());
		};
	}
}

const std::vector<aiMatrix4x4>& AssimpModel::GetBoneMatrices(const AiNodePtr& node, size_t meshIndexIndex)
{
	BOOST_ASSERT(meshIndexIndex < node->MeshCount());
	size_t meshIndex = node->RawNode->mMeshes[meshIndexIndex];

	BOOST_ASSERT(meshIndex < mAiScene->mScene->mNumMeshes);
	const aiMesh* rawMesh = mAiScene->mScene->mMeshes[meshIndex];

	mTempBoneMatrices.resize(rawMesh->mNumBones, aiMatrix4x4());

	auto& anode = mAnimeTree.GetNode(node);
	aiMatrix4x4 globalInverseMeshTransform = anode.GlobalTransform;
	globalInverseMeshTransform.Inverse();

	for (size_t a = 0; a < rawMesh->mNumBones; ++a) {
		const aiBone* rawBone = rawMesh->mBones[a];
		AiNodePtr boneNode = mAiScene->mBoneNodesByName[rawBone->mName.data];
		if (boneNode) {
			auto& boneInfo = mAnimeTree.GetNode(boneNode);// mNodeInfos[boneNode];
			aiMatrix4x4 currentGlobalTransform = boneInfo.GlobalTransform;
			mTempBoneMatrices[a] = globalInverseMeshTransform * currentGlobalTransform * rawBone->mOffsetMatrix;
		}
		else {
			mTempBoneMatrices[a] = globalInverseMeshTransform * rawBone->mOffsetMatrix;
		}
	}
	return mTempBoneMatrices;
}

void VisitNode(const AiNodePtr& curNode, const AiAnimeTree& animeTree, 
	std::vector<AiNodePtr>& nodeVec, EvaluateTransforms& eval) 
{
	nodeVec.push_back(curNode);

	auto& nodeInfo = animeTree.GetNode(curNode);
	if (nodeInfo.ChannelIndex >= 0 && nodeInfo.ChannelIndex < eval.mTransforms.size()) {
		nodeInfo.LocalTransform = (eval.mTransforms[nodeInfo.ChannelIndex]);
	}
	else {
		nodeInfo.LocalTransform = (curNode->RawNode->mTransformation);
	}

	for (int i = 0; i < curNode->ChildCount(); ++i) {
		VisitNode(curNode->Children[i], animeTree, nodeVec, eval);
	}
}

void AssimpModel::Update(float dt)
{
	if (!mAiScene->IsLoaded())
		return;
	if (auto task = std::move(mInitAnimeTreeTask))
		task();
	if (auto task = std::move(mPlayAnimTask))
		task();

	std::vector<AiNodePtr> nodeVec;
	if (mCurrentAnimIndex < 0 || mCurrentAnimIndex >= mAiScene->mScene->mNumAnimations) {
		std::queue<AiNodePtr> nodeQue;
		nodeQue.push(mAiScene->mRootNode);
		while (!nodeQue.empty()) {
			AiNodePtr curNode = nodeQue.front();
			nodeQue.pop();
			nodeVec.push_back(curNode);
			for (int i = 0; i < curNode->ChildCount(); ++i)
				nodeQue.push(curNode->Children[i]);
		}
	}
	else {
		aiAnimation* aiAnim = mAiScene->mScene->mAnimations[mCurrentAnimIndex];
		EvaluateTransforms eval(aiAnim);

		mElapse += dt;
		eval.Update(mElapse);

		VisitNode(mAiScene->mRootNode, mAnimeTree, nodeVec, eval);
		auto& nodeInfo = mAnimeTree.GetNode(mAiScene->mRootNode);
		nodeInfo.LocalTransform = aiMatrix4x4();
	}

	for (int i = 0; i < nodeVec.size(); ++i) {
		AiNodePtr curNode = nodeVec[i];
		auto& curANode = mAnimeTree.GetNode(curNode);

		curANode.GlobalTransform = curANode.LocalTransform;
		AiNodePtr iterNode = curNode->Parent.lock();
		while (iterNode) {
			auto& iterANode = mAnimeTree.GetNode(iterNode);
			curANode.GlobalTransform = iterANode.LocalTransform * curANode.GlobalTransform;
			iterNode = iterNode->Parent.lock();
		}
	}
}

void AssimpModel::PlayAnim(int Index)
{
	mCurrentAnimIndex = Index;
	mElapse = 0;
	if (mAiScene == nullptr) 
		return;

	auto playAnimTask = [this]() {
		if (mCurrentAnimIndex < 0 || mCurrentAnimIndex >= mAiScene->mScene->mNumAnimations) return;

		const aiAnimation* currentAnim = mAiScene->mScene->mAnimations[mCurrentAnimIndex];

		for (auto& iter : *mAiScene) {
			std::string iterName = iter->RawNode->mName.C_Str();
			for (unsigned int a = 0; a < currentAnim->mNumChannels; a++) {
				if (iterName == currentAnim->mChannels[a]->mNodeName.C_Str()) {
					mAnimeTree.GetNode(iter).ChannelIndex = a;
					break;
				}
			}
		}
	};
	if (mAiScene->IsLoaded()) playAnimTask();
	else mPlayAnimTask = playAnimTask;
}

void AssimpModel::DoDraw(const AiNodePtr& node, RenderOperationQueue& opList)
{
	const auto& meshArr = *node;// mNodeInfos[node];
	if (meshArr.MeshCount() > 0) {
		cbWeightedSkin weightedSkin = {};
		auto& anode = mAnimeTree.GetNode(node);
	#if defined EIGEN_DONT_ALIGN_STATICALLY
		weightedSkin.Model = AS_CONST_REF(Eigen::Matrix4f, anode.GlobalTransform);
	#else
		const auto& s = anode.GlobalTransform;
		weightedSkin.Model << 
			 s.a1, s.b1, s.c1, s.d1,
			 s.a2, s.b2, s.c2, s.d2,
			 s.a3, s.b3, s.c3, s.d3,
			 s.a4, s.b4, s.c4, s.d4;
	#endif
		for (int i = 0; i < meshArr.MeshCount(); i++) {
			AssimpMeshPtr mesh = meshArr[i];
			if (mesh->GetRawMesh()->HasBones()) {
				const std::vector<aiMatrix4x4>& boneMatArr = GetBoneMatrices(node, i);
				size_t boneSize = boneMatArr.size(); 
				for (int j = 0; j < std::min<int>(cbWeightedSkin::kModelCount, boneSize); ++j) {
				#if defined EIGEN_DONT_ALIGN_STATICALLY
					weightedSkin.Models[j] = AS_CONST_REF(Eigen::Matrix4f, boneMatArr[j]);
				#else
					const auto& s = boneMatArr[j];
					weightedSkin.Models[j] <<
						s.a1, s.b1, s.c1, s.d1,
						s.a2, s.b2, s.c2, s.d2,
						s.a3, s.b3, s.c3, s.d3,
						s.a4, s.b4, s.c4, s.d4;
				#endif
				}
					
			}
			else {
				weightedSkin.Models[0] = Eigen::Matrix4f::Identity();
			}

			cbModel model = {};
			model.EnableNormalMap = mesh->HasTexture(kTexturePbrNormal);
			model.EnableMetalnessMap = mesh->HasTexture(kTexturePbrMetalness);
			model.EnableRoughnessMap = mesh->HasTexture(kTexturePbrRoughness);
			model.EnableAmbientOcclusionMap = mesh->HasTexture(kTexturePbrAo);
			model.EnableAlbedoMap = mesh->HasTexture(kTexturePbrAlbedo);
			model.EnableAOMap_ChGRoughness_ChBMetalness = model.EnableAmbientOcclusionMap 
				&& !model.EnableRoughnessMap 
				&& !model.EnableMetalnessMap;

			if (mesh->IsLoaded()) {
				RenderOperation op = {};
				op.Material = mMaterial;
				op.IndexBuffer = mesh->GetIndexBuffer();
				op.AddVertexBuffer(mesh->GetVBOSurface());
				op.AddVertexBuffer(mesh->GetVBOSkeleton());
				op.Textures = *mesh->GetTextures();
				op.SetUBOBytes(MAKE_CBNAME(cbWeightedSkin), weightedSkin);
				op.SetUBOBytes(MAKE_CBNAME(cbModel), model);
				op.CameraMask = mCameraMask;
				opList.AddOP(op);
			}
		}
	}

	for (int i = 0; i < node->Children.size(); i++)
		DoDraw(node->Children[i], opList);
}

void AssimpModel::GenRenderOperation(RenderOperationQueue& opList)
{
	if (!mMaterial->IsLoaded() 
		|| !mAiScene->IsLoaded() 
		|| !mAnimeTree.IsInited())
		return;

	int count = opList.Count();
	DoDraw(mAiScene->mRootNode, opList);

	Eigen::Matrix4f world = mTransform->GetSRT();
	for (int i = count; i < opList.Count(); ++i)
		opList[i].WorldTransform = world;
}

}