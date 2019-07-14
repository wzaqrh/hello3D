#pragma once
#include "TRenderSystem.h"
#include "TMesh.h"

bool CheckHR(HRESULT result);

HRESULT CompileShaderFromFile(const char* szFileName, const char* szEntryPoint, const char* szShaderModel, ID3DBlob** ppBlobOut);

extern std::string gModelPath;
std::string GetModelPath();
std::string MakeModelPath(const char* name);

HRESULT InputInit(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight);
HRESULT InputReadMouse();
void InputProcess();
void InputFrame();
void InputGetMouseLocation(int* px, int* py);

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