#include <windows.h>
#include <dinput.h>
#include <dxerr.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <boost/algorithm/clamp.hpp>
#include "core/base/utility.h"

#if _MSC_VER > 1800
#pragma comment(lib, "legacy_stdio_definitions.lib")
#endif
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "d3d9.lib")
#ifdef DEBUG
#pragma comment(lib, "d3dx9d.lib")
#else
#pragma comment(lib, "d3dx9.lib")
#endif

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d11.lib")
#ifdef DEBUG
#pragma comment(lib, "d3dx11d.lib")
#pragma comment(lib, "d3dx10d.lib")
#else
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")
#endif // DEBUG
#pragma comment(lib, "dxerr.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "assimp-vc140-mt.lib")
#pragma comment(lib, "freetype.lib")

namespace mir {

/********** TD3DInput **********/
D3DInput::D3DInput(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight)
{
	memset(m_keyboardState, 0, sizeof(m_keyboardState));
	memset(&m_mouseState, 0, sizeof(m_mouseState));
	memset(&mMouseL,0,sizeof(mMouseL));
	memset(&mMouseR, 0, sizeof(mMouseR));
	Init(hinstance, hwnd, screenWidth, screenHeight);
}
D3DInput::~D3DInput()
{

}

bool D3DInput::Init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight)
{
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	if (CheckHR(DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_directInput, NULL)))
		return false;

	bool result = true;
	//键盘事件
#if 1
		if (!CheckHR(m_directInput->CreateDevice(GUID_SysKeyboard, &m_keyboard, NULL))
			&& !CheckHR(m_keyboard->SetDataFormat(&c_dfDIKeyboard))
			&& !CheckHR(m_keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE))) {
			m_keyboard->Acquire();
		}
		else {
			result = false;
		}
#endif

	//鼠标事件
#if 1
		if (!CheckHR(m_directInput->CreateDevice(GUID_SysMouse, &m_mouse, NULL))
			&& !CheckHR(m_mouse->SetDataFormat(&c_dfDIMouse))
			&& !CheckHR(m_mouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE))) {
			m_mouse->Acquire();
		}
		else {
			result = false;
		}
#endif
	return result;
}

bool D3DInput::ReadKeyboard()
{
	HRESULT result = m_keyboard->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&m_mouseState);
	if (FAILED(result)) {
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED)) {
			m_keyboard->Acquire();
		}
	}
	return result == S_OK;
}

bool D3DInput::ReadMouse()
{
	HRESULT result = m_mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&m_mouseState);
	if (FAILED(result)) {
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED)) {
			m_mouse->Acquire();
		}
	}
	return result == S_OK;
}

void D3DInput::Process()
{
	if (m_mouseState.rgbButtons[0] & 0x80) {//鼠标左键
		mMouseL.x = boost::algorithm::clamp(mMouseL.x + m_mouseState.lX, -m_screenWidth, m_screenWidth);
		mMouseL.y = boost::algorithm::clamp(mMouseL.y + m_mouseState.lY, -m_screenHeight, m_screenHeight);
		mMouseL.z += m_mouseState.lZ;
	}
	else if (m_mouseState.rgbButtons[1] & 0x80) {//鼠标右键
		mMouseR.x = boost::algorithm::clamp(mMouseR.x + m_mouseState.lX, -m_screenWidth, m_screenWidth);
		mMouseR.y = boost::algorithm::clamp(mMouseR.y + m_mouseState.lY, -m_screenHeight, m_screenHeight);
	}
	else {

	}
}

void D3DInput::Frame()
{
	if (ReadMouse()) {
		Process();
	}
}

Int4 D3DInput::GetMouseLocation(bool left)
{
	Int4 ret = left ? mMouseL : mMouseR;
	return ret;
}

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

IncludeStdIo::IncludeStdIo(const std::string& modelPath)
	:mModelPath(modelPath)
{
}

/********** TIncludeStdio **********/
STDMETHODIMP IncludeStdIo::Open(THIS_ D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
{
	std::string fullPath = mModelPath + pFileName;
	FILE* fd = fopen(fullPath.c_str(), "r");
	if (fd) {
		fseek(fd, 0, SEEK_END);
		size_t size = ftell(fd);
		fseek(fd, 0, SEEK_SET);
		size_t first = mBuffer.size();
		mBuffer.resize(first + size);
		fread(&mBuffer[first], sizeof(char), size, fd);
		fclose(fd);

		//if (ppData) *ppData = &mBuffer[first];
		//if (pBytes) *pBytes = mBuffer.size();

		mStrBuffer.push_back(std::string(mBuffer.begin(), mBuffer.end()));
		if (ppData) *ppData = mStrBuffer.back().c_str();
		if (pBytes) *pBytes = mStrBuffer.back().size();
		
		OutputDebugStringA(mStrBuffer.back().c_str());

		return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP IncludeStdIo::Close(THIS_ LPCVOID pData)
{
	return S_OK;
}

std::vector<char> ReadFile(const char* fileName, const char* mode)
{
	std::vector<char> ret;
	FILE* fd = fopen(fileName, "rb");
	if (fd) {
		fseek(fd, 0, SEEK_END);
		size_t size = ftell(fd);
		rewind(fd);

		size_t first = ret.size();
		ret.resize(first + size);
		fread(&ret[first], sizeof(char), size, fd);
		fclose(fd);
		
		//std::string str(ret.begin(), ret.end());
		//OutputDebugStringA(str.c_str());
	}
	return ret;
}


/********** Functions **********/
bool CheckHR(HRESULT hr)
{
	if (FAILED(hr)) {
		__log(DXGetErrorDescriptionA(hr));
		__log(DXGetErrorStringA(hr));

		DXTRACE_ERR_MSGBOX(DXGetErrorDescription(hr), hr);
		DXTRACE_ERR_MSGBOX(DXGetErrorString(hr), hr);
		DXTRACE_ERR_MSGBOX(L"Clear failed!", hr); // Use customized error string
		DXTRACE_MSG(DXGetErrorDescription(hr));
		DXTRACE_ERR(DXGetErrorDescription(hr), hr);

		assert(false);
		return true;
	}
	return false;
}

std::string GetCurDirectory()
{
	std::string wstr;
	unsigned long size = GetCurrentDirectoryA(0, NULL);
	std::vector<char> path;
	path.resize(size);
	char* szPath = &path[0];
	if (GetCurrentDirectoryA(size, szPath) != 0)
		wstr = szPath;
	return wstr;
}

std::string gModelPath;
void SetModelPath(const std::string& modelPath) {
	gModelPath = modelPath;
}
std::string GetModelPath() {
	return GetCurDirectory() + "\\model\\" + gModelPath;
}
std::string MakeModelPath(const char* name) {
	return GetModelPath() + name;
}


double Timer::Update()
{
	double time = GetTickCount() / 1000.0;
	mDeltaTime = mLastTime == 0.0f ? 0.0 : time - mLastTime;
	mLastTime = time;
	return mDeltaTime;
}

void SetDebugName(ID3D11DeviceChild* child, const std::string& name)
{
	if (child != nullptr && name != "")
		child->SetPrivateData(WKPDID_D3DDebugObjectName, name.size(), name.c_str());
}

void __log(const char* msg)
{
	OutputDebugStringA(msg);
	OutputDebugStringA("\n");
}

#define CAPD(CLS) do { sprintf(buf, #CLS "=%d\n", caps.CLS); ss += buf; }while(0)
#define CAPX(CLS) do { sprintf(buf, #CLS "=0x%x\n", caps.CLS); ss += buf; }while(0)
void __log(const D3DCAPS9& caps)
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

	__log(ss.c_str());
}

}