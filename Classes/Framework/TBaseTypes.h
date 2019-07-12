#pragma once
#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include <dinput.h>
#include <dxerr.h>

#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <assert.h>

struct TCamera {
	XMMATRIX mView;
	XMMATRIX mProjection;
public:
	TCamera(int width, int height);
};
typedef std::shared_ptr<TCamera> TCameraPtr;

struct TMaterial {
	ID3D11VertexShader* mVertexShader = nullptr;
	ID3D11PixelShader* mPixelShader = nullptr;
	ID3D11InputLayout* mInputLayout = nullptr;
	
	D3D11_PRIMITIVE_TOPOLOGY mTopoLogy;
	ID3D11SamplerState*	mSampler = nullptr;

	//std::vector<ID3D11ShaderResourceView*> mTextures;
	//ID3D11Buffer* mVertexBuffer = nullptr;
	//ID3D11Buffer* mIndexBuffer = nullptr;
	ID3D11Buffer* mConstantBuffer = nullptr;
};
typedef std::shared_ptr<TMaterial> TMaterialPtr;
