#pragma once

#include <windows.h>
#include <d3d11.h>
#include <D3DX11.h>
#include "TMesh.h"

extern HINSTANCE               g_hInst;
extern HWND                    g_hWnd;
extern D3D_DRIVER_TYPE         g_driverType;
extern D3D_FEATURE_LEVEL       g_featureLevel;
extern ID3D11Device*           g_pd3dDevice ;
extern ID3D11DeviceContext*    g_pImmediateContext ;
extern IDXGISwapChain*         g_pSwapChain ;
extern ID3D11RenderTargetView* g_pRenderTargetView ;
extern ID3D11Texture2D*        g_pDepthStencil ;
extern ID3D11DepthStencilView* g_pDepthStencilView ;
extern int g_ScreenWidth;
extern int g_ScreenHeight;


const int MAX_MATRICES = 256;
struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
	XMMATRIX mModel;
	XMMATRIX Models[MAX_MATRICES];
	XMFLOAT4 vLightDir[2];
	XMFLOAT4 vLightColor[2];
	XMFLOAT4 vOutputColor;
};

void OnRender();
void OnPreInitDevice();
void OnPostInitDevice();
void OnCleanUp();

void InitAll(int w, int h);