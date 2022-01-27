#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/assert.hpp>
#include "core/resource/assimp_resource.h"
#include "core/resource/material.h"
#include "core/resource/resource_manager.h"
#include "core/base/debug.h"
#include "core/base/macros.h"
#include <unordered_map>

namespace mir {
namespace res {

/********** AiSceneLoader **********/
class AiSceneLoader {
public:
	AiSceneLoader(Launch launchMode, ResourceManager& resMng, AiScenePtr asset)
		: mLaunchMode(launchMode), mResourceMng(resMng), mAsset(*asset), mResult(asset) 
	{}
	~AiSceneLoader() 
	{}
	
	TemplateArgs cppcoro::shared_task<bool> Execute(T &&...args) {
		mAsset.SetLoading();
		co_await mResourceMng.SwitchToLaunchService(__LaunchAsync__);

		mAsset.SetLoaded(ExecuteLoadRawData(std::forward<T>(args)...) && co_await ExecuteSetupData());
		return mAsset.IsLoaded();
	}
	TemplateArgs cppcoro::shared_task<bool> operator()(T &&...args) {
		return co_await Execute(std::forward<T>(args)...);
	}
private:
	bool ExecuteLoadRawData(const std::string& imgPath, const std::string& redirectResource)
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

		try
		{
		#define IMPORT_LEFTHAND
			constexpr uint32_t ImportFlags =
			#if defined IMPORT_LEFTHAND
				aiProcess_ConvertToLeftHanded |
			#endif
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
			mAsset.mScene = const_cast<Assimp::Importer*>(mAsset.mImporter)->ReadFile(imgFullpath.string(), ImportFlags);
		}
		catch (...)
		{
			DEBUG_LOG_ERROR("Assimp::Importer ReadFile error");
		}
		return mAsset.mScene != nullptr;
	}
	cppcoro::shared_task<bool> ExecuteSetupData()
	{
		mAsset.mRootNode = mAsset.AddNode(mAsset.mScene->mRootNode);
		if (!co_await ProcessNode(mAsset.mRootNode, mAsset.mScene))
			return false;

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
		return true;
	}
private:
	cppcoro::shared_task<bool> ProcessNode(const AiNodePtr& node, const aiScene* rawScene) {
		COROUTINE_VARIABLES_2(node, rawScene);

		const aiNode* rawNode = node->RawNode;
		for (int i = 0; i < rawNode->mNumMeshes; i++) {
			aiMesh* rawMesh = rawScene->mMeshes[rawNode->mMeshes[i]];
			AssimpMeshPtr mesh = co_await ProcessMesh(rawMesh, rawScene);
			if (mesh == nullptr) 
				return false;
			node->AddMesh(mesh);
		}

		for (int i = 0; i < rawNode->mNumChildren; i++) {
			AiNodePtr child = mAsset.AddNode(rawNode->mChildren[i]);
			node->AddChild(child);
			if (!co_await ProcessNode(child, rawScene))
				return false;
		}
		return true;
	}
	
	static void ReCalculateTangents(vbSurfaceVector& surfVerts, vbSkeletonVector& skeletonVerts, const std::vector<uint32_t>& indices) 
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
	boost::filesystem::path RedirectPathOnDir(const boost::filesystem::path& path, bool forceNotRelative = false) const {
		boost::filesystem::path result(mRedirectResourceDir);
		if (!path.is_relative() || forceNotRelative) return result.append(path.filename().string());
		else return result.append(path.string());
	}
#if !USE_MATERIAL_INSTANCE
	std::vector<ITexturePtr> loadMaterialTextures(const aiMaterial* mat, aiTextureType type, const aiScene* scene) {
		boost::filesystem::path redirectPathProto(mRedirectResourceDir);
		std::vector<ITexturePtr> textures;
		for (UINT i = 0; i < std::max<int>(1, mat->GetTextureCount(type)); i++)
		{
			aiString str;
		#if 0
			if (aiReturn_FAILURE == mat->GetTexture(type, i, &str))
				continue;
		#else
			if (aiReturn_FAILURE == mat->GetTexture(type, i, &str)) {
				switch (type)
				{
				case aiTextureType_DIFFUSE:
				case aiTextureType_BASE_COLOR:
					str.Set("Default_AO");
					break;
				case aiTextureType_NORMALS:
				case aiTextureType_NORMAL_CAMERA:
					str.Set("Default_normal");
					break;
				case aiTextureType_SPECULAR:
					str.Set("Default_specular");
					break;
				case aiTextureType_METALNESS:
					str.Set("Default_matalness");
					break;
				case aiTextureType_EMISSIVE:
				case aiTextureType_EMISSION_COLOR:
					str.Set("Default_emissive");
					break;
				case aiTextureType_DIFFUSE_ROUGHNESS:
					str.Set("Default_diffuse_roughness");
					break;
				default:
					break;
				}
			}
		#endif
			std::string key = str.C_Str();

			if (!mRedirectResourceDir.empty()) {
				boost::filesystem::path texturePath = RedirectPathOnDir(boost::filesystem::path(key));
				if (!mRedirectResourceExt.empty()) texturePath = texturePath.replace_extension(mRedirectResourceExt);
				if (!boost::filesystem::exists(texturePath)) texturePath = texturePath.replace_extension("png");

				if (!boost::filesystem::exists(texturePath)) {
					texturePath = RedirectPathOnDir(boost::filesystem::path(key));
					if (!mRedirectResourceExt.empty()) texturePath = texturePath.replace_extension(mRedirectResourceExt);
					if (!boost::filesystem::exists(texturePath)) texturePath = texturePath.replace_extension("png");
				}

				key = texturePath.string();
			}

			if (!boost::filesystem::is_regular_file(key))
				continue;

			ITexturePtr texInfo = mResourceMng.CreateTextureByFile(mLaunchMode, key);
			textures.push_back(texInfo);
			mAsset.mLoadedTexture[key] = texInfo;
		}
		return textures;
	}
#endif
	cppcoro::shared_task<AssimpMeshPtr> ProcessMesh(const aiMesh* rawMesh, const aiScene* scene) {
		COROUTINE_VARIABLES_2(rawMesh, scene);

		AssimpMeshPtr meshPtr = AssimpMesh::Create();
		auto& mesh = *meshPtr;
		mesh.mAiMesh = rawMesh;

	#if USE_MATERIAL_INSTANCE
		boost::filesystem::path matPath = RedirectPathOnDir(boost::filesystem::path(std::string(rawMesh->mName.C_Str()) + ".Material"));
		std::string loadParam = boost::filesystem::is_regular_file(matPath) ? matPath.string() : MAT_MODEL;
		mesh.mMaterial = co_await mResourceMng.CreateMaterial(mLaunchMode, loadParam);
	#else
		mesh.mUvTransform.assign(kTexturePbrMax, Eigen::Vector4f(0, 0, 1, 1));
		mesh.mFactors.assign(kTexturePbrMax, Eigen::Vector4f::Ones());
		mesh.mFactors[kTexturePbrAo] = Eigen::Vector4f::Zero();
		mesh.mTextures.Resize(kTexturePbrMax);
		if (rawMesh->mMaterialIndex >= 0) {
			TextureVector& textures = mesh.mTextures;
			const aiMaterial* material = scene->mMaterials[rawMesh->mMaterialIndex];

			auto loadTexture = [&](size_t pos, aiTextureType type) {
				std::vector<ITexturePtr> loads = loadMaterialTextures(material, type, scene);
				if (loads.size() > 0)
					textures[pos] = loads[0];
			};
			loadTexture(kTextureDiffuse, aiTextureType_DIFFUSE);
			loadTexture(kTextureNormal, aiTextureType_NORMALS);
			loadTexture(kTextureSpecular, aiTextureType_SPECULAR);

			loadTexture(kTexturePbrAlbedo, aiTextureType_BASE_COLOR);
			loadTexture(kTexturePbrNormal, aiTextureType_NORMAL_CAMERA);
			loadTexture(kTexturePbrMetalness, aiTextureType_METALNESS);
			loadTexture(kTexturePbrRoughness, aiTextureType_DIFFUSE_ROUGHNESS);
			loadTexture(kTexturePbrAo, aiTextureType_AMBIENT_OCCLUSION);
			loadTexture(kTexturePbrEmissive, aiTextureType_EMISSIVE);

			loadTexture(kTexturePbrAo, aiTextureType_UNKNOWN);

			unsigned int channel = 4;
			material->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_FACTOR, (ai_real*)&mesh.mFactors[kTexturePbrAlbedo], &channel);
			channel = 4; material->Get("$tex.scale", aiTextureType_NORMALS, 0, mesh.mFactors[kTexturePbrNormal].x());
			channel = 4; material->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR, (ai_real*)&mesh.mFactors[kTexturePbrMetalness], &channel);
			channel = 4; material->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR, (ai_real*)&mesh.mFactors[kTexturePbrRoughness], &channel);
			channel = 4; material->Get("$tex.strength", aiTextureType_LIGHTMAP, 0, mesh.mFactors[kTexturePbrAo].x());
			channel = 4; material->Get(AI_MATKEY_COLOR_EMISSIVE, (ai_real*)&mesh.mFactors[kTexturePbrEmissive], &channel);

			channel = 4; material->Get(_AI_MATKEY_UVTRANSFORM_BASE, aiTextureType_BASE_COLOR, 0, (ai_real*)&mesh.mUvTransform[kTexturePbrAlbedo], &channel);
			channel = 4; material->Get(_AI_MATKEY_UVTRANSFORM_BASE, aiTextureType_NORMALS, 0, (ai_real*)&mesh.mUvTransform[kTexturePbrNormal], &channel);
			channel = 4; material->Get(_AI_MATKEY_UVTRANSFORM_BASE, aiTextureType_METALNESS, 0, (ai_real*)&mesh.mUvTransform[kTexturePbrMetalness], &channel);
			channel = 4; material->Get(_AI_MATKEY_UVTRANSFORM_BASE, aiTextureType_DIFFUSE_ROUGHNESS, 0, (ai_real*)&mesh.mUvTransform[kTexturePbrRoughness], &channel);
			channel = 4; material->Get(_AI_MATKEY_UVTRANSFORM_BASE, aiTextureType_AMBIENT_OCCLUSION, 0, (ai_real*)&mesh.mUvTransform[kTexturePbrAo], &channel);
			channel = 4; material->Get(_AI_MATKEY_UVTRANSFORM_BASE, aiTextureType_EMISSION_COLOR, 0, (ai_real*)&mesh.mUvTransform[kTexturePbrEmissive], &channel);
		}
		mesh.mHasTangent = (rawMesh->mTangents != nullptr);
	#endif

	#define VEC_ASSIGN(DST, SRC) memcpy(DST.data(), &SRC, sizeof(SRC))
	#define VEC_ASSIGN1(DST, SRC, SIZE) memcpy(DST.data(), &SRC, SIZE)
		std::vector<vbSurface, mir_allocator<vbSurface>>& surfVerts(mesh.mSurfVertexs); surfVerts.resize(rawMesh->mNumVertices);
		std::vector<vbSkeleton, mir_allocator<vbSkeleton>>& skeletonVerts(mesh.mSkeletonVertexs); skeletonVerts.resize(rawMesh->mNumVertices);
		for (size_t vertexId = 0; vertexId < rawMesh->mNumVertices; vertexId++) {
		#if !defined EIGEN_DONT_ALIGN_STATICALLY
			surfVerts[vertexId].Pos = AS_CONST_REF(Eigen::Vector3f, rawMesh->mVertices[vertexId]);
		#else
			VEC_ASSIGN(surfVerts[vertexId].Pos, rawMesh->mVertices[vertexId]);
		#endif
		}
		if (rawMesh->mTextureCoords[0]) {
			const auto& meshTexCoord0 = rawMesh->mTextureCoords[0];
			for (size_t vertexId = 0; vertexId < rawMesh->mNumVertices; vertexId++) {
			#if !defined EIGEN_DONT_ALIGN_STATICALLY
				surfVerts[vertexId].Tex.x() = meshTexCoord0[vertexId].x;
				surfVerts[vertexId].Tex.y() = meshTexCoord0[vertexId].y;
			#else
				VEC_ASSIGN1(surfVerts[vertexId].Tex, meshTexCoord0[vertexId], sizeof(Eigen::Vector2f));
			#endif
			}
		}
		if (rawMesh->mNormals) {
			for (size_t vertexId = 0; vertexId < rawMesh->mNumVertices; vertexId++) {
			#if !defined IMPORT_LEFTHAND
				skeletonVerts[vertexId].Normal.x() = rawMesh->mNormals[vertexId].x;
				skeletonVerts[vertexId].Normal.y() = rawMesh->mNormals[vertexId].y;
				skeletonVerts[vertexId].Normal.z() = -rawMesh->mNormals[vertexId].z;
			#else
			#if !defined EIGEN_DONT_ALIGN_STATICALLY
				skeletonVerts[vertexId].Normal = AS_CONST_REF(Eigen::Vector3f, rawMesh->mNormals[vertexId]);
			#else
				VEC_ASSIGN(skeletonVerts[vertexId].Normal, rawMesh->mNormals[vertexId]);
			#endif
			#endif
			}
		}
		if (rawMesh->mTangents) {
			for (size_t vertexId = 0; vertexId < rawMesh->mNumVertices; vertexId++) {
			#if !defined IMPORT_LEFTHAND
				skeletonVerts[vertexId].Tangent.x() = rawMesh->mTangents[vertexId].x;
				skeletonVerts[vertexId].Tangent.y() = rawMesh->mTangents[vertexId].y;
				skeletonVerts[vertexId].Tangent.z() = -rawMesh->mTangents[vertexId].z;
			#else
			#if !defined EIGEN_DONT_ALIGN_STATICALLY
				skeletonVerts[vertexId].Tangent = AS_CONST_REF(Eigen::Vector3f, rawMesh->mTangents[vertexId]);
			#else
				VEC_ASSIGN(skeletonVerts[vertexId].Tangent, rawMesh->mTangents[vertexId]);
			#endif
			#endif
			}
		}

		if (rawMesh->mBitangents) {
			for (size_t vertexId = 0; vertexId < rawMesh->mNumVertices; vertexId++) {
			#if !defined IMPORT_LEFTHAND
				skeletonVerts[vertexId].BiTangent.x() = rawMesh->mBitangents[vertexId].x;
				skeletonVerts[vertexId].BiTangent.y() = rawMesh->mBitangents[vertexId].y;
				skeletonVerts[vertexId].BiTangent.z() = -rawMesh->mBitangents[vertexId].z;
			#else
			#if !defined EIGEN_DONT_ALIGN_STATICALLY
				skeletonVerts[vertexId].BiTangent = AS_CONST_REF(Eigen::Vector3f, rawMesh->mBitangents[vertexId]);
			#else
				VEC_ASSIGN(skeletonVerts[vertexId].BiTangent, rawMesh->mBitangents[vertexId]);
			#endif
			#endif
			}
		}

		if (rawMesh->HasBones()) {
			std::vector<int> spMap(skeletonVerts.size(), 0);
			for (size_t boneId = 0; boneId < rawMesh->mNumBones; ++boneId) {
				const aiBone* bone = rawMesh->mBones[boneId];
				if (bone->mWeights == nullptr)
					continue;
				for (size_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {
					const aiVertexWeight& vw = bone->mWeights[weightIndex];
					if (vw.mVertexId >= 0 && vw.mVertexId < skeletonVerts.size()) {
						int sp = spMap[vw.mVertexId];
						if (sp < 4) {
							float* bwArray = (float*)&skeletonVerts[vw.mVertexId].BlendWeights;
							bwArray[sp] = vw.mWeight;

							unsigned* biArray = (unsigned*)&skeletonVerts[vw.mVertexId].BlendIndices;
							biArray[sp] = boneId;

							spMap[vw.mVertexId]++;
						}
					}
				}
			}
		}

		std::vector<uint32_t>& indices(mesh.mIndices);
		for (size_t i = 0; i < rawMesh->mNumFaces; i++) {
			const aiFace& face = rawMesh->mFaces[i];
			size_t position = indices.size();
			indices.resize(position + face.mNumIndices);
			memcpy(&indices[position], face.mIndices, face.mNumIndices * sizeof(unsigned int));
		#if !defined IMPORT_LEFTHAND
			for (size_t j = position; j + 3 < position + face.mNumIndices; j += 3)
				std::swap(indices[j + 1], indices[j + 2]);
		#endif
		}

		mesh.Build(mLaunchMode, mResourceMng);
		return meshPtr;
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

cppcoro::shared_task<AiScenePtr> AiResourceFactory::CreateAiScene(Launch launchMode, ResourceManager& resMng, const std::string& assetPath, const std::string& redirectRes, AiScenePtr aiRes)
{
	COROUTINE_VARIABLES_5(launchMode, resMng, assetPath, redirectRes, aiRes);
	//co_await mResourceMng.SwitchToLaunchService(launchMode)

	AiScenePtr scene = IF_OR(aiRes, CreateInstance<AiScene>());
	AiSceneLoaderPtr loader = CreateInstance<AiSceneLoader>(launchMode, resMng, scene);
#if USE_COROUTINE
	co_await loader->Execute(assetPath, redirectRes);
#else
	if (launchMode == LaunchAsync) {
		scene->SetPrepared();
		resMng.AddLoadResourceJob(launchMode, [=](IResourcePtr res, LoadResourceJobPtr nextJob) {
			TIME_PROFILE((boost::format("aiResFac.CreateAiScene cb %1% %2%") % assetPath %redirectRes).str());
			if (loader->ExecuteLoadRawData(assetPath, redirectRes) && loader->ExecuteSetupData()) {
				return true;
			}
			else {
				return false;
			}
		}, scene);
	}
	else {
		TIME_PROFILE((boost::format("aiResFac.CreateAiScene %1% %2%") % assetPath %redirectRes).str());
		scene->SetLoaded(nullptr != loader->Execute(assetPath, redirectRes));
	}
#endif
	return scene;
}

}
}