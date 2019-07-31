#pragma once
#include "std.h"
#include "TMesh.h"
#include "TMovable.h"

const int MAX_MATRICES = 256;
struct cbWeightedSkin
{
	XMMATRIX mModel;
	XMMATRIX Models[MAX_MATRICES];
	int hasNormal;
	int hasMetalness;
	int hasRoughness;
	int hasAO;
};

struct AiNodeInfo {
	aiMatrix4x4 mLocalTransform;
	aiMatrix4x4 mGlobalTransform;
	TMeshSharedPtrVector meshes;
	int channelIndex;
public:
	AiNodeInfo();
	TMeshSharedPtr operator[](int pos);
	int size() const;
	void push_back(TMeshSharedPtr mesh);
};

class TRenderSystem;
class AssimpModel 
	: public IRenderable
{
public:
	AssimpModel(TRenderSystem* RenderSys, TMovablePtr pMove, const char* vsName, const char* psName, std::function<void(TMaterialPtr)> cb = nullptr);
	~AssimpModel();
public:
	void LoadModel(const std::string& imgPath);
	void PlayAnim(int Index);

	void Update(float dt);
	void Draw();
	virtual int GenRenderOperation(TRenderOperationQueue& opList) override;
private:
	const std::vector<aiMatrix4x4>& GetBoneMatrices(const aiNode* pNode, size_t pMeshIndex);
	void DoDraw(aiNode* node, TRenderOperationQueue& opList);
	void LoadMaterial(const char* vsName, const char* psName, std::function<void(TMaterialPtr)> cb);
	void processNode(aiNode * node, const aiScene * scene);
	TMeshSharedPtr processMesh(aiMesh * mesh, const aiScene * scene);
	std::vector<TTexture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const aiScene* scene);
private:
	TMaterialPtr mMaterial;
	TMovablePtr mMove;
	std::function<void(TMaterialPtr)> mMatCb;
public:
	TMeshSharedPtrVector mMeshes;
	std::map<std::string, const aiNode*> mBoneNodesByName;
	std::map<const aiNode*, AiNodeInfo> mNodeInfos;
	std::map<std::string, TTexture> mLoadedTexture;
	aiNode* mRootNode = nullptr;
	const aiScene* mScene = nullptr;
	int mCurrentAnimIndex = -1;
	int mDrawCount;
private:
	std::vector<aiMatrix4x4> mTransforms;
	float mElapse = 0.0f;
	Assimp::Importer* mImporter = nullptr;
	TRenderSystem* mRenderSys;
};

struct Evaluator
{
public:
	std::vector<aiMatrix4x4> mTransforms;
	const aiAnimation* mAnim;
public:
	Evaluator(const aiAnimation* Anim) :mAnim(Anim) {}
	void Eval(float pTime);
};