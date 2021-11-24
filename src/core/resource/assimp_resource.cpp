#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/assert.hpp>
#include "core/base/debug.h"
#include "core/rendersys/interface_type.h"
#include "core/resource/material.h"
#include "core/resource/assimp_resource.h"
#include "core/resource/resource_manager.h"

namespace mir {

#define AS_CONST_REF(TYPE, V) *(const TYPE*)(&V)

/********** AiSceneLoader **********/
class AiSceneLoader {
public:
	AiSceneLoader(Launch launchMode, ResourceManager& resourceMng, 
		MaterialPtr material, AiScenePtr asset)
		: mLaunchMode(launchMode), mResourceMng(resourceMng), mMaterial(material)
		, mAsset(*asset), mResult(asset)
	{}
	~AiSceneLoader() {}
	void ExecuteAsyncPart(const std::string& imgPath, const std::string& redirectResource) 
	{
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
			constexpr uint32_t ImportFlags =
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
			TIME_PROFILE(Assimp_Importer);
			mAsset.mImporter = new Assimp::Importer;
			mAsset.mScene = const_cast<Assimp::Importer*>(mAsset.mImporter)->ReadFile(
				imgFullpath.string(), ImportFlags);
		}
	}
	AiScenePtr ExecuteSyncPart() 
	{
		mAsset.mRootNode = mAsset.AddNode(mAsset.mScene->mRootNode);
		processNode(mAsset.mRootNode, mAsset.mScene);

		BOOST_ASSERT(mAsset.mScene != nullptr);
		for (unsigned int i = 0; i < mAsset.mScene->mNumMeshes; ++i) {
			const aiMesh* mesh = mAsset.mScene->mMeshes[i];
			for (unsigned int n = 0; n < mesh->mNumBones; ++n) {
				const aiBone* bone = mesh->mBones[n];
				BOOST_ASSERT(mAsset.mBoneNodesByName[bone->mName.data] == nullptr
					|| mAsset.mBoneNodesByName[bone->mName.data]->RawNode == mAsset.mScene->mRootNode->FindNode(bone->mName));
				//mAsset.mBoneNodesByName[bone->mName.data] = mAsset.mScene->mRootNode->FindNode(bone->mName);
				auto findRawNode = mAsset.mScene->mRootNode->FindNode(bone->mName);
				auto findIter = std::find_if(mAsset.mNodeBySerializeIndex.begin(), mAsset.mNodeBySerializeIndex.end(), [&findRawNode](const AiNodePtr& nnode) {
					return nnode->RawNode == findRawNode;
				});
				if (findIter != mAsset.mNodeBySerializeIndex.end())
					mAsset.mBoneNodesByName[bone->mName.data] = *findIter;
				else
					mAsset.mBoneNodesByName[bone->mName.data] = nullptr;
			}
		}
		return mResult;
	}
	TemplateArgs AiScenePtr Execute(T &&...args) {
		ExecuteAsyncPart(std::forward<T>(args)...);
		AiScenePtr result = ExecuteSyncPart();
		if (result) result->SetLoaded();
		return result;
	}
	TemplateArgs AiScenePtr operator()(T &&...args) {
		return Execute(std::forward<T>(args)...);
	}
private:
	void processNode(const AiNodePtr& node, const aiScene* rawScene) {
		const aiNode* rawNode = node->RawNode;
		for (int i = 0; i < rawNode->mNumMeshes; i++) {
			aiMesh* rawMesh = rawScene->mMeshes[rawNode->mMeshes[i]];
			node->AddMesh(processMesh(rawMesh, rawScene));
		}

		for (int i = 0; i < rawNode->mNumChildren; i++) {
			AiNodePtr child = mAsset.AddNode(rawNode->mChildren[i]);
			node->AddChild(child);
			processNode(child, rawScene);
		}
	}
	static void ReCalculateTangents(std::vector<AssimpMeshVertex>& vertices, const std::vector<uint32_t>& indices) {
		for (int i = 0; i < indices.size(); i += 3) {
			// Shortcuts for vertices
			AssimpMeshVertex& v0 = vertices[indices[i + 0]];
			AssimpMeshVertex& v1 = vertices[indices[i + 1]];
			AssimpMeshVertex& v2 = vertices[indices[i + 2]];

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
	std::vector<ITexturePtr> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const aiScene* scene) {
		boost::filesystem::path redirectPath(mRedirectResourceDir);
		std::vector<ITexturePtr> textures;
		for (UINT i = 0; i < mat->GetTextureCount(type); i++) {
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

			ITexturePtr texInfo = mResourceMng.CreateTextureByFile(mLaunchMode, key);
			textures.push_back(texInfo);
			mAsset.mLoadedTexture[key] = texInfo;
		}
		return textures;
	}
	AssimpMeshPtr processMesh(const aiMesh* mesh, const aiScene* scene) {
		// Data to fill
		std::vector<AssimpMeshVertex> vertices;
		std::vector<uint32_t> indices;
		TextureBySlotPtr texturesPtr = std::make_shared<TextureBySlot>();
		TextureBySlot& textures = *texturesPtr;
		textures.Resize(4);

		if (mesh->mMaterialIndex >= 0) {
			aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
			//if (textype.empty()) textype = determineTextureType(scene, mat);
		}

		for (size_t vertexId = 0; vertexId < mesh->mNumVertices; vertexId++) {
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

		for (UINT i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			for (UINT j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		if (mesh->mNormals && mesh->mTangents == nullptr) {
			ReCalculateTangents(vertices, indices);
		}

		if (mesh->mMaterialIndex >= 0) {
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

		return AssimpMesh::Create(mLaunchMode, mResourceMng, mesh, 
			std::move(vertices), std::move(indices), texturesPtr);
	}
private:
	const Launch mLaunchMode;
	ResourceManager& mResourceMng;
	MaterialPtr mMaterial;
	AiScene& mAsset;
	AiScenePtr mResult;
private:
	std::string mRedirectResourceDir, mRedirectResourceExt;
};
typedef std::shared_ptr<AiSceneLoader> AiSceneLoaderPtr;
/********** AiAssetManager **********/

AiScenePtr AiResourceFactory::CreateAiScene(Launch launchMode, ResourceManager& resourceMng, 
	MaterialPtr material, const std::string& assetPath, const std::string& redirectRes)
{
	TIME_PROFILE(AiResourceFactory_CreateAiScene);
	AiScenePtr res = std::make_shared<AiScene>();

	AiSceneLoaderPtr loader = std::make_shared<AiSceneLoader>(launchMode, resourceMng, material, res);
	if (launchMode == Launch::Async) {
		res->SetPrepared();
		resourceMng.AddLoadResourceJobAsync([=](IResourcePtr res, LoadResourceJobPtr nextJob) {
			loader->ExecuteAsyncPart(assetPath, redirectRes);
			nextJob->InitSync([=](IResourcePtr res, LoadResourceJobPtr nullJob) {
				loader->ExecuteSyncPart();
				return true;
			});
			return true;
		}, res);
		return res;
	}
	else {
		return loader->Execute(assetPath, redirectRes);
	}
}

}