#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/assert.hpp>
#include "core/resource/assimp_resource.h"
#include "core/resource/material.h"
#include "core/resource/resource_manager.h"
#include "core/base/debug.h"
#include "core/base/macros.h"

namespace mir {

/********** AiSceneLoader **********/
class AiSceneLoader {
public:
	AiSceneLoader(Launch launchMode, ResourceManager& resourceMng, AiScenePtr asset)
		: mLaunchMode(launchMode), mResourceMng(resourceMng), mAsset(*asset), mResult(asset)
	{}
	~AiSceneLoader() {}
	bool ExecuteSyncPartTwo(const std::string& imgPath, const std::string& redirectResource) 
	{
		boost::filesystem::path imgFullpath = boost::filesystem::system_complete(imgPath);

		if (!redirectResource.empty()) {
			namespace boost_property_tree = boost::property_tree;
			boost_property_tree::ptree pt;
			boost_property_tree::read_json(std::stringstream(redirectResource), pt);
			mRedirectResourceDir = pt.get<std::string>("dir", "");
			mRedirectResourceExt = pt.get<std::string>("ext", "");
		}
		else {
			mRedirectResourceDir.clear();
			mRedirectResourceExt.clear();
		}

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
			mAsset.mImporter = new Assimp::Importer;
			mAsset.mScene = const_cast<Assimp::Importer*>(mAsset.mImporter)->ReadFile(
				imgFullpath.string(), ImportFlags);
		}
		return mAsset.mScene != nullptr;
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
		AiScenePtr result = nullptr;
		if (ExecuteSyncPartTwo(std::forward<T>(args)...))
			result = ExecuteSyncPart();
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
	static void ReCalculateTangents(std::vector<vbSurface, mir_allocator<vbSurface>>& surfVerts, 
		std::vector<vbSkeleton, mir_allocator<vbSkeleton>>& skeletonVerts,
		const std::vector<uint32_t>& indices) 
	{
		for (int i = 0; i < indices.size(); i += 3) {
			//vbSurface
			vbSurface& surf0 = surfVerts[indices[i + 0]];
			vbSurface& surf1 = surfVerts[indices[i + 1]];
			vbSurface& surf2 = surfVerts[indices[i + 2]];

			// Shortcuts for UVs
			Eigen::Vector2f& uv0 = surf0.Tex;
			Eigen::Vector2f& uv1 = surf1.Tex;
			Eigen::Vector2f& uv2 = surf2.Tex;

			// Edges of the triangle : postion delta
			Eigen::Vector3f deltaPos1 = surf1.Pos - surf0.Pos;
			Eigen::Vector3f deltaPos2 = surf2.Pos - surf0.Pos;

			//vbSkeleton
			vbSkeleton& skin0 = skeletonVerts[indices[i + 0]];
			vbSkeleton& skin1 = skeletonVerts[indices[i + 1]];
			vbSkeleton& skin2 = skeletonVerts[indices[i + 2]];
			Eigen::Vector2f deltaUV1 = uv1 - uv0;
			Eigen::Vector2f deltaUV2 = uv2 - uv0;
			float r = 1.0f / (deltaUV1.x() * deltaUV2.y() - deltaUV1.y() * deltaUV2.x());
			Eigen::Vector3f tangent = (deltaPos1 * deltaUV2.y() - deltaPos2 * deltaUV1.y()) * r;
			skin0.Tangent = skin0.Tangent + tangent;
			skin1.Tangent = skin1.Tangent + tangent;
			skin2.Tangent = skin2.Tangent + tangent;

			Eigen::Vector3f bitangent = (deltaPos2 * deltaUV1.x() - deltaPos1 * deltaUV2.x()) * r;
			skin0.BiTangent = skin0.BiTangent + bitangent;
			skin1.BiTangent = skin1.BiTangent + bitangent;
			skin2.BiTangent = skin2.BiTangent + bitangent;
		}
	}
	std::vector<ITexturePtr> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const aiScene* scene) {
		boost::filesystem::path redirectPathProto(mRedirectResourceDir);
		std::vector<ITexturePtr> textures;
		for (UINT i = 0; i < mat->GetTextureCount(type); i++) {
			aiString str; mat->GetTexture(type, i, &str);
			std::string key = str.C_Str();

			if (!mRedirectResourceDir.empty()) {
				boost::filesystem::path redirectPath(redirectPathProto);
				boost::filesystem::path texturePath(key);
				if (texturePath.is_relative()) texturePath = redirectPath.append(texturePath.string());
				else texturePath = redirectPath.append(texturePath.filename().string());
				
				if (!mRedirectResourceExt.empty())
					texturePath = texturePath.replace_extension(mRedirectResourceExt);

				if (!boost::filesystem::exists(texturePath))
					texturePath = texturePath.replace_extension("png");

				if (!boost::filesystem::exists(texturePath)) {
					redirectPath = redirectPathProto;
					texturePath = redirectPath.append(boost::filesystem::path(key).filename().string());

					if (!mRedirectResourceExt.empty())
						texturePath = texturePath.replace_extension(mRedirectResourceExt);

					if (!boost::filesystem::exists(texturePath))
						texturePath = texturePath.replace_extension("png");
				}

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
		std::vector<vbSurface, mir_allocator<vbSurface>> surfVerts;
		std::vector<vbSkeleton, mir_allocator<vbSkeleton>> skeletonVerts;
		std::vector<uint32_t> indices;
		TextureBySlotPtr texturesPtr = CreateInstance<TextureBySlot>();
		TextureBySlot& textures = *texturesPtr;
		textures.Resize(4);

		if (mesh->mMaterialIndex >= 0) {
			aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
			//if (textype.empty()) textype = determineTextureType(scene, mat);
		}

		for (size_t vertexId = 0; vertexId < mesh->mNumVertices; vertexId++) {
			surfVerts.emplace_back();
			vbSurface surf = surfVerts.back();
			surf.Pos = AS_CONST_REF(Eigen::Vector3f, mesh->mVertices[vertexId]);
			if (mesh->mTextureCoords[0]) {
				surf.Tex.x() = mesh->mTextureCoords[0][vertexId].x;
				surf.Tex.y() = mesh->mTextureCoords[0][vertexId].y;
			}

			skeletonVerts.emplace_back();
			vbSkeleton skin = skeletonVerts.back();
			if (mesh->mNormals) skin.Normal = AS_CONST_REF(Eigen::Vector3f, mesh->mNormals[vertexId]);
			if (mesh->mTangents) skin.Tangent = AS_CONST_REF(Eigen::Vector3f, mesh->mTangents[vertexId]);
			if (mesh->mBitangents) skin.BiTangent = AS_CONST_REF(Eigen::Vector3f, mesh->mBitangents[vertexId]);
		}

		if (mesh->HasBones()) {
			std::map<int, int> spMap;
			for (int boneId = 0; boneId < mesh->mNumBones; ++boneId) {
				aiBone* bone = mesh->mBones[boneId];
				for (int k = 0; k < bone->mNumWeights; ++k) {
					aiVertexWeight& vw = bone->mWeights[k];
					if (vw.mVertexId >= 0 && vw.mVertexId < skeletonVerts.size()) {
						int sp = spMap[vw.mVertexId];
						if (sp < 4) {
							FLOAT* BlendWeights = (FLOAT*)&skeletonVerts[vw.mVertexId].BlendWeights;
							BlendWeights[sp] = vw.mWeight;

							unsigned int* BlendIndices = (unsigned int*)&skeletonVerts[vw.mVertexId].BlendIndices;
							BlendIndices[sp] = boneId;

							spMap[vw.mVertexId]++;
						}
					}
				}
			}
		}

		for (size_t i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			for (size_t j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		if (mesh->mNormals && mesh->mTangents == nullptr) {
			ReCalculateTangents(surfVerts, skeletonVerts, indices);
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
			std::move(surfVerts), std::move(skeletonVerts), 
			std::move(indices), texturesPtr);
	}
private:
	const Launch mLaunchMode;
	ResourceManager& mResourceMng;
	AiScene& mAsset;
	AiScenePtr mResult;
private:
	std::string mRedirectResourceDir, mRedirectResourceExt;
};
typedef std::shared_ptr<AiSceneLoader> AiSceneLoaderPtr;
/********** AiAssetManager **********/

AiScenePtr AiResourceFactory::CreateAiScene(Launch launchMode, ResourceManager& resourceMng, 
	const std::string& assetPath, const std::string& redirectRes, AiScenePtr aiRes)
{
	AiScenePtr res = IF_OR(aiRes, CreateInstance<AiScene>());
	AiSceneLoaderPtr loader = CreateInstance<AiSceneLoader>(launchMode, resourceMng, res);
	if (launchMode == LaunchAsync) {
		res->SetPrepared();
		resourceMng.AddLoadResourceJob(launchMode, [=](IResourcePtr res, LoadResourceJobPtr nextJob) {
			TIME_PROFILE((boost::format("aiResFac.CreateAiScene cb %1% %2%") %assetPath %redirectRes).str());
			if (loader->ExecuteSyncPartTwo(assetPath, redirectRes) && loader->ExecuteSyncPart()) {
				return true;
			}
			else return false;
		}, res);
	}
	else {
		TIME_PROFILE((boost::format("aiResFac.CreateAiScene %1% %2%") %assetPath %redirectRes).str());
		res->SetLoaded(nullptr != loader->Execute(assetPath, redirectRes));
	}
	return res;
}

}