#pragma once
#include "TRenderSystem.h"
#include "TMesh.h"

bool CheckHR(HRESULT result);

HRESULT CompileShaderFromFile(const char* szFileName, const char* szEntryPoint, const char* szShaderModel, ID3DBlob** ppBlobOut);

extern std::string gModelPath;
std::string GetModelPath();
std::string MakeModelPath(const char* name);

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

class SDTimer {
	double mLastTime = 0.0;
public:
	double mDeltaTime = 0.0;
public:
	double Update();
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