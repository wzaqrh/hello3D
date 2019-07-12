#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include "Utility.h"
#include <dinput.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

IDirectInput8* m_directInput;
IDirectInputDevice8* m_keyboard;
IDirectInputDevice8* m_mouse;

unsigned char m_keyboardState[256];
DIMOUSESTATE m_mouseState;

int m_screenWidth, m_screenHeight;
int m_mouseX, m_mouseY;

HRESULT InputInit(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight)
{
	HRESULT result;

	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	// Initialize the main direct input interface.
	result = DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_directInput, NULL);
	if (CheckHR(result))
		return result;

	if (0)
	{
		// Initialize the direct input interface for the keyboard.
		result = m_directInput->CreateDevice(GUID_SysKeyboard, &m_keyboard, NULL);
		if (CheckHR(result))
			return result;

		// Set the data format.  In this case since it is a keyboard we can use the predefined data format.
		result = m_keyboard->SetDataFormat(&c_dfDIKeyboard);
		if (CheckHR(result))
			return result;

		// Set the cooperative level of the keyboard to not share with other programs.
		result = m_keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
		if (CheckHR(result))
			return false;
		//Once they keyboard is setup we then call Acquire to finally get access to the keyboard for use from this point forward.

		// Now acquire the keyboard.
		result = m_keyboard->Acquire();
		if (CheckHR(result))
			return false;
	}

	{
		// Initialize the direct input interface for the mouse.
		result = m_directInput->CreateDevice(GUID_SysMouse, &m_mouse, NULL);
		if (CheckHR(result))
			return result;

		// Set the data format for the mouse using the pre-defined mouse data format.
		result = m_mouse->SetDataFormat(&c_dfDIMouse);
		if (CheckHR(result))
			return result;
		//We use non - exclusive cooperative settings for the mouse.We will have to check for when it goes in and out of focus and re - acquire it each time.

		// Set the cooperative level of the mouse to share with other programs.
		result = m_mouse->SetCooperativeLevel(hwnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);// DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
		if (CheckHR(result))
			return result;
		//Once the mouse is setup we acquire it so that we can begin using it.

		// Acquire the mouse.
		result = m_mouse->Acquire();
		if (CheckHR(result))
			return result;
	}
	return result;
}

HRESULT InputReadMouse()
{
	HRESULT result;
	// Read the mouse device.
	result = m_mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&m_mouseState);
	if (CheckHR(result))
	{
		// If the mouse lost focus or was not acquired then try to get control back.
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
		{
			m_mouse->Acquire();
		}
		else
		{
			return result;
		}
	}

	return S_OK;
}

void InputProcess()
{
	// Update the location of the mouse cursor based on the change of the mouse location during the frame.
	if (m_mouseState.rgbButtons[0] & 0x80) {
		m_mouseX += m_mouseState.lX;
		m_mouseY += m_mouseState.lY;

		m_mouseX = min(m_screenWidth,max(-m_screenWidth, m_mouseX));
		m_mouseY = min(m_screenHeight, max(-m_screenHeight, m_mouseY));
	}
	else {
		/*m_mouseX = 0;
		m_mouseY = 0;*/
	}
}

void InputFrame()
{
	if (S_OK == InputReadMouse()) {
		InputProcess();
	}
}

void InputGetMouseLocation(int* px, int* py)
{
	*px = m_mouseX;
	*py = m_mouseY;
}


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
	if (CheckHR(hr))
	{
		if (pErrorBlob != NULL)
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
		if (pErrorBlob) pErrorBlob->Release();
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
