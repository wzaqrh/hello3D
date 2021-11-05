#include "core/base/utility.h"
#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include <dinput.h>

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
TD3DInput::TD3DInput(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight)
{
	memset(m_keyboardState, 0, sizeof(m_keyboardState));
	memset(&m_mouseState, 0, sizeof(m_mouseState));
	memset(&mMouseL,0,sizeof(mMouseL));
	memset(&mMouseR, 0, sizeof(mMouseR));
	Init(hinstance, hwnd, screenWidth, screenHeight);
}
TD3DInput::~TD3DInput()
{

}

bool TD3DInput::Init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight)
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

bool TD3DInput::ReadKeyboard()
{
	HRESULT result = m_keyboard->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&m_mouseState);
	if (FAILED(result)) {
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED)) {
			m_keyboard->Acquire();
		}
	}
	return result == S_OK;
}

bool TD3DInput::ReadMouse()
{
	HRESULT result = m_mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&m_mouseState);
	if (FAILED(result)) {
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED)) {
			m_mouse->Acquire();
		}
	}
	return result == S_OK;
}

void TD3DInput::Process()
{
	if (m_mouseState.rgbButtons[0] & 0x80) {//鼠标左键
		mMouseL.x = clamp(-m_screenWidth, m_screenWidth, mMouseL.x + m_mouseState.lX);
		mMouseL.y = clamp(-m_screenHeight, m_screenHeight, mMouseL.y + m_mouseState.lY);
		mMouseL.z += m_mouseState.lZ;
	}
	else if (m_mouseState.rgbButtons[1] & 0x80) {//鼠标右键
		mMouseR.x = clamp(-m_screenWidth, m_screenWidth, mMouseR.x + m_mouseState.lX);
		mMouseR.y = clamp(-m_screenHeight, m_screenHeight, mMouseR.y + m_mouseState.lY);
	}
	else {

	}
}

void TD3DInput::Frame()
{
	if (ReadMouse()) {
		Process();
	}
}

TINT4 TD3DInput::GetMouseLocation(bool left)
{
	TINT4 ret = left ? mMouseL : mMouseR;
	return ret;
}

/********** TTimeProfile **********/
TTimeProfile::TTimeProfile(const std::string& name)
{
	mName = name;
	mCurTime = timeGetTime();
}

TTimeProfile::~TTimeProfile()
{
	char szBuf[260]; 
	sprintf(szBuf, "%s takes %d ms\n", mName.c_str(), timeGetTime() - mCurTime);
	OutputDebugStringA(szBuf);
}

TIncludeStdio::TIncludeStdio(const std::string& modelPath)
	:mModelPath(modelPath)
{
}

/********** TIncludeStdio **********/
STDMETHODIMP TIncludeStdio::Open(THIS_ D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
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

STDMETHODIMP TIncludeStdio::Close(THIS_ LPCVOID pData)
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
std::string GetModelPath() {
	return GetCurDirectory() + "\\model\\" + gModelPath;
}
std::string MakeModelPath(const char* name) {
	return GetModelPath() + name;
}

std::string GetFileExt(const std::string& fileName)
{
	std::string ext;
	size_t pos = fileName.find_last_of(".");
	if (pos != std::string::npos) {
		ext = fileName.substr(pos + 1);
	}
	return ext;
}

bool IsFileExist(const std::string& fileName)
{
	WIN32_FIND_DATAA wfd;
	HANDLE hFind = FindFirstFileA(fileName.c_str(), &wfd);
	if (INVALID_HANDLE_VALUE != hFind) {
		return true;
	}
	return false;
}

double SDTimer::Update()
{
	double time = GetTickCount() / 1000.0;
	mDeltaTime = mLastTime == 0.0f ? 0.0 : time - mLastTime;
	mLastTime = time;
	return mDeltaTime;
}

void OutPutMatrix(FILE* fd, const aiMatrix4x4& m) {
	if (fd == nullptr) return;

	fprintf(fd, "%.3f %.3f %.3f %.3f\n", m.a1, m.a2, m.a3, m.a4);
	fprintf(fd, "%.3f %.3f %.3f %.3f\n", m.b1, m.b2, m.b3, m.b4);
	fprintf(fd, "%.3f %.3f %.3f %.3f\n", m.c1, m.c2, m.c3, m.c4);
	fprintf(fd, "%.3f %.3f %.3f %.3f\n", m.d1, m.d2, m.d3, m.d4);

	fprintf(fd, "\n\n");
	fflush(fd);
}

void OutPutMatrix(FILE* fd, const XMMATRIX& m) {
	if (fd == nullptr) return;

	for (int i = 0; i < 4; ++i)
		fprintf(fd, "%.3f %.3f %.3f %.3f\n", m.m[i][0], m.m[i][1], m.m[i][2], m.m[i][3]);
	fprintf(fd, "\n\n");
	fflush(fd);
}

XMMATRIX ToXM(const aiMatrix4x4& m) {
	XMMATRIX r;
	static_assert(sizeof(r) == sizeof(m), "");
	aiMatrix4x4 mm = m;
	memcpy(&r, &mm, sizeof(mm));
	return r;
}

XMFLOAT3 ToXM(const aiVector3D& v)
{
	XMFLOAT3 r = XMFLOAT3(v.x, v.y, v.z);
	return r;
}

aiMatrix4x4 FromXM(const XMMATRIX& m) {
	aiMatrix4x4 r;
	static_assert(sizeof(r) == sizeof(m), "");
	memcpy(&r, &m, sizeof(m));
	r.Transpose();
	return r;
}

XMFLOAT3 operator-(XMFLOAT3 lhs, XMFLOAT3 rht) {
	XMFLOAT3 ret;
	ret.x = lhs.x - rht.x;
	ret.y = lhs.y - rht.y;
	ret.z = lhs.z - rht.z;
	return ret;
}
XMFLOAT3 operator+(XMFLOAT3 lhs, XMFLOAT3 rht) {
	XMFLOAT3 ret;
	ret.x = lhs.x + rht.x;
	ret.y = lhs.y + rht.y;
	ret.z = lhs.z + rht.z;
	return ret;
}
XMFLOAT3 operator*(XMFLOAT3 lhs, float d) {
	XMFLOAT3 ret;
	ret.x = lhs.x * d;
	ret.y = lhs.y * d;
	ret.z = lhs.z * d;
	return ret;
}
XMFLOAT2 operator-(XMFLOAT2 lhs, XMFLOAT2 rht) {
	XMFLOAT2 ret;
	ret.x = lhs.x - rht.x;
	ret.y = lhs.y - rht.y;
	return ret;
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

XMMATRIX XM::Inverse(const XMMATRIX& m)
{
	XMVECTOR det = XMMatrixDeterminant(COPY_TO_GPU(m));
	XMMATRIX ret = COPY_TO_GPU(XMMatrixInverse(&det, m));
	return ret;
}

}