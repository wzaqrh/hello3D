#pragma once
#include <windows.h>
#include <d3d11.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;
#include "core/base/math.h"
#include "core/rendersys/d3d11/predeclare.h"
#include "core/rendersys/program.h"

namespace mir {

class VertexShader11 : public ImplementResource<IVertexShader>
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	VertexShader11(IBlobDataPtr pBlob) :mBlob(pBlob) {}
	IBlobDataPtr GetBlob() const override { return mBlob; }

	ComPtr<ID3D11VertexShader>& GetShader11() { return mShader; }
public:
	ComPtr<ID3D11VertexShader> mShader = nullptr;
	IBlobDataPtr mBlob;
	ComPtr<ID3DBlob> mErrBlob = nullptr;
};

class PixelShader11 : public ImplementResource<IPixelShader>
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	PixelShader11(IBlobDataPtr pBlob) :mBlob(pBlob) {}
	IBlobDataPtr GetBlob() const override { return mBlob; }

	ComPtr<ID3D11PixelShader>& GetShader11() { return mShader; }
public:
	ComPtr<ID3D11PixelShader> mShader = nullptr;
	IBlobDataPtr mBlob;
	ComPtr<ID3DBlob> mErrBlob = nullptr;
};

class Program11 : public ImplementResource<IProgram>
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	Program11();
	void SetVertex(VertexShader11Ptr pVertex);
	void SetPixel(PixelShader11Ptr pPixel);
	IVertexShaderPtr GetVertex() const override { return mVertex; }
	IPixelShaderPtr GetPixel() const override { return mPixel; }
public:
	VertexShader11Ptr mVertex;
	PixelShader11Ptr mPixel;
};

}