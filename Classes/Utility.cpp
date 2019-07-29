#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include "Utility.h"
#include <dinput.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

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

/********** Functions **********/
bool CheckHR(HRESULT result)
{
	if (FAILED(result)) {
		assert(false);
		return true;
	}
	return false;
}

HRESULT CompileShaderFromFile(const char* szFileName, const char* szEntryPoint, const char* szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* pErrorBlob;
	hr = D3DX11CompileFromFileA(szFileName, NULL, NULL, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		if (pErrorBlob != NULL)
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
		if (pErrorBlob) pErrorBlob->Release();
		CheckHR(hr);
		return hr;
	}
	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
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
