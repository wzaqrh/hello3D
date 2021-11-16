#pragma once
#include <windows.h>
#include <dinput.h>
#include "core/base/std.h"
#include "core/mir_export.h"

namespace mir {
namespace input {

class MIR_CORE_API D3DInput {
public:
	D3DInput(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight);
	~D3DInput();
	bool Init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight);
	void Frame();
	Eigen::Vector4i GetMouseLeftLocation() const { return mMouseL; }
	Eigen::Vector4i GetMouseRightLocation() const { return mMouseR; }
private:
	bool ReadKeyboard();
	bool ReadMouse();
	void Process();
private:
	IDirectInput8* m_directInput = nullptr;
	IDirectInputDevice8* m_keyboard = nullptr;
	IDirectInputDevice8* m_mouse = nullptr;

	unsigned char m_keyboardState[256];
	DIMOUSESTATE m_mouseState;

	long m_screenWidth = 0, m_screenHeight = 0;
	Eigen::Vector4i mMouseL, mMouseR;
};

std::vector<char> ReadFile(const char* fileName, const char* mode);

}
}