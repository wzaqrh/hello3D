#include <boost/filesystem.hpp>
#include <boost/format.hpp>
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
	
	TemplateArgs CoTask<bool> Execute(T &&...args) {
		mAsset.SetLoading();
		CoAwait mResourceMng.SwitchToLaunchService(__LaunchAsync__);

		mAsset.SetLoaded(ExecuteLoadRawData(std::forward<T>(args)...) && CoAwait ExecuteSetupData());
		return mAsset.IsLoaded();
	}
	TemplateArgs CoTask<bool> operator()(T &&...args) {
		return CoAwait Execute(std::forward<T>(args)...);
	}
private:
	bool ExecuteLoadRawData(const std::string& imgPath, const std::string& redirectResource)
	{
		TIME_PROFILE((boost::format("\t\taiScene.LoadRawData (%1% %2%)") %imgPath %redirectResource).str());

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
				//aiProcess_ConvertToLeftHanded |
			#endif
				aiProcess_Triangulate |
				aiProcess_CalcTangentSpace |
				aiProcess_GenBoundingBoxes;/* |
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
	CoTask<bool> ExecuteSetupData()
	{
		COROUTINE_VARIABLES;
		mAsset.mRootNode = mAsset.AddNode(mAsset.mScene->mRootNode);
		std::vector<CoTask<bool>> tasks;
		ProcessNode(mAsset.mRootNode, mAsset.mScene, tasks);
		CoAwait WhenAllReady(std::move(tasks));

		BOOST_ASSERT(mAsset.mScene != nullptr);
		for (unsigned i = 0; i < mAsset.mScene->mNumMeshes; ++i) {
			const aiMesh* mesh = mAsset.mScene->mMeshes[i];
			for (unsigned j = 0; j < mesh->mNumBones; ++j) {
				const aiBone* bone = mesh->mBones[j];
				BOOST_ASSERT(mAsset.mBoneNodesByName[bone->mName.data] == nullptr
					|| mAsset.mBoneNodesByName[bone->mName.data]->RawNode == mAsset.mScene->mRootNode->FindNode(bone->mName));

				//mAsset.mBoneNodesByName[bone->mName.data] = mAsset.mScene->mRootNode->FindNode(bone->mName);
				auto findRawNode = mAsset.mScene->mRootNode->FindNode(bone->mName);
				auto findIter = std::find_if(mAsset.mNodeBySerializeIndex.begin(), mAsset.mNodeBySerializeIndex.end(), [&findRawNode](const AiNodePtr& nnode) {
					return nnode->RawNode == findRawNode;
				});
				mAsset.mBoneNodesByName[bone->mName.data] = IF_AND_OR(findIter != mAsset.mNodeBySerializeIndex.end(), *findIter, nullptr);
			}
		}
		CoReturn true;
	}
private:
	AiNodePtr ProcessNode(const AiNodePtr& node, const aiScene* rawScene, std::vector<CoTask<bool>>& tasks) {
		COROUTINE_VARIABLES_2(node, rawScene);

		const aiNode* rawNode = node->RawNode;
		for (int i = 0; i < rawNode->mNumMeshes; i++) {
			aiMesh* rawMesh = rawScene->mMeshes[rawNode->mMeshes[i]];
			node->AddMesh(ProcessMesh(rawMesh, rawScene, tasks));
		}

		for (int i = 0; i < rawNode->mNumChildren; i++) {
			AiNodePtr child = mAsset.AddNode(rawNode->mChildren[i]);
			node->AddChild(ProcessNode(child, rawScene, tasks));
		}
		return node;
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
	AssimpMeshPtr ProcessMesh(const aiMesh* rawMesh, const aiScene* scene, std::vector<CoTask<bool>>& tasks) const {
		COROUTINE_VARIABLES_2(rawMesh, scene);

		AssimpMeshPtr meshPtr = CreateInstance<AssimpMesh>();
		auto& mesh = *meshPtr;
		mesh.mAiMesh = rawMesh;

		boost::filesystem::path matPath = RedirectPathOnDir(boost::filesystem::path(std::string(rawMesh->mName.C_Str()) + ".Material"));
		std::string loadParam = boost::filesystem::is_regular_file(matPath) ? matPath.string() : MAT_MODEL;
		tasks.push_back(mResourceMng.CreateMaterial(mesh.mMaterial, mLaunchMode, loadParam));

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
CoTask<bool> AiResourceFactory::CreateAiScene(Launch launchMode, AiScenePtr& scene, ResourceManager& resMng, std::string assetPath, std::string redirectRes) ThreadSafe
{
	COROUTINE_VARIABLES_5(launchMode, scene, resMng, assetPath, redirectRes);
	//CoAwait mResourceMng.SwitchToLaunchService(launchMode)
	TIME_PROFILE((boost::format("\taiResFac.CreateAiScene (%1% %2%)") %assetPath %redirectRes).str());

	scene = IF_OR(scene, CreateInstance<AiScene>());
	AiSceneLoaderPtr loader = CreateInstance<AiSceneLoader>(launchMode, resMng, scene);
	CoAwait loader->Execute(assetPath, redirectRes);
	CoReturn scene->IsLoaded();
}

}
}