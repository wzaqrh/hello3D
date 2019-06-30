#include "AssimpModel.h"
#include "Utility.h"


AssimpModel::AssimpModel()
{
}


AssimpModel::~AssimpModel()
{
	delete mImporter;
}

void AssimpModel::LoadModel(ID3D11Device* dev, const std::string& imgPath)
{
	mDevice = dev;

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
}

struct Evaluator 
{
public:
	std::vector<aiMatrix4x4> mTransforms;
	const aiAnimation* mAnim;
public:
	Evaluator(const aiAnimation* Anim) :mAnim(Anim) {}
	void Eval(float pTime);
};

void Evaluator::Eval(float pTime)
{
	//pTime = 12.469;

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

		std::string name = "IK_Auge_L";
		if (name == channel->mNodeName.C_Str()) {
			channel = channel;
		}

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

void VisitNode(aiNode* cur, std::map<const aiNode*, AiNodeInfo>& mNodeInfos, std::vector<aiNode*>& vec, Evaluator& eval) {
	vec.push_back(cur);

	auto& nodeInfo = mNodeInfos[cur];
	if (nodeInfo.channelIndex >= 0 && nodeInfo.channelIndex < eval.mTransforms.size()) {
		nodeInfo.mLocalTransform = (eval.mTransforms[nodeInfo.channelIndex]);
	}
	else {
		nodeInfo.mLocalTransform = ((cur->mTransformation));
	}

	for (int i = 0; i < cur->mNumChildren; ++i) {
		VisitNode(cur->mChildren[i], mNodeInfos, vec, eval);
	}
}

void OutPutMatrix(FILE* fd, const aiMatrix4x4& m) {
	if (fd == nullptr) return;

	fprintf(fd, "%.3f %.3f %.3f %.3f\n", m.a1, m.a2, m.a3, m.a4);
	fprintf(fd, "%.3f %.3f %.3f %.3f\n", m.b1, m.b2, m.b3, m.b4);
	fprintf(fd, "%.3f %.3f %.3f %.3f\n", m.c1, m.c2, m.c3, m.c4);
	fprintf(fd, "%.3f %.3f %.3f %.3f\n", m.d1, m.d2, m.d3, m.d4);

	fprintf(fd, "\n\n");
	fflush(fd);
}

static void OutPutMatrix(FILE* fd, const XMMATRIX& m) {
	if (fd == nullptr) return;

	for (int i = 0; i < 4; ++i)
		fprintf(fd, "%.3f %.3f %.3f %.3f\n", m.m[i][0], m.m[i][1], m.m[i][2], m.m[i][3]);
	fprintf(fd, "\n\n");
	fflush(fd);
}

void AssimpModel::Update(float dt)
{
	if (mCurrentAnimIndex < 0 || mCurrentAnimIndex >= mScene->mNumAnimations) return;
	mElapse += dt;

	aiAnimation* aiAnim = mScene->mAnimations[mCurrentAnimIndex];
	Evaluator eval(aiAnim);
	eval.Eval(mElapse);

	std::vector<aiNode*> vec;
	VisitNode(mRootNode, mNodeInfos, vec, eval);

	for (int i = 0; i < vec.size(); ++i) {
		aiNode* cur = vec[i];
		auto& nodeInfo = mNodeInfos[cur];
		
		std::string name = "IK_Auge_L";
		if (name == cur->mName.C_Str()) {
			cur = cur;
		}
		static FILE* fd = fopen("D:\\Tutorial.txt", "w");

		int cnt = 0;
		if (name == cur->mName.C_Str() && fd) {
			fprintf(fd, "%s %d\n", cur->mName.C_Str(), cnt++);
			OutPutMatrix(fd, nodeInfo.mLocalTransform);
		}

		nodeInfo.mGlobalTransform = (nodeInfo.mLocalTransform);
		aiNode* iter = cur->mParent;
		while (iter) {
			nodeInfo.mGlobalTransform = (mNodeInfos[iter].mLocalTransform * nodeInfo.mGlobalTransform);
			
			if (name == cur->mName.C_Str() && fd) {
				fprintf(fd, "%s %d\n", iter->mName.C_Str(), cnt++);
				OutPutMatrix(fd, mNodeInfos[iter].mLocalTransform);
			}
			iter = iter->mParent;
		}

		cur = cur;
		if (name == cur->mName.C_Str() && fd) {
			OutPutMatrix(fd, nodeInfo.mGlobalTransform);

			fclose(fd);
			fd = nullptr;
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

void AssimpModel::processNode(aiNode* node, const aiScene* scene)
{
	mNodeInfos[node];
	for (int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* meshData = scene->mMeshes[node->mMeshes[i]];
		auto mesh = processMesh(meshData, scene);
		mMeshes.push_back(mesh);
		mNodeInfos[node].push_back(mesh);
	}

	for (int i = 0; i < node->mNumChildren; i++)
	{
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

	// Walk through each of the mesh's vertices
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

	return std::make_shared<TMesh>(mesh, mDevice, vertices, indices, textures);
}

std::vector<TextureInfo> AssimpModel::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName, const aiScene* scene)
{
	std::vector<TextureInfo> textures;
	for (UINT i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str; mat->GetTexture(type, i, &str);
		std::string key = str.C_Str();

		TextureInfo texInfo = TextureInfo(key, GetTexByPath(key));
		textures.push_back(texInfo);
		mLoadedTexture[key] = texInfo;
	}
	return textures;
}

const std::vector<aiMatrix4x4>& AssimpModel::GetBoneMatrices(const aiNode* pNode,size_t pMeshIndex)
{
	assert(pMeshIndex < pNode->mNumMeshes);
	size_t meshIndex = pNode->mMeshes[pMeshIndex];

	assert(meshIndex < mScene->mNumMeshes);
	const aiMesh* mesh = mScene->mMeshes[meshIndex];

	// resize array and initialize it with identity matrices
	mTransforms.resize(mesh->mNumBones, aiMatrix4x4());

	// calculate the mesh's inverse global transform
	aiMatrix4x4 globalInverseMeshTransform = mNodeInfos[pNode].mGlobalTransform;
	globalInverseMeshTransform.Inverse();

	// Bone matrices transform from mesh coordinates in bind pose to mesh coordinates in skinned pose
	// Therefore the formula is offsetMatrix * currentGlobalTransform * inverseCurrentMeshTransform
	for (size_t a = 0; a < mesh->mNumBones; ++a) {
		static FILE* fd = fopen("D:\\Tutorial_Bone.txt", "w");

		const aiBone* bone = mesh->mBones[a];
		const aiNode* boneNode = mBoneNodesByName[bone->mName.data];
		auto& boneInfo = mNodeInfos[boneNode];
		aiMatrix4x4 currentGlobalTransform = boneInfo.mGlobalTransform;
		mTransforms[a] = globalInverseMeshTransform * currentGlobalTransform * bone->mOffsetMatrix;

		if (fd) fprintf(fd, "%s\n", pNode->mName.C_Str());
		OutPutMatrix(fd, globalInverseMeshTransform);
		if (fd) fprintf(fd, "%s\n", bone->mName.data);
		OutPutMatrix(fd, currentGlobalTransform);
		OutPutMatrix(fd, bone->mOffsetMatrix);
		
		if (fd) fclose(fd);
		fd = nullptr;
	}

	// and return the result
	return mTransforms;
}


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

//XMMATRIX AiNodeInfo::localTransform() const
//{
//	return ToXM(mLocalTransform);
//}


//void AiNodeInfo::setLocalTransform(const XMMATRIX& m)
//{
//	mLocalTransform = FromXM(m);
//}
//
//
//const aiMatrix4x4& AiNodeInfo::globalTransformAI() const
//{
//	return mGlobalTransform;
//}

//XMMATRIX AiNodeInfo::globalTransform() const
//{
//	return ToXM(mGlobalTransform);
//}

//void AiNodeInfo::setGlobalTransform(const XMMATRIX& m)
//{
//	mGlobalTransform = FromXM(m);
//}

XMMATRIX ToXM(const aiMatrix4x4& m) {
	XMMATRIX r;
	static_assert(sizeof(r) == sizeof(m), "");
	aiMatrix4x4 mm = m;
	//mm.Transpose();
	memcpy(&r, &mm, sizeof(mm));
	return r;
}

aiMatrix4x4 FromXM(const XMMATRIX& m) {
	aiMatrix4x4 r;
	static_assert(sizeof(r) == sizeof(m), "");
	memcpy(&r, &m, sizeof(m));
	r.Transpose();
	return r;
}
