#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "core/renderable/assimp_model.h"
#include "core/rendersys/material.h"
#include "core/rendersys/material_cb.h"
#include "core/rendersys/material_factory.h"
#include "core/rendersys/resource_manager.h"
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
AssimpModel::AssimpModel(ResourceManager& resourceMng, MaterialFactory& matFac, TransformPtr pMove, const std::string& matType)
	:mResourceMng(resourceMng)
{
	mTransform = pMove ? pMove : std::make_shared<Transform>();
	mMaterial = matFac.GetMaterial(matType);
}

AssimpModel::~AssimpModel()
{
	delete mImporter;
}

const unsigned int ImportFlags =
	aiProcess_ConvertToLeftHanded |
	aiProcess_Triangulate |
	aiProcess_CalcTangentSpace;/* |
	aiProcess_SortByPType |
	aiProcess_PreTransformVertices |
	aiProcess_GenNormals |
	aiProcess_GenUVCoords |
	aiProcess_OptimizeMeshes |
	aiProcess_Debone |
	aiProcess_ValidateDataStructure;*/
void AssimpModel::LoadModel(const std::string& imgPath, const std::string& redirectResource)
{
	TIME_PROFILE(AssimpModel_LoadModel);

	boost::filesystem::path imgFullpath = boost::filesystem::system_complete(imgPath);

	namespace boost_property_tree = boost::property_tree;
	boost_property_tree::ptree pt;
	boost_property_tree::read_json(std::stringstream(redirectResource), pt);
	mRedirectResourceDir = pt.get<std::string>("dir", "");
	mRedirectResourceExt = pt.get<std::string>("ext", "");

	if (!mRedirectResourceDir.empty()) {
		mRedirectResourceDir = boost::filesystem::system_complete(mRedirectResourceDir).string();
	}
	else {
		mRedirectResourceDir = imgFullpath.parent_path().string();
	}
	BOOST_ASSERT(mRedirectResourceDir.empty() || mRedirectResourceDir.back() != '/');

	{
		TIME_PROFILE(Assimp_Importer);
		delete mImporter;
		mImporter = new Assimp::Importer;
		mScene = mImporter->ReadFile(imgFullpath.string(), ImportFlags);
	}

	assert(mScene != nullptr);
	for (unsigned int i = 0; i < mScene->mNumMeshes; ++i) {
		const aiMesh* mesh = mScene->mMeshes[i];
		for (unsigned int n = 0; n < mesh->mNumBones; ++n) {
			const aiBone* bone = mesh->mBones[n];

			assert(mBoneNodesByName[bone->mName.data] == nullptr || mBoneNodesByName[bone->mName.data] == mScene->mRootNode->FindNode(bone->mName));
			mBoneNodesByName[bone->mName.data] = mScene->mRootNode->FindNode(bone->mName);
		}
	}

	mRootNode = mScene->mRootNode;
	processNode(mScene->mRootNode, mScene);

	{
		TIME_PROFILE(AssimpModel_LoadModel_Update);
		Update(0);
	}
}

void AssimpModel::processNode(aiNode* node, const aiScene* scene)
{
	mNodeInfos[node].mLocalTransform = node->mTransformation;
	mNodeInfos[node].mGlobalTransform = mNodeInfos[node].mLocalTransform;
	for (int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* meshData = scene->mMeshes[node->mMeshes[i]];
		auto mesh = processMesh(meshData, scene);
		mMeshes.push_back(mesh);
		mNodeInfos[node].AddMesh(mesh);
	}

	for (int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene);
	}
}

void ReCalculateTangents(std::vector<AssimpMeshVertex>& vertices, const std::vector<UINT>& indices) {
	for (int i = 0; i < indices.size(); i += 3) {
		// Shortcuts for vertices
		AssimpMeshVertex& v0 = vertices[indices[i+0]];
		AssimpMeshVertex& v1 = vertices[indices[i+1]];
		AssimpMeshVertex& v2 = vertices[indices[i+2]];

		// Shortcuts for UVs
		Eigen::Vector2f& uv0 = v0.Tex;
		Eigen::Vector2f& uv1 = v1.Tex;
		Eigen::Vector2f& uv2 = v2.Tex;

		// Edges of the triangle : postion delta
		Eigen::Vector3f deltaPos1 = v1.Pos - v0.Pos;
		Eigen::Vector3f deltaPos2 = v2.Pos - v0.Pos;

		// UV delta
		Eigen::Vector2f deltaUV1 = uv1 - uv0;
		Eigen::Vector2f deltaUV2 = uv2 - uv0;
	#if !defined MESH_VETREX_POSTEX
		float r = 1.0f / (deltaUV1.x() * deltaUV2.y() - deltaUV1.y() * deltaUV2.x());
		Eigen::Vector3f tangent = (deltaPos1 * deltaUV2.y() - deltaPos2 * deltaUV1.y()) * r;
		v0.Tangent = v0.Tangent + tangent;
		v1.Tangent = v1.Tangent + tangent;
		v2.Tangent = v2.Tangent + tangent;

		Eigen::Vector3f bitangent = (deltaPos2 * deltaUV1.x() - deltaPos1 * deltaUV2.x()) * r;
		v0.BiTangent = v0.BiTangent + bitangent;
		v1.BiTangent = v1.BiTangent + bitangent;
		v2.BiTangent = v2.BiTangent + bitangent;
	#endif
	}
}

AssimpMeshPtr AssimpModel::processMesh(aiMesh * mesh, const aiScene * scene)
{
	// Data to fill
	std::vector<AssimpMeshVertex> vertices;
	std::vector<UINT> indices;
	TextureBySlotPtr texturesPtr = std::make_shared<TextureBySlot>();
	TextureBySlot& textures = *texturesPtr;
	textures.Resize(4);

	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
		//if (textype.empty()) textype = determineTextureType(scene, mat);
	}

	for (size_t vertexId = 0; vertexId < mesh->mNumVertices; vertexId++)
	{
		AssimpMeshVertex vertex;
		memset(&vertex, 0, sizeof(vertex));
		vertex.Pos = AS_CONST_REF(Eigen::Vector3f, mesh->mVertices[vertexId]);
		if (mesh->mTextureCoords[0]) {
			vertex.Tex.x() = mesh->mTextureCoords[0][vertexId].x;
			vertex.Tex.y() = mesh->mTextureCoords[0][vertexId].y;
		}
	#if !defined MESH_VETREX_POSTEX
		if (mesh->mNormals) vertex.Normal = AS_CONST_REF(Eigen::Vector3f, mesh->mNormals[vertexId]);
		if (mesh->mTangents) vertex.Tangent = AS_CONST_REF(Eigen::Vector3f, mesh->mTangents[vertexId]);
		if (mesh->mBitangents) vertex.BiTangent = AS_CONST_REF(Eigen::Vector3f, mesh->mBitangents[vertexId]);
	#endif
		vertices.push_back(vertex);
	}

#if !defined MESH_VETREX_POSTEX
	if (mesh->HasBones()) {
		std::map<int, int> spMap;
		for (int boneId = 0; boneId < mesh->mNumBones; ++boneId) {
			aiBone* bone = mesh->mBones[boneId];
			for (int k = 0; k < bone->mNumWeights; ++k) {
				aiVertexWeight& vw = bone->mWeights[k];
				if (vw.mVertexId >= 0 && vw.mVertexId < vertices.size()) {
					int sp = spMap[vw.mVertexId];
					if (sp < 4) {
						FLOAT* BlendWeights = (FLOAT*)&vertices[vw.mVertexId].BlendWeights;
						BlendWeights[sp] = vw.mWeight;

						unsigned int* BlendIndices = (unsigned int*)&vertices[vw.mVertexId].BlendIndices;
						BlendIndices[sp] = boneId;

						spMap[vw.mVertexId]++;
					}
				}
			}
		}
	}
#endif

	for (UINT i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (UINT j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	if (mesh->mNormals && mesh->mTangents == nullptr) {
		ReCalculateTangents(vertices, indices);
	}

	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		std::vector<ITexturePtr> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, scene);
		if (diffuseMaps.size() > 0) 
			textures[kTextureDiffuse] = diffuseMaps[0];

		std::vector<ITexturePtr> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, scene);
		if (specularMaps.size() > 0) 
			textures[kTextureSpecular] = specularMaps[0];

		std::vector<ITexturePtr> normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, scene);
		if (normalMaps.size() > 0) 
			textures[kTextureNormal] = normalMaps[0];
	}

	auto material = mMaterial->Clone(mResourceMng);
	//if (mMatCb) mMatCb(material);
	return AssimpMesh::Create(mesh, vertices, indices, texturesPtr, material, mResourceMng);
}

std::vector<ITexturePtr> AssimpModel::loadMaterialTextures(aiMaterial* mat, 
	aiTextureType type, 
	const aiScene* scene)
{
	boost::filesystem::path redirectPath(mRedirectResourceDir);
	std::vector<ITexturePtr> textures;
	for (UINT i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str; mat->GetTexture(type, i, &str);
		std::string key = str.C_Str();

		if (!mRedirectResourceDir.empty()) {
			boost::filesystem::path texturePath(key);
			if (texturePath.is_relative()) texturePath = redirectPath.append(texturePath.string());
			else texturePath = redirectPath.append(texturePath.filename().string());

			if (!mRedirectResourceExt.empty()) 
				texturePath = texturePath.replace_extension(mRedirectResourceExt);

			if (!boost::filesystem::exists(texturePath)) 
				texturePath = texturePath.replace_extension("png");

			if (!boost::filesystem::exists(texturePath))
				continue;

			key = texturePath.string();
		}

		ITexturePtr texInfo = mResourceMng.CreateTexture(key, kFormatUnknown, true, false);
		textures.push_back(texInfo);
		mLoadedTexture[key] = texInfo;
	}
	return textures;
}

const std::vector<aiMatrix4x4>& AssimpModel::GetBoneMatrices(const aiNode* pNode, size_t pMeshIndex)
{
	assert(pMeshIndex < pNode->mNumMeshes);
	size_t meshIndex = pNode->mMeshes[pMeshIndex];

	assert(meshIndex < mScene->mNumMeshes);
	const aiMesh* mesh = mScene->mMeshes[meshIndex];

	mTransforms.resize(mesh->mNumBones, aiMatrix4x4());

	aiMatrix4x4 globalInverseMeshTransform = mNodeInfos[pNode].mGlobalTransform;
	globalInverseMeshTransform.Inverse();

	for (size_t a = 0; a < mesh->mNumBones; ++a) {
		const aiBone* bone = mesh->mBones[a];
		const aiNode* boneNode = mBoneNodesByName[bone->mName.data];
		auto& boneInfo = mNodeInfos[boneNode];
		aiMatrix4x4 currentGlobalTransform = boneInfo.mGlobalTransform;
		mTransforms[a] = globalInverseMeshTransform * currentGlobalTransform * bone->mOffsetMatrix;
	}
	return mTransforms;
}

void VisitNode(aiNode* cur, 
	std::map<const aiNode*, AiNodeInfo>& mNodeInfos, 
	std::vector<aiNode*>& vec, 
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
	std::vector<aiNode*> vec;
	if (mCurrentAnimIndex < 0 || mCurrentAnimIndex >= mScene->mNumAnimations) {
		std::queue<aiNode*> que;
		que.push(mRootNode);
		while (!que.empty()) {
			aiNode* cur = que.front();
			que.pop();
			vec.push_back(cur);
			for (int i = 0; i < cur->mNumChildren; ++i)
				que.push(cur->mChildren[i]);
		}
	}
	else {
		aiAnimation* aiAnim = mScene->mAnimations[mCurrentAnimIndex];
		EvaluateTransforms eval(aiAnim);

		mElapse += dt;
		eval.Update(mElapse);

		VisitNode(mRootNode, mNodeInfos, vec, eval);
	}

	for (int i = 0; i < vec.size(); ++i) {
		aiNode* cur = vec[i];
		auto& nodeInfo = mNodeInfos[cur];

		nodeInfo.mGlobalTransform = (nodeInfo.mLocalTransform);
		aiNode* iter = cur->mParent;
		while (iter) {
			nodeInfo.mGlobalTransform = mNodeInfos[iter].mLocalTransform 
									  * nodeInfo.mGlobalTransform;
			iter = iter->mParent;
		}
	}
}

void AssimpModel::PlayAnim(int Index)
{
	mCurrentAnimIndex = Index;
	mElapse = 0;
	if (mCurrentAnimIndex < 0 || mCurrentAnimIndex >= mScene->mNumAnimations) return;

	const aiAnimation* currentAnim = mScene->mAnimations[mCurrentAnimIndex];

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

void AssimpModel::DoDraw(aiNode* node, RenderOperationQueue& opList)
{
	auto& meshes = mNodeInfos[node];
	if (meshes.MeshCount() > 0) {
		cbWeightedSkin weightedSkin = {};
		weightedSkin.Model = AS_CONST_REF(Eigen::Matrix4f, mNodeInfos[node].mGlobalTransform);
		//mRenderSys.mDeviceContext->UpdateSubresource(
		//	mMaterial->CurTech()->mPasses[0]->mConstBuffers[1], 0, NULL, &weightedSkin, 0, 0);

		for (int i = 0; i < meshes.MeshCount(); i++) {
			auto mesh = meshes[i];
			if (mesh->GetAiMesh()->HasBones()) {
				const auto& boneMats = GetBoneMatrices(node, i);
				size_t boneSize = boneMats.size(); 
				//assert(boneSize <= MAX_MATRICES);
				for (int j = 0; j < min(MAX_MATRICES, boneSize); ++j)
					weightedSkin.Models[j] = AS_CONST_REF(Eigen::Matrix4f, boneMats[j]);
			}
			else {
				weightedSkin.Models[0] = Eigen::Matrix4f::Identity();
			}

			weightedSkin.hasNormal = mesh->HasTexture(kTexturePbrNormal);
			weightedSkin.hasMetalness = mesh->HasTexture(kTexturePbrMetalness);
			weightedSkin.hasRoughness = mesh->HasTexture(kTexturePbrRoughness);
			weightedSkin.hasAO = mesh->HasTexture(kTexturePbrAo);
			mesh->GetMaterial()->CurTech()->UpdateConstBufferByName(
				mResourceMng, MAKE_CBNAME(cbWeightedSkin), Data::Make(weightedSkin));

			mesh->GenRenderOperation(opList);
		}
	}

	for (int i = 0; i < node->mNumChildren; i++)
		DoDraw(node->mChildren[i], opList);
}

int AssimpModel::GenRenderOperation(RenderOperationQueue& opList)
{
	if (!mMaterial->IsLoaded())
		return 0;

	int count = opList.Count();
	DoDraw(mRootNode, opList);

	Eigen::Matrix4f world = mTransform->GetMatrix();
	for (int i = count; i < opList.Count(); ++i)
		opList[i].mWorldTransform = world;

	return opList.Count() - count;
}

}