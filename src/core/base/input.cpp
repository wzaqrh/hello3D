#include <boost/algorithm/clamp.hpp>
#include "core/base/input.h"
#include "core/base/debug.h"

namespace mir {
namespace input {

/********** TD3DInput **********/
D3DInput::D3DInput(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight)
{
	memset(m_keyboardState, 0, sizeof(m_keyboardState));
	memset(&m_mouseState, 0, sizeof(m_mouseState));
	memset(&mMouseL, 0, sizeof(mMouseL));
	memset(&mMouseR, 0, sizeof(mMouseR));
	mMouseWheel = 0;
	mMouseMiddleDown = false;
	Init(hinstance, hwnd, screenWidth, screenHeight);
}
D3DInput::~D3DInput()
{}

bool D3DInput::Init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight)
{
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	if (CheckHR(DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_directInput, NULL)))
		return false;

	bool result = true;
	//�����¼�
	if (!CheckHR(m_directInput->CreateDevice(GUID_SysKeyboard, &m_keyboard, NULL))
		&& !CheckHR(m_keyboard->SetDataFormat(&c_dfDIKeyboard))
		&& !CheckHR(m_keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE))) {
		m_keyboard->Acquire();
	}
	else {
		result = false;
	}

	//����¼�
	if (!CheckHR(m_directInput->CreateDevice(GUID_SysMouse, &m_mouse, NULL))
		&& !CheckHR(m_mouse->SetDataFormat(&c_dfDIMouse))
		&& !CheckHR(m_mouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE))) {
		m_mouse->Acquire();
	}
	else {
		result = false;
	}

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
	if (m_mouseState.rgbButtons[0] & 0x80) {//������
		mMouseL.x() = boost::algorithm::clamp(mMouseL.x() + m_mouseState.lX, -m_screenWidth, m_screenWidth);
		mMouseL.y() = boost::algorithm::clamp(mMouseL.y() + m_mouseState.lY, -m_screenHeight, m_screenHeight);
	}
	else if (m_mouseState.rgbButtons[1] & 0x80) {//����Ҽ�
		mMouseR.x() = boost::algorithm::clamp(mMouseR.x() + m_mouseState.lX, -m_screenWidth, m_screenWidth);
		mMouseR.y() = boost::algorithm::clamp(mMouseR.y() + m_mouseState.lY, -m_screenHeight, m_screenHeight);
	}
	mMouseWheel = m_mouseState.lZ;
	mMouseMiddleDown = (m_mouseState.rgbButtons[2] & 0x80);
}

void D3DInput::Frame()
{
	if (ReadMouse()) {
		Process();
	}
}

/********** Functions **********/
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
	}
	return ret;
}

}
}