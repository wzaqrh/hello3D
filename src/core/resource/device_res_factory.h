#pragma once
#include <boost/assert.hpp>
#include "core/mir_export.h"
#include "core/base/cppcoro.h"
#include "core/base/launch.h"
#include "core/base/declare_macros.h"
#include "core/rendersys/blob.h"
#include "core/rendersys/program.h"
#include "core/rendersys/input_layout.h"
#include "core/rendersys/hardware_buffer.h"
#include "core/rendersys/texture.h"
#include "core/rendersys/framebuffer.h"
#include "core/rendersys/render_system.h"

namespace mir {
namespace res {

class MIR_CORE_API DeviceResFactory : boost::noncopyable
{
public:
	DeviceResFactory(RenderSystem& renderSys) :mRenderSys(renderSys) {}

	TemplateArgs IIndexBufferPtr CreateIndexBuffer(Launch launchMode, T &&...args) ThreadSafe {
		auto res = mRenderSys.CreateResource(kDeviceResourceIndexBuffer); ResSetLaunch;
		mRenderSys.LoadIndexBuffer(res, std::forward<T>(args)...);
		res->SetLoaded();
		return std::static_pointer_cast<IIndexBuffer>(res);
	}
	TemplateArgs IVertexBufferPtr CreateVertexBuffer(Launch launchMode, T &&...args) ThreadSafe {
		auto res = mRenderSys.CreateResource(kDeviceResourceVertexBuffer); ResSetLaunch;
		res->SetLoaded(nullptr != mRenderSys.LoadVertexBuffer(res, std::forward<T>(args)...));
		return std::static_pointer_cast<IVertexBuffer>(res);
	}
	TemplateArgs IContantBufferPtr CreateConstBuffer(Launch launchMode, T &&...args) ThreadSafe {
		auto res = mRenderSys.CreateResource(kDeviceResourceContantBuffer); ResSetLaunch;
		res->SetLoaded(nullptr != mRenderSys.LoadConstBuffer(res, std::forward<T>(args)...));
		return std::static_pointer_cast<IContantBuffer>(res);
	}
	TemplateArgs bool UpdateBuffer(T &&...args) ThreadSafe {
		return mRenderSys.UpdateBuffer(std::forward<T>(args)...);
	}

	TemplateArgs IInputLayoutPtr CreateLayout(Launch launchMode, IProgramPtr program, T &&...args) ThreadSafe {
		auto res = mRenderSys.CreateResource(kDeviceResourceInputLayout); ResSetLaunch;
		res->SetLoaded(nullptr != mRenderSys.LoadLayout(res, program, std::forward<T>(args)...));
		return std::static_pointer_cast<IInputLayout>(res);
	}
	TemplateArgs ISamplerStatePtr CreateSampler(Launch launchMode, T &&...args) ThreadSafe {
		auto res = mRenderSys.CreateResource(kDeviceResourceSamplerState); ResSetLaunch;
		res->SetLoaded(nullptr != mRenderSys.LoadSampler(res, std::forward<T>(args)...));
		return std::static_pointer_cast<ISamplerState>(res);
	}

	TemplateArgs ITexturePtr CreateTexture(ResourceFormat format, T &&...args) ThreadSafe {
		auto res = mRenderSys.CreateResource(kDeviceResourceTexture);
		res->SetLoaded(nullptr != mRenderSys.LoadTexture(res, format, std::forward<T>(args)...));
		return std::static_pointer_cast<ITexture>(res);
	}
	TemplateArgs bool LoadRawTextureData(T &&...args) ThreadSafe {
		return mRenderSys.LoadRawTextureData(std::forward<T>(args)...);
	}

	TemplateArgs IFrameBufferPtr CreateFrameBuffer(Launch launchMode, T &&...args) ThreadSafe {
		auto res = mRenderSys.CreateResource(kDeviceResourceFrameBuffer); ResSetLaunch;
		res->SetLoaded(nullptr != mRenderSys.LoadFrameBuffer(res, std::forward<T>(args)...));
		return std::static_pointer_cast<IFrameBuffer>(res);
	}
private:
	RenderSystem& mRenderSys;
};

}
}