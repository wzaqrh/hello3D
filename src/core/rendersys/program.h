#pragma once
#include "core/rendersys/predeclare.h"
#include "core/resource/resource.h"

namespace mir {

enum ShaderType {
	kShaderVertex,
	kShaderPixel
};

interface IShader : public IResource
{
	virtual ShaderType GetType() const = 0;
	virtual IBlobDataPtr GetBlob() const = 0;
};

interface IVertexShader : public IShader
{
	ShaderType GetType() const override final { return kShaderVertex; }
};

interface IPixelShader : public IShader
{
	ShaderType GetType() const override final { return kShaderPixel; }
};

interface IProgram : public IResource
{
	virtual IVertexShaderPtr GetVertex() const = 0;
	virtual IPixelShaderPtr GetPixel() const = 0;
};

}