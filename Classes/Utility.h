#pragma once
#include "TRenderSystem.h"
#include "TMesh.h"

bool CheckHR(HRESULT result);

extern std::string gModelPath;
std::string GetModelPath();
std::string MakeModelPath(const char* name);

bool IsFileExist(const std::string& fileName);

class TD3DInputImpl;
class TD3DInput {
public:
	TD3DInput(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight);
	~TD3DInput();
	bool Init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight);
	void Frame();
	TINT4 GetMouseLocation(bool left);
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
	TINT4 mMouseL, mMouseR;
};

class TTimeProfile {
	std::string mName;
	unsigned int mCurTime;
public:
	TTimeProfile(const std::string& name);
	~TTimeProfile();
};
#define TIME_PROFILE(NAME) TTimeProfile NAME(#NAME)
#define TIME_PROFILE2(NAME1,NAME2) TTimeProfile NAME(#NAME1+(":"+NAME2))

class SDTimer {
	double mLastTime = 0.0;
public:
	double mDeltaTime = 0.0;
public:
	double Update();
};

struct TIncludeStdio : public ID3DInclude
{
	std::string mModelPath;
	std::vector<char> mBuffer;
	std::vector<std::string> mStrBuffer;
public:
	TIncludeStdio(const std::string& modelPath);
	STDMETHOD(Open)(THIS_ D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes);
	STDMETHOD(Close)(THIS_ LPCVOID pData);
};

aiMatrix4x4 FromXM(const XMMATRIX& m);
XMMATRIX ToXM(const aiMatrix4x4& m);
XMFLOAT3 ToXM(const aiVector3D& v);

XMFLOAT3 operator-(XMFLOAT3 lhs, XMFLOAT3 rht);
XMFLOAT3 operator+(XMFLOAT3 lhs, XMFLOAT3 rht);
XMFLOAT3 operator*(XMFLOAT3 lhs, float d);
XMFLOAT2 operator-(XMFLOAT2 lhs, XMFLOAT2 rht);

void OutPutMatrix(FILE* fd, const aiMatrix4x4& m);
void OutPutMatrix(FILE* fd, const XMMATRIX& m);

#define C_WINDOW_WIDTH 1024
#define C_WINDOW_HEIGHT 1024

#ifdef _DEBUG
void SetDebugName(ID3D11DeviceChild* child, const std::string& name);
#define SET_DEBUG_NAME(A,NAME)
#else
#define SET_DEBUG_NAME(A,NAME)
#endif