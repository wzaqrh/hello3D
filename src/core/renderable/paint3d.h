#pragma once
#include "core/mir_export.h"
#include "core/base/declare_macros.h"
#include "core/base/launch.h"
#include "core/base/attribute_struct.h"
#include "core/rendersys/predeclare.h"
#include "core/renderable/renderable_base.h"

namespace mir {
namespace rend {

class MIR_CORE_API Paint3DBase : public Renderable {
public:
	Paint3DBase(Launch launchMode, ResourceManager& resMng, const res::MaterialInstance& matLine);
	virtual void Clear();
	virtual void SetColor(unsigned color);
	void SetColor(const Eigen::Vector4f& color);
public:
	TransformPtr GetTransform() const { return GetComponent<Transform>(); }
	void GenRenderOperation(RenderOperationQueue& ops) override;
	Eigen::AlignedBox3f GetWorldAABB() const override { return Eigen::AlignedBox3f(); }
	void GetMaterials(std::vector<res::MaterialInstance>& mtls) const override;
protected:
	ResourceManager& mResMng;
	const Launch mLaunchMode;
	res::MaterialInstance mMaterial;
	IVertexBufferPtr mVertexBuffer;
	IIndexBufferPtr mIndexBuffer;
protected:
	unsigned mColor;
	std::vector<vbSurface> mVertexs;
	std::vector<short> mIndices;
	size_t mIndexPos, mVertexPos;
	bool mDataDirty;
};

class MIR_CORE_API LinePaint3D : public Paint3DBase {
	typedef Paint3DBase Super;
public:
	LinePaint3D(Launch launchMode, ResourceManager& resMng, const res::MaterialInstance& matLine);
	void DrawLine(const Eigen::Vector3f& p0, const Eigen::Vector3f& p1);
	void DrawPolygon(const Eigen::Vector3f pts[], size_t count);
	void DrawAABBEdge(const Eigen::AlignedBox3f& aabb);
	void DrawRectEdge(const Eigen::Vector3f& plb, const Eigen::Vector3f& prt, const Eigen::Vector3f& normal);
};

class MIR_CORE_API Paint3D : public Paint3DBase
{
	typedef Paint3DBase Super;
public:
	Paint3D(Launch launchMode, ResourceManager& resMng, const res::MaterialInstance& matTri, const res::MaterialInstance& matLine);
	void Clear() override;
	void SetColor(unsigned color) override;
	void DrawCube(const Eigen::Vector3f& plb, const Eigen::Vector3f& prt, const Eigen::Vector3f& direction);
	TemplateArgs void DrawLine(T &&...args) { return mLinePaint->DrawLine(std::forward<T>(args)...); }
	TemplateArgs void DrawPolygon(T &&...args) { return mLinePaint->DrawPolygon(std::forward<T>(args)...); }
	TemplateArgs void DrawAABBEdge(T &&...args) { return mLinePaint->DrawAABBEdge(std::forward<T>(args)...); }
	TemplateArgs void DrawRectEdge(T &&...args) { return mLinePaint->DrawRectEdge(std::forward<T>(args)...); }
public:
	void GenRenderOperation(RenderOperationQueue& ops) override;
private:
	LinePaint3DPtr mLinePaint;
};

}
}
