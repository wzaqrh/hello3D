#pragma once

#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>

#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <assert.h>

#include "Model.h"

HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

std::string GetModelPath();
std::string MakeModelPath(const char* name);

ID3D11ShaderResourceView* CreateTexture(const char* pSrcFile);
ID3D11ShaderResourceView* GetTexByPath(const std::string& imgPath);

bool UpdateBuffer(ID3D11Buffer* ubo, void* data, int dataSize);

ID3D11SamplerState* CreateSampler();
ID3D11PixelShader*  CreatePS();
ID3D11VertexShader* CreateVS(ID3DBlob*& pVSBlob);
ID3D11Buffer* CreateVBO(const std::vector<SimpleVertex>& Vertexs);
ID3D11Buffer* CreateVIO(const std::vector<short>& Indices);
ID3D11Buffer* CreateUBO(int width, int height, ConstantBuffer& constBuf);

HRESULT InputInit(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight);
HRESULT InputReadMouse();
void InputProcess();
void InputFrame();
void InputGetMouseLocation(int* px, int* py);