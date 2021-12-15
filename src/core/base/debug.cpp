#include <Windows.h>
#include <d3dcompiler.h>
#include <dxerr.h>
#include <boost/assert.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "core/base/debug.h"
#include "core/rendersys/blob.h"
#include "core/resource/resource.h"

namespace mir {
namespace debug {


/********** TTimeProfile **********/
TimeProfile::TimeProfile(const std::string& name)
{
	mName = name;
	mCurTime = timeGetTime();

	char szBuf[260];
	sprintf(szBuf, "%s : %d\n", mName.c_str(), mCurTime);
	OutputDebugStringA(szBuf);
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

/********** CheckHRXXX **********/
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

bool CheckCompileFailed(HRESULT hr, ID3DBlob* pErrorBlob)
{
	if (FAILED(hr)) 
	{
		if (pErrorBlob != NULL) 
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
			pErrorBlob = nullptr;
		}
		CheckHR(hr);
		return true;
	}
	return false;
}

bool CheckCompileFailed(HRESULT hr, IBlobDataPtr data)
{
	if (FAILED(hr)) 
	{
		ID3DBlob* pErrorBlob = nullptr;
		D3DGetDebugInfo(data->GetBytes(), data->GetSize(), &pErrorBlob);
		if (pErrorBlob != NULL) 
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
			pErrorBlob = nullptr;
		}
		CheckHR(hr);
		return true;
	}
	return false;
}

/********** SetDebugXXX **********/
static void SetChildrenPrivateData(const std::vector<void*>& children, const std::string& name)
{
	for (auto& child : children)
		if (child) static_cast<ID3D11DeviceChild*>(child)->SetPrivateData(WKPDID_D3DDebugObjectName, name.size(), name.c_str());
}

void ResourceAddDebugDevice(IResourcePtr res, void* device)
{
#if defined MIR_RESOURCE_DEBUG
	if (res) {
		res->_Debug._DeviceChilds.push_back(device);
		SetChildrenPrivateData(res->_Debug._DeviceChilds, res->_Debug.GetDebugInfo());
	}
#endif
}

void SetDebugPrivData(IResourcePtr res, const std::string& privData)
{
#if defined MIR_RESOURCE_DEBUG
	if (res) {
		res->_Debug._PrivData = (privData);
		SetChildrenPrivateData(res->_Debug._DeviceChilds, res->_Debug.GetDebugInfo());
	}
#endif
}

void SetDebugResourcePath(IResourcePtr res, const std::string& resPath)
{
#if defined MIR_RESOURCE_DEBUG
	if (res) {
		res->_Debug._ResourcePath = (resPath);
		SetChildrenPrivateData(res->_Debug._DeviceChilds, res->_Debug.GetDebugInfo());
	}
#endif
}

void SetDebugCallStack(IResourcePtr res, const std::string& callstack)
{
#if defined MIR_RESOURCE_DEBUG
	if (res) {
		res->_Debug._CallStack = (callstack);
		SetChildrenPrivateData(res->_Debug._DeviceChilds, res->_Debug.GetDebugInfo());
	}
#endif
}

/********** Log **********/
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