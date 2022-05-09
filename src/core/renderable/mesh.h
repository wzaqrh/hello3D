#pragma once
#include "core/mir_export.h"
#include "core/base/declare_macros.h"
#include "core/base/launch.h"
#include "core/base/attribute_struct.h"
#include "core/renderable/renderable_base.h"

namespace mir {
namespace rend {

class MIR_CORE_API Mesh : public RenderableSingleRenderOp 
{
	typedef RenderableSingleRenderOp Super;
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	Mesh(Launch launchMode, ResourceManager& resMng, const res::MaterialInstance& material, int vertCount = 1024, int indexCount = 1024);

	void Clear();
	void SetVertexs(const vbSurface* vertData, int vertCount);
	void SetVertexs(const vbSurface* vertData, int vertCount, int vertPos);
	void SetPositions(const Eigen::Vector3f* posData, int count);
	void SetColors(const Eigen::Vector4f* colorData, int count);
	void SetUVs(const Eigen::Vector2f* uvData, int count);
	void SetSubMeshCount(int count);
	void SetIndices(const unsigned int* indiceData, int indicePos, int indiceCount, int indiceBase, int subMeshIndex);
	void SetTexture(int slot, ITexturePtr texture, int subMeshIndex);
private:
	void GenRenderOperation(RenderOperationQueue& opList) override;
private:
	int mVertPos = 0, mVertDirty = false;
	vbSurfaceVector mVertices;

	int mIndiceDirty = false;
	std::vector<unsigned int> mIndices;

	struct SubMesh {
		short IndicePos, IndiceCount, IndiceBase;
		TextureVector Textures;
	};
	std::vector<SubMesh> mSubMeshs;
};

}
}