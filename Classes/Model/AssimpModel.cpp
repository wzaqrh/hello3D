#include "AssimpModel.h"
#include "Utility.h"
#include "TRenderSystem.h"

/********** AiNodeInfo **********/
AiNodeInfo::AiNodeInfo()
{
	channelIndex = -1;
}

TMeshSharedPtr AiNodeInfo::operator[](int pos)
{
	return meshes[pos];
}

int AiNodeInfo::size() const
{
	return meshes.size();
}

void AiNodeInfo::push_back(TMeshSharedPtr mesh)
{
	meshes.push_back(mesh);
}

/********** Evaluator **********/
void Evaluator::Eval(float pTime)
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

/********** AssimpModel **********/
AssimpModel::AssimpModel(TRenderSystem* RenderSys, const char* vsName, const char* psName)
{
	mRenderSys = RenderSys;
	LoadMaterial(vsName, psName);
}

AssimpModel::~AssimpModel()
{
	delete mImporter;
}

void AssimpModel::LoadModel(const std::string& imgPath)
{
	delete mImporter;
	mImporter = new Assimp::Importer;
	mScene = mImporter->ReadFile(imgPath,
		aiProcess_Triangulate |
		aiProcess_ConvertToLeftHanded);

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

	Update(0);
}

void AssimpModel::processNode(aiNode* node, const aiScene* scene)
{
	mNodeInfos[node].mLocalTransform = node->mTransformation;
	for (int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* meshData = scene->mMeshes[node->mMeshes[i]];
		auto mesh = processMesh(meshData, scene);
		mMeshes.push_back(mesh);
		mNodeInfos[node].push_back(mesh);
	}

	for (int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene);
	}
}

TMeshSharedPtr AssimpModel::processMesh(aiMesh * mesh, const aiScene * scene)
{
	// Data to fill
	std::vector<SimpleVertex> vertices;
	std::vector<UINT> indices;
	std::vector<TextureInfo> textures;

	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
		//if (textype.empty()) textype = determineTextureType(scene, mat);
	}

	for (UINT vertexId = 0; vertexId < mesh->mNumVertices; vertexId++)
	{
		SimpleVertex vertex;
		memset(&vertex, 0, sizeof(vertex));

		vertex.Pos.x = mesh->mVertices[vertexId].x;
		vertex.Pos.y = mesh->mVertices[vertexId].y;
		vertex.Pos.z = mesh->mVertices[vertexId].z;

		if (mesh->mTextureCoords[0])
		{
			vertex.Tex.x = (float)mesh->mTextureCoords[0][vertexId].x;
			vertex.Tex.y = (float)mesh->mTextureCoords[0][vertexId].y;
		}
		vertices.push_back(vertex);
	}

	std::map<int, int> spMap;
	if (mesh->HasBones()) {
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

	for (UINT i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];

		for (UINT j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		std::vector<TextureInfo> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", scene);
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
	}

	return std::make_shared<TMesh>(mesh, vertices, indices, textures, mRenderSys);
}

std::vector<TextureInfo> AssimpModel::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName, const aiScene* scene)
{
	std::vector<TextureInfo> textures;
	for (UINT i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str; mat->GetTexture(type, i, &str);
		std::string key = str.C_Str();

		TextureInfo texInfo = TextureInfo(key, mRenderSys->GetTexByPath(key));
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

void VisitNode(aiNode* cur, std::map<const aiNode*, AiNodeInfo>& mNodeInfos, std::vector<aiNode*>& vec, Evaluator& eval) {
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
		Evaluator eval(aiAnim);

		mElapse += dt;
		eval.Eval(mElapse);

		VisitNode(mRootNode, mNodeInfos, vec, eval);
	}

	for (int i = 0; i < vec.size(); ++i) {
		aiNode* cur = vec[i];
		auto& nodeInfo = mNodeInfos[cur];

		nodeInfo.mGlobalTransform = (nodeInfo.mLocalTransform);
		aiNode* iter = cur->mParent;
		while (iter) {
			nodeInfo.mGlobalTransform = (mNodeInfos[iter].mLocalTransform * nodeInfo.mGlobalTransform);
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

void AssimpModel::LoadMaterial(const char* vsName, const char* psName)
{
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 3 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 6 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 8 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 12 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	mMaterial = mRenderSys->CreateMaterial(vsName, psName, layout, ARRAYSIZE(layout));
	mMaterial->mConstantBuffers.push_back(mRenderSys->CreateConstBuffer(sizeof(cbWeightedSkin)));
}

void AssimpModel::DoDraw(aiNode* node)
{
	auto& meshes = mNodeInfos[node];
	if (meshes.size() > 0) {
		mWeightedSkin.mModel = ToXM(mNodeInfos[node].mGlobalTransform);
		mRenderSys->mDeviceContext->UpdateSubresource(mMaterial->mConstantBuffers[1], 0, NULL, &mWeightedSkin, 0, 0);

		for (int i = 0; i < meshes.size(); i++) {
			auto mesh = meshes[i];

			if (mesh->data->HasBones()) {
				const auto& boneMats = GetBoneMatrices(node, i);
				size_t boneSize = boneMats.size(); assert(boneSize <= MAX_MATRICES);
				for (int j = 0; j < min(MAX_MATRICES, boneSize); ++j)
					mWeightedSkin.Models[j] = ToXM(boneMats[j]);
				mRenderSys->mDeviceContext->UpdateSubresource(mMaterial->mConstantBuffers[1], 0, NULL, &mWeightedSkin, 0, 0);
			}
			else {
				mWeightedSkin.Models[0] = XMMatrixIdentity();
			}
			mesh->Draw(mRenderSys);
		}
	}

	for (int i = 0; i < node->mNumChildren; i++)
		DoDraw(node->mChildren[i]);
}

void AssimpModel::Draw()
{
	DoDraw(mRootNode);
}