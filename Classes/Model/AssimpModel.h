#pragma once
#include "TPredefine.h"
#include "TMesh.h"
#include "IRenderable.h"

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

class IRenderSystem;
class AssimpModel 
	: public IRenderable
{
public:
	AssimpModel(IRenderSystem* RenderSys, TMovablePtr pMove, 
		const std::string& shaderName, const std::vector<D3D11_INPUT_ELEMENT_DESC>& layouts, std::function<void(TMaterialPtr)> cb = nullptr);
	AssimpModel(IRenderSystem* RenderSys, TMovablePtr pMove, const std::string& matType);
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
	void LoadMaterial(const std::string& shaderName, const std::vector<D3D11_INPUT_ELEMENT_DESC>& layout, std::function<void(TMaterialPtr)> cb);
	void LoadMaterial(const std::string& matType, std::function<void(TMaterialPtr)> cb);
	void processNode(aiNode * node, const aiScene * scene);
	TMeshSharedPtr processMesh(aiMesh * mesh, const aiScene * scene);
	std::vector<ITexturePtr> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const aiScene* scene);
private:
	TMaterialPtr mMaterial;
	TMovablePtr mMove;
	std::function<void(TMaterialPtr)> mMatCb;
public:
	TMeshSharedPtrVector mMeshes;
	std::map<std::string, const aiNode*> mBoneNodesByName;
	std::map<const aiNode*, AiNodeInfo> mNodeInfos;
	std::map<std::string, ITexturePtr> mLoadedTexture;
	aiNode* mRootNode = nullptr;
	const aiScene* mScene = nullptr;
	int mCurrentAnimIndex = -1;
	int mDrawCount;
private:
	std::vector<aiMatrix4x4> mTransforms;
	float mElapse = 0.0f;
	Assimp::Importer* mImporter = nullptr;
	IRenderSystem* mRenderSys;
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