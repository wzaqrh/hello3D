#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include "Utility.h"
#include "Model.h"
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
	if (FAILED(result))
	{
		return result;
	}

	if (0)
	{
		// Initialize the direct input interface for the keyboard.
		result = m_directInput->CreateDevice(GUID_SysKeyboard, &m_keyboard, NULL);
		if (FAILED(result))
		{
			return result;
		}

		// Set the data format.  In this case since it is a keyboard we can use the predefined data format.
		result = m_keyboard->SetDataFormat(&c_dfDIKeyboard);
		if (FAILED(result))
		{
			return result;
		}

		// Set the cooperative level of the keyboard to not share with other programs.
		result = m_keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
		if (FAILED(result))
		{
			return false;
		}
		//Once they keyboard is setup we then call Acquire to finally get access to the keyboard for use from this point forward.

		// Now acquire the keyboard.
		result = m_keyboard->Acquire();
		if (FAILED(result))
		{
			return false;
		}
	}

	{
		// Initialize the direct input interface for the mouse.
		result = m_directInput->CreateDevice(GUID_SysMouse, &m_mouse, NULL);
		if (FAILED(result))
		{
			return result;
		}

		// Set the data format for the mouse using the pre-defined mouse data format.
		result = m_mouse->SetDataFormat(&c_dfDIMouse);
		if (FAILED(result))
		{
			return result;
		}
		//We use non - exclusive cooperative settings for the mouse.We will have to check for when it goes in and out of focus and re - acquire it each time.

		// Set the cooperative level of the mouse to share with other programs.
		result = m_mouse->SetCooperativeLevel(hwnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);// DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
		if (FAILED(result))
		{
			return result;
		}
		//Once the mouse is setup we acquire it so that we can begin using it.

		// Acquire the mouse.
		result = m_mouse->Acquire();
		if (FAILED(result))
		{
			return result;
		}
	}
}

HRESULT InputReadMouse()
{
	HRESULT result;
	// Read the mouse device.
	result = m_mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&m_mouseState);
	if (FAILED(result))
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



HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* pErrorBlob;
	hr = D3DX11CompileFromFile(szFileName, NULL, NULL, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		if (pErrorBlob != NULL)
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
		if (pErrorBlob) pErrorBlob->Release();
		return hr;
	}
	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
}

std::string GetModelPath()
{
	return "D:\\project\\helloFBX\\Tutorial01\\Debug\\model\\";
}

ID3D11ShaderResourceView* CreateTexture(const char* pSrcFile)
{
	HRESULT hr = S_OK;
	ID3D11ShaderResourceView* pTextureRV = nullptr;
	hr = D3DX11CreateShaderResourceViewFromFileA(g_pd3dDevice, (GetModelPath() + pSrcFile).c_str(), NULL, NULL, &pTextureRV, NULL);
	if (FAILED(hr))
		return nullptr;
	return pTextureRV;
}

std::map<std::string, ID3D11ShaderResourceView*> TexByPath;
ID3D11ShaderResourceView* GetTexByPath(const std::string& __imgPath) {
	std::string imgPath = __imgPath;
	auto pos = __imgPath.find_last_of("\\");
	if (pos != std::string::npos) {
		imgPath = __imgPath.substr(pos + 1, std::string::npos);
	}

	ID3D11ShaderResourceView* texView = nullptr;
	if (TexByPath.find(imgPath) == TexByPath.end()) {
		texView = CreateTexture(imgPath.c_str());
		TexByPath.insert(std::make_pair(imgPath, texView));
	}
	else {
		texView = TexByPath[imgPath];
	}
	return texView;
}

std::string MakeModelPath(const char* name)
{
	return GetModelPath() + name;
}

bool UpdateBuffer(ID3D11Buffer* ubo, void* data, int dataSize)
{
	HRESULT hr = S_OK;
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	hr = (g_pImmediateContext->Map(ubo, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	if (FAILED(hr))
		return false;
	memcpy(MappedResource.pData, data, dataSize);
	g_pImmediateContext->Unmap(ubo, 0);
	return true;
}

ID3D11SamplerState* CreateSampler()
{
	HRESULT hr = S_OK;

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	
	ID3D11SamplerState* pSamplerLinear = nullptr;
	hr = g_pd3dDevice->CreateSamplerState(&sampDesc, &pSamplerLinear);
	if (FAILED(hr))
		return nullptr;
	return pSamplerLinear;
}

ID3D11PixelShader*  CreatePS()
{
	HRESULT hr = S_OK;

	// Compile the pixel shader
	ID3DBlob* pBlob = NULL;
	hr = CompileShaderFromFile(L"Light.fx", "PS", "ps_4_0", &pBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return nullptr;
	}

	ID3D11PixelShader* PixelShaderSolid = nullptr;
	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &PixelShaderSolid);
	pBlob->Release();
	if (FAILED(hr))
		return nullptr;

	return PixelShaderSolid;
}


ID3D11VertexShader* CreateVS(ID3DBlob*& pVSBlob)
{
	HRESULT hr = S_OK;

	// Compile the vertex shader
	pVSBlob = NULL;
	hr = CompileShaderFromFile(L"Light.fx", "VS", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return nullptr;
	}

	ID3D11VertexShader* pVertexShader = nullptr;
	hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &pVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return nullptr;
	}
	return pVertexShader;
}

ID3D11Buffer* CreateVBO(const std::vector<SimpleVertex>& Vertexs)
{
	HRESULT hr = S_OK;

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * Vertexs.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = &Vertexs[0];

	ID3D11Buffer* pVertexBuffer = nullptr;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &pVertexBuffer);
	if (FAILED(hr))
		return nullptr;
	return pVertexBuffer;
}

ID3D11Buffer* CreateVIO(const std::vector<short>& Indices)
{
	HRESULT hr = S_OK;

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * Indices.size();        // 36 vertices needed for 12 triangles in a triangle list
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = &Indices[0];

	ID3D11Buffer* pIndexBuffer = nullptr;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &pIndexBuffer);
	if (FAILED(hr))
		return nullptr;
	return pIndexBuffer;
}

ID3D11Buffer* CreateUBO(int width, int height, ConstantBuffer& constBuf)
{
	HRESULT hr = S_OK;

	ID3D11Buffer* pConstantBuffer = nullptr;
	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = g_pd3dDevice->CreateBuffer(&bd, NULL, &pConstantBuffer);
	if (FAILED(hr))
		return nullptr;

	// Initialize the world matrices
	constBuf.mWorld = XMMatrixIdentity();

	// Initialize the view matrix
	XMVECTOR Eye = XMVectorSet(0.0f, 0.0f, -10.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	constBuf.mView = XMMatrixLookAtLH(Eye, At, Up);

	// Initialize the projection matrix
	constBuf.mProjection = XMMatrixPerspectiveFovLH(XM_PIDIV4, width / (FLOAT)height, 0.01f, 100.0f);

	XMMATRIX mv = constBuf.mView * constBuf.mProjection;
	return pConstantBuffer;
}