#include "core/renderable/paint3d.h"
#include "core/scene/transform.h"
#include "core/resource/resource_manager.h"
#include "core/base/macros.h"

namespace mir {
namespace rend {

/********** Paint3DBase **********/
const Eigen::Vector2f UV_ZERO = Eigen::Vector2f::Zero();

Paint3DBase::Paint3DBase(Launch launchMode, ResourceManager& resMng, const res::MaterialInstance& matLine)
	: mLaunchMode(launchMode)
	, mResMng(resMng)
	, mMaterial(matLine)
{
	mColor = -1;
	mIndexPos = 0;
	mVertexPos = 0;

	mIndices.resize(1024);
	mVertexs.resize(1024);

	mIndexBuffer = resMng.CreateIndexBuffer(__launchMode__, kFormatR16UInt, Data::MakeSize(mIndices));
	mVertexBuffer = resMng.CreateVertexBuffer(__launchMode__, sizeof(vbSurface), 0, Data::MakeSize(mVertexs));
}

void Paint3DBase::GenRenderOperation(RenderOperationQueue& ops)
{
	if (!mMaterial->IsLoaded()) return;
	if (mIndexPos <= 0) return;

	if (mDataDirty) {
		mDataDirty = false;
		mResMng.UpdateBuffer(mIndexBuffer, Data::Make(mIndices));
		mResMng.UpdateBuffer(mVertexBuffer, Data::Make(mVertexs));
	}

	RenderOperation op = {};
	op.Material = mMaterial;
	op.IndexBuffer = mIndexBuffer;
	op.AddVertexBuffer(mVertexBuffer);
	if (GetTransform()) op.WorldTransform = GetTransform()->GetWorldMatrix();
	op.IndexCount = mIndexPos;
	ops.AddOP(op);
}

void Paint3DBase::Clear()
{
	mIndexPos = 0;
	mVertexPos = 0;
}

void Paint3DBase::SetColor(unsigned color)
{
	mColor = color;
}
void Paint3DBase::SetColor(const Eigen::Vector4f& color)
{
	unsigned char c[4] = {
		static_cast<unsigned char>(color.x() * 255),
		static_cast<unsigned char>(color.y() * 255),
		static_cast<unsigned char>(color.z() * 255),
		static_cast<unsigned char>(color.w() * 255),
	};
	SetColor(*((int*)c));
}

/********** LinePaint3D **********/
LinePaint3D::LinePaint3D(Launch launchMode, ResourceManager& resMng, const res::MaterialInstance& matLine)
	: Super(launchMode, resMng, matLine)
{}

void LinePaint3D::DrawLine(const Eigen::Vector3f& p0, const Eigen::Vector3f& p1)
{
	mVertexs[mVertexPos++] = vbSurface{ p0, mColor, UV_ZERO };
	mVertexs[mVertexPos++] = vbSurface{ p1, mColor, UV_ZERO };

	mIndices[mIndexPos++] = mVertexPos - 2;
	mIndices[mIndexPos++] = mVertexPos - 1;

	mDataDirty = true;
}

void LinePaint3D::DrawPolygon(const Eigen::Vector3f pts[], size_t count)
{
	for (size_t i = 0; i < count - 1; ++i) {
		mIndices[mIndexPos++] = mVertexPos;
		mIndices[mIndexPos++] = mVertexPos + 1;
		mVertexs[mVertexPos++] = vbSurface{ pts[i], mColor, UV_ZERO };
		mVertexs[mVertexPos++] = vbSurface{ pts[i + 1], mColor, UV_ZERO };
	}

	mDataDirty = true;
}

void LinePaint3D::DrawRectEdge(const Eigen::Vector3f& plb, const Eigen::Vector3f& prt, const Eigen::Vector3f& normal)
{
	Eigen::Quaternionf rot = Eigen::Quaternionf::FromTwoVectors(Eigen::Vector3f(0, 0, -1), normal);
	Eigen::Quaternionf rot_inv = rot.inverse();

	Eigen::Vector3f p0 = rot_inv * plb;
	Eigen::Vector3f p1 = rot_inv * prt;
	BOOST_ASSERT(fabs(p0.z() - p1.z()) < 1e-2f);

	float z0 = p0.z();
	float x0 = std::min(p0.x(), p1.x());
	float y0 = std::min(p0.y(), p1.y());
	float x1 = std::max(p0.x(), p1.x());
	float y1 = std::max(p0.y(), p1.y());
	Eigen::Vector3f pts[5] = {
		Eigen::Vector3f(x0, y0, z0),
		Eigen::Vector3f(x1, y0, z0),
		Eigen::Vector3f(x1, y1, z0),
		Eigen::Vector3f(x0, y1, z0),
	};
	for (auto& p : pts)
		p = rot * p;
	pts[4] = pts[0];
	DrawPolygon(pts, 5);
}

void LinePaint3D::DrawAABBEdge(const Eigen::AlignedBox3f& aabb)
{
	if (!aabb.isEmpty()) {
		Eigen::Vector3f p0 = aabb.min();
		Eigen::Vector3f p1 = aabb.max();
		DrawRectEdge(p0, Eigen::Vector3f(p1.x(), p1.y(), p0.z()), Eigen::Vector3f(0, 0, -1));//front
		DrawRectEdge(Eigen::Vector3f(p0.x(), p0.y(), p1.z()), p1, Eigen::Vector3f(0, 0, 1));//back
		DrawRectEdge(Eigen::Vector3f(p0.x(), p1.y(), p0.z()), p1, Eigen::Vector3f(0, 1, 0));//up
		DrawRectEdge(p0, Eigen::Vector3f(p1.x(), p0.y(), p1.z()), Eigen::Vector3f(0, -1, 0));//down
	}
}

/********** Cube **********/
Paint3D::Paint3D(Launch launchMode, ResourceManager& resMng, const res::MaterialInstance& matTri, const res::MaterialInstance& matLine)
	: Super(launchMode, resMng, matTri)
{
	mLinePaint = CreateInstance<LinePaint3D>(launchMode, resMng, matLine);
}

void Paint3D::Clear()
{
	Super::Clear();
	mLinePaint->Clear();
}

void Paint3D::SetColor(unsigned color)
{
	mColor = color;
	mLinePaint->SetColor(color);
}

inline constexpr std::array<short, 6> GetFaceIndice(short i0, short i1, short i2, short i3) {//i0-i3·¨Ïß³¯Íâ
	return std::array<short, 6>{ i0, i1, i2, i0, i2, i3 };
}
inline void ApplyFace(std::vector<short>& mIndices, size_t& mIndexPos, short base, const std::array<short, 6>& face_side) {
	for (size_t i = 0; i < 6; ++i)
		mIndices[mIndexPos++] = base + face_side[i];
}
void Paint3D::DrawCube(const Eigen::Vector3f& plb, const Eigen::Vector3f& prt, const Eigen::Vector3f& direction)
{
	Eigen::Quaternionf rot = Eigen::Quaternionf::FromTwoVectors(Eigen::Vector3f(0, 0, -1), direction);
	Eigen::Quaternionf rot_inv = rot.inverse();
	
	Eigen::Vector3f p0 = rot_inv * plb;
	Eigen::Vector3f p1 = rot_inv * prt;

	short base = mVertexPos;
	mVertexs[base + 0] = (vbSurface{ Eigen::Vector3f(p0.x(), p0.y(), p0.z()), mColor, UV_ZERO });
	mVertexs[base + 1] = (vbSurface{ Eigen::Vector3f(p0.x(), p1.y(), p0.z()), mColor, UV_ZERO });
	mVertexs[base + 2] = (vbSurface{ Eigen::Vector3f(p1.x(), p1.y(), p0.z()), mColor, UV_ZERO });
	mVertexs[base + 3] = (vbSurface{ Eigen::Vector3f(p1.x(), p0.y(), p0.z()), mColor, UV_ZERO });

	mVertexs[base + 4] = (vbSurface{ Eigen::Vector3f(p0.x(), p0.y(), p1.z()), mColor, UV_ZERO });
	mVertexs[base + 5] = (vbSurface{ Eigen::Vector3f(p0.x(), p1.y(), p1.z()), mColor, UV_ZERO });
	mVertexs[base + 6] = (vbSurface{ Eigen::Vector3f(p1.x(), p1.y(), p1.z()), mColor, UV_ZERO });
	mVertexs[base + 7] = (vbSurface{ Eigen::Vector3f(p1.x(), p0.y(), p1.z()), mColor, UV_ZERO });
	mVertexPos += 8;

	for (size_t i = 0; i < 8; ++i)
		mVertexs[base + i].Pos = rot * mVertexs[base + i].Pos;

	std::array<short, 6> face_front = GetFaceIndice(0,1,2,3);
	std::array<short, 6> face_back = GetFaceIndice(7,6,5,4);
	std::array<short, 6> face_right = GetFaceIndice(2,6,7,3);
	std::array<short, 6> face_left = GetFaceIndice(0,4,5,1);
	std::array<short, 6> face_top = GetFaceIndice(1,5,6,2);
	std::array<short, 6> face_bottom = GetFaceIndice(3,7,4,0);
	ApplyFace(mIndices, mIndexPos, base, face_front);
	ApplyFace(mIndices, mIndexPos, base, face_back);
	ApplyFace(mIndices, mIndexPos, base, face_right);
	ApplyFace(mIndices, mIndexPos, base, face_left);
	ApplyFace(mIndices, mIndexPos, base, face_top);
	ApplyFace(mIndices, mIndexPos, base, face_bottom);

	mDataDirty = true;
}

void Paint3D::GenRenderOperation(RenderOperationQueue& ops)
{
	Super::GenRenderOperation(ops);
	mLinePaint->GenRenderOperation(ops);
}

}
}