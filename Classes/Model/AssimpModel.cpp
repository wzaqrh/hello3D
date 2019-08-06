#include "AssimpModel.h"
#include "TMaterialCB.h"
#include "Utility.h"
#include "TRenderSystem.h"
#include "TInterfaceType.h"
#include "Lesson6.h"

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
AssimpModel::AssimpModel(TRenderSystem* RenderSys, TMovablePtr pMove, const char* vsName, const char* psName, std::function<void(TMaterialPtr)> cb)
{
	mMove = pMove ? pMove : std::make_shared<TMovable>();
	mRenderSys = RenderSys;
	LoadMaterial(vsName, psName, cb);
}

AssimpModel::AssimpModel(TRenderSystem* RenderSys, TMovablePtr pMove, const std::string& matType, std::function<void(TMaterialPtr)> cb /*= nullptr*/)
{
	mMove = pMove ? pMove : std::make_shared<TMovable>();
	mRenderSys = RenderSys;
	LoadMaterial(matType, cb);
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

void AssimpModel::LoadModel(const std::string& imgPath)
{
	TIME_PROFILE(AssimpModel_LoadModel);

	{
		TIME_PROFILE(Assimp_Importer);
		delete mImporter;
		mImporter = new Assimp::Importer;
		mScene = mImporter->ReadFile(imgPath, ImportFlags);
	}

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
		mNodeInfos[node].push_back(mesh);
	}

	for (int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene);
	}
}

void ReCalculateTangents(std::vector<MeshVertex>& vertices, const std::vector<UINT>& indices) {
	for (int i = 0; i < indices.size(); i += 3) {
		// Shortcuts for vertices
		MeshVertex& v0 = vertices[indices[i+0]];
		MeshVertex& v1 = vertices[indices[i+1]];
		MeshVertex& v2 = vertices[indices[i+2]];

		// Shortcuts for UVs
		XMFLOAT2& uv0 = v0.Tex;
		XMFLOAT2& uv1 = v1.Tex;
		XMFLOAT2& uv2 = v2.Tex;

		// Edges of the triangle : postion delta
		XMFLOAT3 deltaPos1 = v1.Pos - v0.Pos;
		XMFLOAT3 deltaPos2 = v2.Pos - v0.Pos;

		// UV delta
		XMFLOAT2 deltaUV1 = uv1 - uv0;
		XMFLOAT2 deltaUV2 = uv2 - uv0;

		float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
		XMFLOAT3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
		v0.Tangent = v0.Tangent + tangent;
		v1.Tangent = v1.Tangent + tangent;
		v2.Tangent = v2.Tangent + tangent;

		XMFLOAT3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;
		v0.BiTangent = v0.BiTangent + bitangent;
		v1.BiTangent = v1.BiTangent + bitangent;
		v2.BiTangent = v2.BiTangent + bitangent;
	}
}

TMeshSharedPtr AssimpModel::processMesh(aiMesh * mesh, const aiScene * scene)
{
	// Data to fill
	std::vector<MeshVertex> vertices;
	std::vector<UINT> indices;
	TTextureBySlotPtr texturesPtr = std::make_shared<TTextureBySlot>();
	TTextureBySlot& textures = *texturesPtr;
	textures.resize(4);

	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
		//if (textype.empty()) textype = determineTextureType(scene, mat);
	}

	for (UINT vertexId = 0; vertexId < mesh->mNumVertices; vertexId++)
	{
		MeshVertex vertex;
		memset(&vertex, 0, sizeof(vertex));
		vertex.Pos = ToXM(mesh->mVertices[vertexId]);
		if (mesh->mTextureCoords[0]) {
			vertex.Tex.x = (float)mesh->mTextureCoords[0][vertexId].x;
			vertex.Tex.y = (float)mesh->mTextureCoords[0][vertexId].y;
		}
		if (mesh->mNormals) {
			vertex.Normal = ToXM(mesh->mNormals[vertexId]);
		}
		if (mesh->mTangents) {
			vertex.Tangent = ToXM(mesh->mTangents[vertexId]);
		}
		if (mesh->mBitangents) {
			vertex.BiTangent = ToXM(mesh->mBitangents[vertexId]);
		}
		vertices.push_back(vertex);
	}

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

		std::vector<TTexturePtr> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, scene);
		if (diffuseMaps.size() > 0) 
			textures[E_TEXTURE_DIFFUSE] = diffuseMaps[0];

		std::vector<TTexturePtr> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, scene);
		if (specularMaps.size() > 0) 
			textures[E_TEXTURE_SPECULAR] = specularMaps[0];

		std::vector<TTexturePtr> normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, scene);
		if (normalMaps.size() > 0) 
			textures[E_TEXTURE_NORMAL] = normalMaps[0];
	}

	auto material = mMaterial->Clone(mRenderSys);
	if (mMatCb) mMatCb(material);
	return std::make_shared<TMesh>(mesh, vertices, indices, texturesPtr, material, mRenderSys);
}

std::vector<TTexturePtr> AssimpModel::loadMaterialTextures(aiMaterial* mat, aiTextureType type, const aiScene* scene)
{
	std::vector<TTexturePtr> textures;
	for (UINT i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str; mat->GetTexture(type, i, &str);
		std::string key = str.C_Str();

		TTexturePtr texInfo = mRenderSys->GetTexByPath(key);
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

void AssimpModel::LoadMaterial(const char* vsName, const char* psName, std::function<void(TMaterialPtr)> cb)
{
	std::function<void(TMaterialPtr)> callback;
	if (vsName && psName) {
		callback = [&](TMaterialPtr mat) {
			TMaterialBuilder builder(mat);
			auto program = builder.SetProgram(mRenderSys->CreateProgram(vsName, psName, nullptr, nullptr));
			D3D11_INPUT_ELEMENT_DESC layout[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 3 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, 6 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 9 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 11 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 15 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 2, DXGI_FORMAT_R32G32B32_FLOAT, 0, 19 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};
			builder.SetInputLayout(mRenderSys->CreateLayout(program, layout, ARRAYSIZE(layout)));
		};
	}
	mMaterial = mRenderSys->CreateMaterial(E_MAT_MODEL_PBR, callback);
	mMatCb = cb;
}

void AssimpModel::LoadMaterial(const std::string& matType, std::function<void(TMaterialPtr)> cb)
{
	mMaterial = mRenderSys->CreateMaterial(matType, nullptr);
	mMatCb = cb;
}

void AssimpModel::DoDraw(aiNode* node, TRenderOperationQueue& opList)
{
	auto& meshes = mNodeInfos[node];
	if (meshes.size() > 0) {
		cbWeightedSkin weightedSkin = {};
		weightedSkin.mModel = ToXM(mNodeInfos[node].mGlobalTransform);
		//mRenderSys->mDeviceContext->UpdateSubresource(mMaterial->CurTech()->mPasses[0]->mConstBuffers[1], 0, NULL, &weightedSkin, 0, 0);

		for (int i = 0; i < meshes.size(); i++) {
			auto mesh = meshes[i];
			if (mesh->data->HasBones()) {
				const auto& boneMats = GetBoneMatrices(node, i);
				size_t boneSize = boneMats.size(); assert(boneSize <= MAX_MATRICES);
				for (int j = 0; j < min(MAX_MATRICES, boneSize); ++j)
					weightedSkin.Models[j] = ToXM(boneMats[j]);
			}
			else {
				weightedSkin.Models[0] = XMMatrixIdentity();
			}

			weightedSkin.hasNormal = mesh->HasTexture(E_TEXTURE_PBR_NORMAL);
			weightedSkin.hasMetalness = mesh->HasTexture(E_TEXTURE_PBR_METALNESS);
			weightedSkin.hasRoughness = mesh->HasTexture(E_TEXTURE_PBR_ROUGHNESS);
			weightedSkin.hasAO = mesh->HasTexture(E_TEXTURE_PBR_AO);
			mesh->mMaterial->CurTech()->UpdateConstBufferByName(mRenderSys, MAKE_CBNAME(cbWeightedSkin), &weightedSkin);

#ifdef USE_RENDER_OP
			TRenderOperation op = {};
			op.mMaterial = mesh->mMaterial;
			op.mIndexBuffer = mesh->mIndexBuffer;
			op.mVertexBuffer = mesh->mVertexBuffer;
			op.mTextures = *mesh->mTextures;
			opList.push_back(op);
#else
//#define DEBUG_UNITY_PBR 1
#if DEBUG_UNITY_PBR
			if (mesh->indices.size() == 18258) {
				cbUnityMaterial cb;
				//cb._Color = XMFLOAT4(1, 0, 0, 0);
				//cb._SpecLightOff = 1;
				//cb._OcclusionStrength = 0;
				//cb._GlossMapScale = 0;
				mRenderSys->UpdateConstBuffer(mesh->mMaterial->CurTech()->mPasses[0]->mConstantBuffers[2], &cb);
			}
			else {
				cbUnityMaterial cb;
				//cb._Color = XMFLOAT4(0, 0, 0, 0);
				//cb._SpecLightOff = 1;
				//cb._OcclusionStrength = 0;
				//cb._GlossMapScale = 0;
				mRenderSys->UpdateConstBuffer(mesh->mMaterial->CurTech()->mPasses[0]->mConstantBuffers[2], &cb);
			}
#endif
			mesh->Draw(mRenderSys);
#endif
		}
	}

	for (int i = 0; i < node->mNumChildren; i++)
		DoDraw(node->mChildren[i], opList);
}

int AssimpModel::GenRenderOperation(TRenderOperationQueue& opList)
{
	int count = opList.size();
	mDrawCount = 0;
	DoDraw(mRootNode, opList);

	XMMATRIX world = mMove->GetWorldTransform();
	for (int i = count; i < opList.size(); ++i) {
		opList[i].mWorldTransform = world;
	}

	return opList.size() - count;
}

void AssimpModel::Draw()
{
	mRenderSys->Draw(this);
}
