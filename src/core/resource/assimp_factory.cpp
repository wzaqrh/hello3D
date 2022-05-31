#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/assert.hpp>
#include <assimp/cimport.h>
#include <assimp/Importer.hpp>
#include <assimp/ai_assert.h>
#include <assimp/cfileio.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/pbrmaterial.h>
#include <assimp/IOSystem.hpp>
#include <assimp/IOStream.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/DefaultLogger.hpp>
#include <unordered_map>
#include "core/base/debug.h"
#include "core/base/macros.h"
#include "core/resource/assimp_factory.h"
#include "core/resource/assimp_resource.h"
#include "core/resource/material_name.h"
#include "core/resource/material.h"
#include "core/resource/material_factory.h"
#include "core/resource/resource_manager.h"

#define VEC_ASSIGN(DST, SRC) static_assert(sizeof(DST) == sizeof(SRC)); memcpy(DST.data(), &SRC, sizeof(SRC))
#define VEC_ASSIGN1(DST, SRC, SIZE) static_assert(sizeof(DST) == SIZE); memcpy(DST.data(), &SRC, SIZE)
#define VEC_ASSIGN2(DST, SRC, SIZE) memcpy(DST.data(), &SRC, sizeof(SRC))

namespace mir {
namespace res {

/********** AiSceneLoader **********/
struct ResourceRedirector {
public:
	void Init(const std::string& resPath, const std::string& redirectResource) 
	{
		mResFullPath = boost::filesystem::system_complete(resPath);

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
			mRedirectResourceDir = mResFullPath.parent_path().string();
		}
		BOOST_ASSERT(mRedirectResourceDir.empty() || mRedirectResourceDir.back() != '/');
	}
	boost::filesystem::path operator()(const boost::filesystem::path& path, bool forceNotRelative = false) const {
		boost::filesystem::path result(mRedirectResourceDir);
		if (!path.is_relative() || forceNotRelative) return result.append(path.filename().string());
		else return result.append(path.string());
	}
	const boost::filesystem::path& GetResFullPath() const { return mResFullPath; }
private:
	boost::filesystem::path mResFullPath;
	std::string mRedirectResourceDir, mRedirectResourceExt;
};

class AiSceneLoader {
public:
	AiSceneLoader(Launch lchMode, ResourceManager& resMng, MaterialLoadParam loadParam, AiScenePtr asset)
		: mLaunchMode(lchMode), mResMng(resMng), mLoadParam(loadParam), mAsset(*asset), mResult(asset) 
	{}
	~AiSceneLoader() {
		delete mAssetImporter;
	}

	TemplateArgs CoTask<bool> Execute(T &&...args) {
		mAsset.SetLoading();
		CoAwait mResMng.SwitchToLaunchService(__LaunchAsync__);

		mAsset.SetLoaded(ExecuteLoadRawData(std::forward<T>(args)...) && CoAwait ExecuteSetupData());
		return mAsset.IsLoaded();
	}
	TemplateArgs CoTask<bool> operator()(T &&...args) {
		return CoAwait Execute(std::forward<T>(args)...);
	}
private:
	bool ExecuteLoadRawData(const std::string& resPath, const std::string& redirectResource)
	{
		TIME_PROFILE((boost::format("\t\tAiSceneLoader.Execute (%1% %2%)") %resPath %redirectResource).str());
		mRedirectPathOnDir.Init(resPath, redirectResource);

		try
		{
		#define IMPORT_LEFTHAND
			constexpr uint32_t ImportFlags =
			#if defined IMPORT_LEFTHAND
				aiProcess_ConvertToLeftHanded |
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
			mAssetImporter = new Assimp::Importer;
			mAssetScene = const_cast<Assimp::Importer*>(mAssetImporter)->ReadFile(mRedirectPathOnDir.GetResFullPath().string(), ImportFlags);
		}
		catch (...)
		{
			DEBUG_LOG_ERROR("Assimp::Importer ReadFile error");
		}
		return mAssetScene != nullptr;
	}
	CoTask<bool> ExecuteSetupData()
	{
		COROUTINE_VARIABLES;
		BOOST_ASSERT(mAssetScene != nullptr);

		if (mAssetScene->mNumAnimations > 0) {
			mAsset.mAnimations.resize(mAssetScene->mNumAnimations);
			memcpy(&mAsset.mAnimations[0], &mAssetScene->mAnimations[0], mAssetScene->mNumAnimations * sizeof(mAssetScene->mAnimations[0]));
		}

		std::vector<CoTask<bool>> tasks;
		mAsset.mRootNode = ProcessNode(mAssetScene->mRootNode, mAssetScene, tasks);
		CoAwait WhenAllReady(std::move(tasks));

		for (const auto& mesh : mAsset.GetMeshes()) {
			for (auto& bone : mesh->mBones) {
				bone.mRelateNode = mAsset.FindNodeByName(bone.mName);
			}
		}
		CoReturn true;
	}
private:
	AiNodePtr ProcessNode(const aiNode* rawNode, const aiScene* rawScene, std::vector<CoTask<bool>>& tasks) {
		AiNodePtr node = mAsset.AddNode();
		COROUTINE_VARIABLES_2(node, rawScene);

		node->mName = rawNode->mName.C_Str();
		node->mLocalTransform = node->mGlobalTransform = *(const Eigen::Matrix4f*)&rawNode->mTransformation;
		for (int i = 0; i < rawNode->mNumMeshes; i++) {
			unsigned meshIndex = rawNode->mMeshes[i];
			aiMesh* rawMesh = rawScene->mMeshes[meshIndex];
			node->AddMesh(ProcessMesh(rawMesh, meshIndex, rawScene, tasks));
		}

		for (int i = 0; i < rawNode->mNumChildren; i++) {
			node->AddChild(ProcessNode(rawNode->mChildren[i], rawScene, tasks));
		}
		return node;
	}
	
	AssimpMeshPtr ProcessMesh(const aiMesh* rawMesh, int meshIndex, const aiScene* scene, std::vector<CoTask<bool>>& tasks) const {
		COROUTINE_VARIABLES_2(rawMesh, scene);

		AssimpMeshPtr meshPtr = mAsset.AddMesh();
		auto& mesh = *meshPtr;
		mesh.mHasBones = rawMesh->HasBones();
		mesh.mSceneMeshIndex = meshIndex;

		const aiVector3D& mmin = rawMesh->mAABB.mMin, &mmax = rawMesh->mAABB.mMax;
		mesh.mAABB = Eigen::AlignedBox3f();
		mesh.mAABB.extend(Eigen::Vector3f(mmin.x, mmin.y, mmin.z));
		mesh.mAABB.extend(Eigen::Vector3f(mmax.x, mmax.y, mmax.z));

		boost::filesystem::path matPath = mRedirectPathOnDir(boost::filesystem::path(std::string(rawMesh->mName.C_Str()) + ".Material"));
		MaterialLoadParam loadParam = mLoadParam;
		loadParam.ShaderVariantName = boost::filesystem::is_regular_file(matPath) ? matPath.string() : MAT_MODEL;
		tasks.push_back(mResMng.CreateMaterial(mesh.mMaterial, mLaunchMode, loadParam));

		if (rawMesh->mNumBones > 0) {
			mesh.mBones.resize(rawMesh->mNumBones);
			for (size_t i = 0; i < mesh.mBones.size(); ++i) {
				auto& dst = mesh.mBones[i];
				const aiBone& src = *rawMesh->mBones[i];
				dst.mName = src.mName.C_Str();
				dst.mOffsetMatrix = *(Eigen::Matrix4f*)&src.mOffsetMatrix;
				if (src.mNumWeights) {
					dst.mWeights.resize(src.mNumWeights);
					memcpy(&dst.mWeights[0], &src.mWeights[0], src.mNumWeights * sizeof(src.mWeights[0]));
					static_assert(sizeof(dst.mWeights[0]) == sizeof(src.mWeights[0]), "");
				}
			}
			memcpy(&mesh.mBones[0], rawMesh->mBones, rawMesh->mNumBones * sizeof(rawMesh->mBones[0]));
		}

		auto& surfVerts = mesh.mSurfVertexs; surfVerts.resize(rawMesh->mNumVertices);
		auto& skeletonVerts = mesh.mSkeletonVertexs; skeletonVerts.resize(rawMesh->mNumVertices);
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
			#if !defined EIGEN_DONT_ALIGN_STATICALLY
				skeletonVerts[vertexId].Normal = AS_CONST_REF(Eigen::Vector3f, rawMesh->mNormals[vertexId]);
			#else
				VEC_ASSIGN(skeletonVerts[vertexId].Normal, rawMesh->mNormals[vertexId]);
			#endif
			}
		}
		if (rawMesh->mTangents) {
			for (size_t vertexId = 0; vertexId < rawMesh->mNumVertices; vertexId++) {
			#if !defined EIGEN_DONT_ALIGN_STATICALLY
				skeletonVerts[vertexId].Tangent = AS_CONST_REF(Eigen::Vector3f, rawMesh->mTangents[vertexId]);
			#else
				VEC_ASSIGN2(skeletonVerts[vertexId].Tangent, rawMesh->mTangents[vertexId]);
				skeletonVerts[vertexId].Tangent.w() = 1.0f;
			#endif
			}
		}
		if (rawMesh->mBitangents) {
			for (size_t vertexId = 0; vertexId < rawMesh->mNumVertices; vertexId++) {
			#if !defined EIGEN_DONT_ALIGN_STATICALLY
				skeletonVerts[vertexId].BiTangent = AS_CONST_REF(Eigen::Vector3f, rawMesh->mBitangents[vertexId]);
			#else
				VEC_ASSIGN(skeletonVerts[vertexId].BiTangent, rawMesh->mBitangents[vertexId]);
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

		mesh.Build(mLaunchMode, mResMng);
		return meshPtr;
	}
private:
	const Launch mLaunchMode;
	ResourceManager& mResMng;
	MaterialLoadParam mLoadParam;
	AiScene& mAsset;
	AiScenePtr mResult;
private:
	ResourceRedirector mRedirectPathOnDir;
	const Assimp::Importer* mAssetImporter = nullptr;
	const aiScene* mAssetScene = nullptr;
};
typedef std::shared_ptr<AiSceneLoader> AiSceneLoaderPtr;

/********** ObjLoader **********/
class AiSceneObjLoader {
public:
	AiSceneObjLoader(Launch lchMode, ResourceManager& resMng, MaterialLoadParam loadParam, AiScenePtr asset)
	: mLaunchMode(lchMode), mResMng(resMng), mLoadParam(loadParam), mAsset(*asset), mResult(asset)
	{}

	TemplateArgs CoTask<bool> Execute(T &&...args) {
		mAsset.SetLoading();
		CoAwait mResMng.SwitchToLaunchService(__LaunchAsync__);

		mAsset.SetLoaded(ExecuteLoadRawData(std::forward<T>(args)...) && CoAwait ExecuteSetupData());
		return mAsset.IsLoaded();
	}
	TemplateArgs CoTask<bool> operator()(T &&...args) {
		return CoAwait Execute(std::forward<T>(args)...);
	}
private:
	struct ObjNode {
		std::vector<Eigen::Vector3f> Vertices;
		std::vector<Eigen::Vector2f> Uvs;
		std::vector<Eigen::Vector3f> Normals;
		std::vector<unsigned int> Indices;
		std::string MtlName;
	};
	static bool LoadOBJ(const std::string& path,
		std::vector<Eigen::Vector3f>& vertices,
		std::vector<Eigen::Vector2f>& uvs,
		std::vector<Eigen::Vector3f>& normals,
		std::vector<unsigned int>& vertexIndices,
		std::string& materalName) 
	{
		FILE* fd = fopen(path.c_str(), "r");
		if (fd) {
			char lineHeader[128];
			while (fscanf(fd, "%s", lineHeader) != EOF) {
				if (strcmp(lineHeader, "v") == 0) {
					Eigen::Vector3f vertex;
					fscanf(fd, "%f %f %f\n", &vertex.x(), &vertex.y(), &vertex.z());
					vertex.x() = -vertex.x();
					vertices.push_back(vertex);
				}
				else if (strcmp(lineHeader, "vt") == 0) {
					Eigen::Vector2f uv;
					fscanf(fd, "%f %f\n", &uv.x(), &uv.y());
					uvs.push_back(uv);
				}
				else if (strcmp(lineHeader, "vn") == 0) {
					Eigen::Vector3f normal;
					fscanf(fd, "%f %f %f\n", &normal.x(), &normal.y(), &normal.z());
					normal.x() = -normal.x();
					normals.push_back(normal);
				}
				else if (strcmp(lineHeader, "f") == 0) {
					std::string vertex1, vertex2, vertex3;
					unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
					int matches = fscanf(fd, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
					if (matches != 9) {
						fclose(fd);
						fd = NULL;
						break;
					}
					vertexIndices.push_back(vertexIndex[2]-1);
					vertexIndices.push_back(vertexIndex[1]-1);
					vertexIndices.push_back(vertexIndex[0]-1);
				}
				else if (strcmp(lineHeader, "usemtl") == 0) {
					char szMtl[260];
					fscanf(fd, "%s\n", szMtl);
					materalName = szMtl;
				}
				else {
					// Probably a comment, eat up the rest of the line
					char stupidBuffer[1000];
					fgets(stupidBuffer, 1000, fd);
				}
			}
			if (fd) fclose(fd);
		}
		return fd != NULL;
	}
	bool ExecuteLoadRawData(const std::string& resPath, const std::string& redirectResource) {
		TIME_PROFILE((boost::format("\t\tAiSceneObjLoader.Execute (%1% %2%)") %resPath %redirectResource).str());
		mRedirectPathOnDir.Init(resPath, redirectResource);

		boost::filesystem::path resFullPath = boost::filesystem::system_complete(resPath);
		return LoadOBJ(resFullPath.string(), mObjNode.Vertices, mObjNode.Uvs, mObjNode.Normals, mObjNode.Indices, mObjNode.MtlName);
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
			Eigen::Vector3f tangent = (deltaPos1 * deltaUV2.y() - deltaPos2 * deltaUV1.y()) * r; tangent.normalize();
			skin0.Tangent.head<3>() = skin0.Tangent.head<3>() + tangent;
			skin1.Tangent.head<3>() = skin1.Tangent.head<3>() + tangent;
			skin2.Tangent.head<3>() = skin2.Tangent.head<3>() + tangent;

			Eigen::Vector3f bitangent = (deltaPos2 * deltaUV1.x() - deltaPos1 * deltaUV2.x()) * r; bitangent.normalize();
			skin0.BiTangent = skin0.BiTangent + bitangent;
			skin1.BiTangent = skin1.BiTangent + bitangent;
			skin2.BiTangent = skin2.BiTangent + bitangent;
		}
		for (auto& skin : skeletonVerts) {
			skin.Tangent.head<3>().normalize();
			skin.BiTangent.normalize();

			float dp = skin.BiTangent.head<3>().dot(skin.Normal.cross(skin.Tangent.head<3>()));
			if (dp < 0.0f) skin.Tangent.w() = -1.0;
			else skin.Tangent.w() = 1.0;
		}
	}
	CoTask<bool> ExecuteSetupData() {
		AiNodePtr node = mAsset.mRootNode = mAsset.AddNode();
		node->mName = IF_AND_OR(!mObjNode.MtlName.empty(), mObjNode.MtlName, mRedirectPathOnDir.GetResFullPath().stem().string());
		node->mLocalTransform = node->mGlobalTransform = Eigen::Matrix4f::Identity();

		size_t meshIndex = 0;
		{
			auto pMesh = mAsset.AddMesh();
			auto& mesh = *pMesh;

			mesh.mHasBones = false;
			mesh.mSceneMeshIndex = meshIndex++;

			boost::filesystem::path matPath = mRedirectPathOnDir(boost::filesystem::path(node->mName + ".Material"));
			auto loadParam = mLoadParam;
			loadParam.ShaderVariantName = boost::filesystem::is_regular_file(matPath) ? matPath.string() : MAT_MODEL;
			CoAwait mResMng.CreateMaterial(mesh.mMaterial, mLaunchMode, loadParam);

			mesh.mAABB = Eigen::AlignedBox3f();
			auto& surfVerts = mesh.mSurfVertexs; surfVerts.resize(mObjNode.Vertices.size());
			auto& skeletonVerts = mesh.mSkeletonVertexs; skeletonVerts.resize(mObjNode.Vertices.size());
			for (int vertexId = 0; vertexId < mObjNode.Vertices.size(); vertexId++) {
				const auto& src = mObjNode;
				auto& dst_surf = surfVerts[vertexId];
				auto& dst_skeleton = skeletonVerts[vertexId];

			#if !defined EIGEN_DONT_ALIGN_STATICALLY
				dst_surf.Pos.x() = src.Position.X;
				dst_surf.Pos.y() = src.Position.Y;
				dst_surf.Pos.z() = src.Position.Z;
			#else
				VEC_ASSIGN(dst_surf.Pos, src.Vertices[vertexId]);
			#endif
				mesh.mAABB.extend(dst_surf.Pos);

			#if !defined EIGEN_DONT_ALIGN_STATICALLY
				surfVerts[vertexId].Tex.x() = src.TextureCoordinate.X;
				surfVerts[vertexId].Tex.y() = src.TextureCoordinate.Y;
			#else
				VEC_ASSIGN(dst_surf.Tex, src.Uvs[vertexId]);
			#endif

			#if !defined EIGEN_DONT_ALIGN_STATICALLY
				skeletonVerts[vertexId].Normal = AS_CONST_REF(Eigen::Vector3f, rawMesh->mNormals[vertexId]);
			#else
				VEC_ASSIGN(dst_skeleton.Normal, src.Normals[vertexId]);
			#endif
			}
			
			mesh.mIndices = std::move(mObjNode.Indices);
			ReCalculateTangents(surfVerts, skeletonVerts, mesh.mIndices);

			mesh.Build(mLaunchMode, mResMng);
			node->AddMesh(pMesh);
		}
		CoReturn true;
	}
private:
	const Launch mLaunchMode;
	ResourceManager& mResMng;
	MaterialLoadParam mLoadParam;
	AiScene& mAsset;
	AiScenePtr mResult;
private:
	ResourceRedirector mRedirectPathOnDir;
	ObjNode mObjNode;
};
typedef std::shared_ptr<AiSceneObjLoader> AiSceneObjLoaderPtr;

/********** AiAssetManager **********/
AiResourceFactory::AiResourceFactory(ResourceManager& resMng)
: mResMng(resMng)
{
}

CoTask<bool> AiResourceFactory::DoCreateAiScene(AiScenePtr& scene, Launch lchMode, std::string assetPath, std::string redirectRes, MaterialLoadParam mlp) ThreadSafe
{
	//CoAwait mResourceMng.SwitchToLaunchService(lchMode)
	COROUTINE_VARIABLES_4(lchMode, scene, assetPath, redirectRes);
	TIME_PROFILE((boost::format("\taiResFac.CreateAiScene (%1% %2%)") % assetPath % redirectRes).str());

	scene = IF_OR(scene, CreateInstance<AiScene>());
	if (boost::filesystem::path(assetPath).extension() != ".obj") {
		auto loader = CreateInstance<AiSceneLoader>(lchMode, mResMng, mlp, scene);
		CoAwait loader->Execute(assetPath, redirectRes);
	}
	else {
		auto loader = CreateInstance<AiSceneObjLoader>(lchMode, mResMng, mlp, scene);
		CoAwait loader->Execute(assetPath, redirectRes);
	}
	CoReturn scene->IsLoaded();
}
CoTask<bool> AiResourceFactory::CreateAiScene(AiScenePtr& aiScene, Launch lchMode, std::string assetPath, std::string redirectRes, MaterialLoadParam mlp) ThreadSafe
{
	//CoAwait SwitchToLaunchService(lchMode);
	COROUTINE_VARIABLES_3(lchMode, assetPath, redirectRes);

	AiResourceKey key{ assetPath, redirectRes, mlp };
	bool resNeedLoad = false;
	aiScene = mAiSceneByKey.GetOrAdd(key, [&]() {
		auto aiScene = CreateInstance<res::AiScene>();
		DEBUG_SET_RES_PATH(aiScene, (boost::format("path:%1%, redirect:%2%") % assetPath % redirectRes).str());
		DEBUG_SET_CALL(aiScene, lchMode);
		resNeedLoad = true;
		return aiScene;
	});
	if (resNeedLoad) {
		CoAwait this->DoCreateAiScene(aiScene, lchMode, std::move(assetPath), std::move(redirectRes), std::move(mlp));
	}
	else {
		CoAwait mResMng.WaitResComplete(aiScene);
	}
	CoReturn aiScene->IsLoaded();
}

}
}