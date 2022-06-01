#pragma once
#include "core/mir_export.h"
#include "core/base/declare_macros.h"
#include "core/base/launch.h"
#include "core/rendersys/predeclare.h"
#include "core/renderable/renderable_base.h"

namespace mir {
namespace rend {

struct SkyboxVertex {
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	Eigen::Vector3f Pos;
};

struct SphericalHarmonicsConstants {
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	Eigen::Matrix4f C0C1 = Eigen::Matrix4f::Zero();
	Eigen::Matrix4f C2 = Eigen::Matrix4f::Zero();
	Eigen::Vector4f C2_2 = Eigen::Vector4f::Zero();
};

class MIR_CORE_API SkyBox : public RenderableSingleRenderOp
{
	typedef RenderableSingleRenderOp Super;
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	SkyBox(Launch launchMode, ResourceManager& resMng, const res::MaterialInstance& material);
	void SetDiffuseEnvMap(const ITexturePtr& texture);
	void SetLutMap(const ITexturePtr& texture);
	void SetSphericalHarmonicsConstants(const SphericalHarmonicsConstants& shc);
public:
	const ITexturePtr& GetDiffuseEnvMap() const { return mDiffuseEnvMap; }
	const ITexturePtr& GetLutMap() const { return mLutMap; }
	const SphericalHarmonicsConstants& GetSphericalHarmonicsConstants() const { return mSHConstants; }
private:
	ITexturePtr mLutMap, mDiffuseEnvMap;
	SphericalHarmonicsConstants mSHConstants;
};

}
}