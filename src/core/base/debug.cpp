#include <Windows.h>
#include <dxerr.h>
#include <boost/assert.hpp>
#include "core/base/debug.h"

namespace mir {
namespace debug {

/********** TTimeProfile **********/
TimeProfile::TimeProfile(const std::string& name)
{
	mName = name;
	mCurTime = timeGetTime();
}

TimeProfile::~TimeProfile()
{
	char szBuf[260];
	sprintf(szBuf, "%s takes %d ms\n", mName.c_str(), timeGetTime() - mCurTime);
	OutputDebugStringA(szBuf);
}

/********** Timer **********/
double Timer::Update()
{
	double time = GetTickCount() / 1000.0;
	mDeltaTime = mLastTime == 0.0f ? 0.0 : time - mLastTime;
	mLastTime = time;
	return mDeltaTime;
}

/********** Functions **********/
bool CheckHResultFailed(HRESULT hr)
{
	if (FAILED(hr)) {
		Log(DXGetErrorDescriptionA(hr));
		Log(DXGetErrorStringA(hr));

		DXTRACE_ERR_MSGBOX(DXGetErrorDescription(hr), hr);
		DXTRACE_ERR_MSGBOX(DXGetErrorString(hr), hr);
		DXTRACE_ERR_MSGBOX(L"Clear failed!", hr); // Use customized error string
		DXTRACE_MSG(DXGetErrorDescription(hr));
		DXTRACE_ERR(DXGetErrorDescription(hr), hr);

		BOOST_ASSERT(false);
		return true;
	}
	return false;
}

void SetDebugName(ID3D11DeviceChild* child, const std::string& name)
{
	if (child != nullptr && name != "")
		child->SetPrivateData(WKPDID_D3DDebugObjectName, name.size(), name.c_str());
}

void Log(const char* msg)
{
	OutputDebugStringA(msg);
	OutputDebugStringA("\n");
}

#define CAPD(CLS) do { sprintf(buf, #CLS "=%d\n", caps.CLS); ss += buf; }while(0)
#define CAPX(CLS) do { sprintf(buf, #CLS "=0x%x\n", caps.CLS); ss += buf; }while(0)
void Log(const D3DCAPS9& caps)
{
	std::string ss = "d3d caps";
	char buf[4096];
	CAPD(DeviceType);
	CAPD(AdapterOrdinal);
	CAPD(Caps);
	CAPD(Caps2);
	CAPD(Caps3);
	CAPX(PresentationIntervals);//D3DPRESENT_INTERVAL_IMMEDIATE=8000000fx

	/* Cursor Caps */
	CAPX(CursorCaps);//D3DCURSORCAPS_COLOR=1

	/* 3D Device Caps */
	CAPX(DevCaps);

	CAPX(PrimitiveMiscCaps);//D3DDEVCAPS_CANBLTSYSTONONLOCAL
	CAPX(RasterCaps);
	CAPX(ZCmpCaps);
	CAPX(SrcBlendCaps);
	CAPX(DestBlendCaps);
	CAPX(AlphaCmpCaps);
	CAPX(ShadeCaps);
	CAPX(TextureCaps);
	CAPX(TextureFilterCaps);          // D3DPTFILTERCAPS for IDirect3DTexture9's
	CAPX(CubeTextureFilterCaps);      // D3DPTFILTERCAPS for IDirect3DCubeTexture9's
	CAPX(VolumeTextureFilterCaps);    // D3DPTFILTERCAPS for IDirect3DVolumeTexture9's
	CAPX(TextureAddressCaps);         // D3DPTADDRESSCAPS for IDirect3DTexture9's
	CAPX(VolumeTextureAddressCaps);   // D3DPTADDRESSCAPS for IDirect3DVolumeTexture9's

	CAPX(LineCaps);                   // D3DLINECAPS

	CAPD(MaxTextureWidth, MaxTextureHeight);
	CAPD(MaxVolumeExtent);

	CAPD(MaxTextureRepeat);
	CAPD(MaxTextureAspectRatio);
	CAPD(MaxAnisotropy);
	CAPD(MaxVertexW);

	CAPD(GuardBandLeft);
	CAPD(GuardBandTop);
	CAPD(GuardBandRight);
	CAPD(GuardBandBottom);

	CAPD(ExtentsAdjust);
	CAPX(StencilCaps);

	CAPX(FVFCaps);
	CAPX(TextureOpCaps);
	CAPD(MaxTextureBlendStages);
	CAPD(MaxSimultaneousTextures);

	CAPX(VertexProcessingCaps);
	CAPD(MaxActiveLights);
	CAPD(MaxUserClipPlanes);
	CAPD(MaxVertexBlendMatrices);
	CAPD(MaxVertexBlendMatrixIndex);

	CAPD(MaxPointSize);

	CAPD(MaxPrimitiveCount);          // max number of primitives per DrawPrimitive call
	CAPD(MaxVertexIndex);
	CAPD(MaxStreams);
	CAPD(MaxStreamStride);            // max stride for SetStreamSource

	CAPX(VertexShaderVersion);
	CAPD(MaxVertexShaderConst);       // number of vertex shader constant registers

	CAPX(PixelShaderVersion);
	CAPD(PixelShader1xMaxValue);      // max value storable in registers of ps.1.x shaders

	CAPD(DevCaps2);
	CAPD(MaxNpatchTessellationLevel);
	CAPD(Reserved5);
	CAPD(MasterAdapterOrdinal);       // ordinal of master adaptor for adapter group
	CAPD(AdapterOrdinalInGroup);      // ordinal inside the adapter group
	CAPD(NumberOfAdaptersInGroup);    // number of adapters in this adapter group (only if master)
	CAPD(DeclTypes);                  // Data types, supported in vertex declarations
	CAPD(NumSimultaneousRTs);         // Will be at least 1
	CAPX(StretchRectFilterCaps);      // Filter caps supported by StretchRect
	CAPX(VS20Caps);
	CAPX(PS20Caps);
	CAPX(VertexTextureFilterCaps);    // D3DPTFILTERCAPS for IDirect3DTexture9's for texture, used in vertex shaders
	CAPD(MaxVShaderInstructionsExecuted); // maximum number of vertex shader instructions that can be executed
	CAPD(MaxPShaderInstructionsExecuted); // maximum number of pixel shader instructions that can be executed
	CAPD(MaxVertexShader30InstructionSlots);
	CAPD(MaxPixelShader30InstructionSlots);

	Log(ss.c_str());
}

}
}