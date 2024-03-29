#pragma once
#include <boost/noncopyable.hpp>
#include "core/base/stl.h"
#include "core/base/math.h"
#include "core/base/cppcoro.h"
#include "core/base/launch.h"
#include "core/base/data.h"
#include "core/base/tpl/atomic_map.h"
#include "core/base/declare_macros.h"
#include "core/rendersys/predeclare.h"
#include "core/resource/predeclare.h"
#include "core/rendersys/texture.h"

namespace mir {
namespace res {

class MIR_CORE_API TextureFactory : boost::noncopyable
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	TextureFactory(ResourceManager& resMng);
	~TextureFactory();

	CoTask<bool> CreateTextureByData(ITexturePtr& texture, Launch lchMode, ResourceFormat format, Eigen::Vector4i w_h_mip_face, const Data2 datas[]) ThreadSafe ThreadMaySwitch;
	CoTask<bool> CreateTextureByFile(ITexturePtr& texture, Launch lchMode, std::string filepath, ResourceFormat format = kFormatUnknown, bool autoGenMipmap = false) ThreadSafe ThreadMaySwitch;
	DECLARE_COTASK_FUNCTIONS(ITexturePtr, CreateTextureByData, ThreadSafe ThreadMaySwitch);
	DECLARE_COTASK_FUNCTIONS(ITexturePtr, CreateTextureByFile, ThreadSafe ThreadMaySwitch);
private:
	CoTask<bool> _LoadTextureByFile(ITexturePtr texture, Launch lchMode, std::string filepath, ResourceFormat format, bool autoGenMipmap) ThreadSafe ThreadMaySwitch;
private:
	ResourceManager& mResMng;
	RenderSystem& mRenderSys;
	tpl::AtomicMap<std::string, ITexturePtr> mTextureByKey;
};

}
}