#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/assert.hpp>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include "core/base/debug.h"
#include "core/scene/transform.h"
#include "core/renderable/assimp_model.h"
#include "core/resource/resource_manager.h"
#include "core/resource/assimp_factory.h"

namespace mir {
namespace rend {

#define AS_CONST_REF(TYPE, V) *(const TYPE*)(&V)
#define AS_REF(TYPE, V) *(TYPE*)(&V)
struct EvaluateTransforms
{
public:
	std::vector<Eigen::Matrix4f> mTransforms;
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

		for (unsigned int channelIndex = 0; channelIndex < mAnim->mNumChannels; ++channelIndex) {
			const aiNodeAnim* channel = mAnim->mChannels[channelIndex];

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
			aiMatrix4x4& mat = (aiMatrix4x4&)mTransforms[channelIndex];
			mat = aiMatrix4x4(presentRotation.GetMatrix());
			mat.a1 *= presentScaling.x; mat.b1 *= presentScaling.x; mat.c1 *= presentScaling.x;
			mat.a2 *= presentScaling.y; mat.b2 *= presentScaling.y; mat.c2 *= presentScaling.y;
			mat.a3 *= presentScaling.z; mat.b3 *= presentScaling.z; mat.c3 *= presentScaling.z;
			mat.a4 = presentPosition.x; mat.b4 = presentPosition.y; mat.c4 = presentPosition.z;
		}
	}
};

/********** AssimpModel **********/
CoTask<bool> AssimpModel::LoadModel(std::string assetPath, std::string redirectResource)
{
	if (!CoAwait mResMng.CreateAiScene(mAiScene, mLaunchMode, std::move(assetPath), std::move(redirectResource), mLoadParam)) 
		CoReturn false;

	COROUTINE_VARIABLES_2(assetPath, redirectResource);
	mAABB = mAiScene->GetAABB();
	mAnimeTree.Init(mAiScene->GetNodes());
	CoAwait UpdateFrame(0);
	CoReturn true;
}

const std::vector<Eigen::Matrix4f>& AssimpModel::GetBoneMatrices(const res::AiNodePtr& node, const res::AssimpMeshPtr& mesh)
{
	auto& animRoot = mAnimeTree.GetNode(node);
	aiMatrix4x4 rootGlobalTransformInv = AS_REF(aiMatrix4x4, animRoot.GlobalTransform);
	rootGlobalTransformInv.Inverse();

	auto& bones = mesh->GetBones();
	mTempBoneMatrices.resize(bones.size());
	for (size_t i = 0; i < bones.size(); ++i) {
		const auto& bone = bones[i];
		res::AiNodePtr boneNode = bone.mRelateNode.lock();
		if (boneNode) {
			auto& animNode = mAnimeTree.GetNode(boneNode);
			const aiMatrix4x4& nodeGlobalTransform = AS_CONST_REF(aiMatrix4x4, animNode.GlobalTransform);
			AS_REF(aiMatrix4x4, mTempBoneMatrices[i]) = rootGlobalTransformInv * nodeGlobalTransform * AS_CONST_REF(aiMatrix4x4, bone.mOffsetMatrix);
		}
		else {
			AS_REF(aiMatrix4x4, mTempBoneMatrices[i]) = rootGlobalTransformInv * AS_CONST_REF(aiMatrix4x4, bone.mOffsetMatrix);
		}
	}
	return mTempBoneMatrices;
}

void VisitNode(const res::AiNodePtr& curNode, const AiAnimeTree& animeTree, std::vector<res::AiNodePtr>& nodeVec, EvaluateTransforms& eval)
{
	nodeVec.push_back(curNode);

	auto& curAnimeNode = animeTree.GetNode(curNode);
	if (curAnimeNode.ChannelIndex >= 0 && curAnimeNode.ChannelIndex < eval.mTransforms.size()) {
		curAnimeNode.LocalTransform = eval.mTransforms[curAnimeNode.ChannelIndex];
	}
	else {
		curAnimeNode.LocalTransform = (curNode->GetLocalTransform());
	}

	for (const auto& child : curNode->GetChildren()) {
		VisitNode(child, animeTree, nodeVec, eval);
	}
}

CoTask<void> AssimpModel::UpdateFrame(float dt)
{
	CoAwait Super::UpdateFrame(dt);
	BOOST_ASSERT(mAiScene->IsLoaded());
	if (mAiScene == nullptr || !mAiScene->IsLoaded()) CoReturn;

#if MIR_MATERIAL_HOTLOAD
	for (auto& mesh : mAiScene->GetMeshes()) {
		auto material = mesh->GetMaterial();
		if (material->IsOutOfDate()) {
			CoAwait material.Reload(mLaunchMode, mResMng);
		}
	}
#endif

	std::vector<res::AiNodePtr> nodeVec;
	if (mCurrentAnimIndex < 0 || mCurrentAnimIndex >= mAiScene->mAnimations.size()) {
		std::queue<res::AiNodePtr> nodeQue;
		nodeQue.push(mAiScene->mRootNode);
		while (!nodeQue.empty()) {
			res::AiNodePtr curNode = nodeQue.front();
			nodeQue.pop();
			nodeVec.push_back(curNode);
			for (int i = 0; i < curNode->ChildCount(); ++i)
				nodeQue.push(curNode->Children[i]);
		}
	}
	else {
		const aiAnimation* aiAnim = mAiScene->mAnimations[mCurrentAnimIndex];
		EvaluateTransforms eval(aiAnim);

		mElapse += dt;
		eval.Update(mElapse);

		VisitNode(mAiScene->mRootNode, mAnimeTree, nodeVec, eval);
	#if defined ROOT_TPOSE_IDENTITY
		auto& nodeInfo = mAnimeTree.GetNode(mAiScene->mRootNode);
		nodeInfo.LocalTransform = aiMatrix4x4();
	#endif
	}

	for (res::AiNodePtr curNode : nodeVec) {
		auto& curAnimNode = mAnimeTree.GetNode(curNode);
		curAnimNode.GlobalTransform = curAnimNode.LocalTransform;
		res::AiNodePtr iterNode = curNode->Parent.lock();
		while (iterNode) {
			auto& iterANode = mAnimeTree.GetNode(iterNode);
			curAnimNode.GlobalTransform = iterANode.LocalTransform * curAnimNode.GlobalTransform;
			iterNode = iterNode->Parent.lock();
		}
	}
	CoReturn;
}

void AssimpModel::PlayAnim(int Index)
{
	BOOST_ASSERT(mAiScene->IsLoaded());
	if (mAiScene == nullptr || !mAiScene->IsLoaded()) return;

	mCurrentAnimIndex = Index;
	mElapse = 0;
	if (mCurrentAnimIndex < 0 || mCurrentAnimIndex >= mAiScene->mAnimations.size()) return;

	const aiAnimation* currentAnim = mAiScene->mAnimations[mCurrentAnimIndex];
	for (auto& node : mAiScene->GetNodes()) {
		const std::string& nodeName = node->mName;
		for (unsigned int channelIndex = 0; channelIndex < currentAnim->mNumChannels; channelIndex++) {
			if (currentAnim->mChannels[channelIndex]->mNodeName.C_Str() == nodeName) {
				mAnimeTree.GetNode(node).ChannelIndex = channelIndex;
				break;
			}
		}
	}
}

void AssimpModel::DoDraw(const res::AiNodePtr& node, RenderOperationQueue& ops)
{
	if (node->MeshCount() > 0) {
		auto& rootAnimeNode = mAnimeTree.GetNode(node);

	#if defined EIGEN_DONT_ALIGN_STATICALLY
		const auto& rootModel = AS_CONST_REF(Eigen::Matrix4f, rootAnimeNode.GlobalTransform);
	#else
		Eigen::Matrix4f rootModel;
		const auto& s = rootAnimeNode.GlobalTransform;
		rootModel <<
			s.a1, s.b1, s.c1, s.d1,
			s.a2, s.b2, s.c2, s.d2,
			s.a3, s.b3, s.c3, s.d3,
			s.a4, s.b4, s.c4, s.d4;
	#endif
		for (const auto& mesh : node->GetMeshes()) 
		{
			res::MaterialInstance mat = mesh->GetMaterial();

			mat.SetProperty<Eigen::Matrix4f>("Model", rootModel);

			using ModelArray56 = std::array<Eigen::Matrix4f, 56>;
			ModelArray56& models = mat.GetProperty<ModelArray56>("Models");
			if (mesh->HasBones()) {
				const auto& boneMats = GetBoneMatrices(node, mesh);
				size_t boneCount = boneMats.size(); 
				for (int j = 0; j < std::min<int>(cbWeightedSkin::kModelCount, boneCount); ++j) {
				#if defined EIGEN_DONT_ALIGN_STATICALLY
					models[j] = AS_CONST_REF(Eigen::Matrix4f, boneMats[j]);
				#else
					const auto& s = boneMatArr[j];
					models[j] <<
						s.a1, s.b1, s.c1, s.d1,
						s.a2, s.b2, s.c2, s.d2,
						s.a3, s.b3, s.c3, s.d3,
						s.a4, s.b4, s.c4, s.d4;
				#endif
				}
			}
			else {
				models[0] = Eigen::Matrix4f::Identity();
			}

			if (mesh->IsLoaded()) {
				RenderOperation op = {};
				op.IndexBuffer = mesh->GetIndexBuffer();
				op.AddVertexBuffer(mesh->GetVBOSurface());
				op.AddVertexBuffer(mesh->GetVBOSkeleton());
				op.Material = mat;
				op.CameraMask = mCameraMask;
				ops.AddOP(op);
			}
		}
	}

	for (const auto& child : node->GetChildren())
		DoDraw(child, ops);
}

void AssimpModel::GenRenderOperation(RenderOperationQueue& opList)
{
	if (!mAiScene->IsLoaded()
		|| !mAnimeTree.IsInited())
		return;

	int count = opList.Count();
	DoDraw(mAiScene->mRootNode, opList);

	Eigen::Matrix4f world = GetTransform()->GetWorldMatrix();
	for (int i = count; i < opList.Count(); ++i) {
		opList[i].CastShadow = mCastShadow;
		opList[i].CameraMask = mCameraMask;
		opList[i].WorldTransform = world;
	}
}

}
}