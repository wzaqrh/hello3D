#pragma once
#include <windows.h>
#include <dinput.h>
#include "core/mir_export.h"
#include "core/rendersys/base_type.h"
#include "core/base/d3d_enum_convert.h"

namespace mir {

bool CheckHR(HRESULT result);

MIR_CORE_API std::string GetCurDirectory();
MIR_CORE_API void SetModelPath(const std::string& modelPath);
MIR_CORE_API std::string GetModelPath();
MIR_CORE_API std::string MakeModelPath(const char* name);struct Int4 {
	int x, y, z, w;
};
class MIR_CORE_API D3DInput {
public:
	D3DInput(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight);
	~D3DInput();
	bool Init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight);
	void Frame();
	Int4 GetMouseLocation(bool left);
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
	Int4 mMouseL, mMouseR;
};

class MIR_CORE_API TimeProfile {
	std::string mName;
	unsigned int mCurTime;
public:
	TimeProfile(const std::string& name);
	~TimeProfile();
};
#define TIME_PROFILE(NAME) TimeProfile NAME(#NAME)
#define TIME_PROFILE2(NAME1,NAME2) TimeProfile NAME(#NAME1+(":"+NAME2))

class MIR_CORE_API Timer {
	double mLastTime = 0.0;
public:
	double mDeltaTime = 0.0;
public:
	double Update();
};

struct IncludeStdIo : public ID3DInclude
{
	std::string mModelPath;
	std::vector<char> mBuffer;
	std::vector<std::string> mStrBuffer;
public:
	IncludeStdIo(const std::string& modelPath);
	STDMETHOD(Open)(THIS_ D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes);
	STDMETHOD(Close)(THIS_ LPCVOID pData);
};

std::vector<char> ReadFile(const char* fileName, const char* mode);

#define COPY_TO_GPU(M) (M)

#define C_WINDOW_WIDTH 1024
#define C_WINDOW_HEIGHT 768

#ifdef _DEBUG
void SetDebugName(ID3D11DeviceChild* child, const std::string& name);
#define SET_DEBUG_NAME(A,NAME)
#else
#define SET_DEBUG_NAME(A,NAME)
#endif

void __log(const char* msg);
void __log(const D3DCAPS9& caps);

}