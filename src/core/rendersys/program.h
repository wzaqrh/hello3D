#pragma once
#include "core/rendersys/predeclare.h"
#include "core/resource/resource.h"

namespace mir {

struct ShaderCompileMacro {
	std::string Name, Definition;
};
struct ShaderCompileDesc {
	std::vector<ShaderCompileMacro> Macros;
	std::string EntryPoint, ShaderModel, SourcePath;
	int ShaderType;
};
inline bool operator==(const ShaderCompileMacro& l, const ShaderCompileMacro& r) {
	return l.Name == r.Name
		&& l.Definition == r.Definition;
}
inline bool operator<(const ShaderCompileMacro& l, const ShaderCompileMacro& r) {
	if (l.Name != r.Name) return l.Name < r.Name;
	return l.Definition < r.Definition;
}
inline bool operator==(const ShaderCompileDesc& l, const ShaderCompileDesc& r) {
	return l.EntryPoint == r.EntryPoint
		&& l.ShaderModel == r.ShaderModel
		&& l.SourcePath == r.SourcePath
		&& l.Macros == r.Macros;
}
inline bool operator<(const ShaderCompileDesc& l, const ShaderCompileDesc& r) {
	if (l.EntryPoint != r.EntryPoint) return l.EntryPoint < r.EntryPoint;
	if (l.ShaderModel != r.ShaderModel) return l.ShaderModel < r.ShaderModel;
	if (l.SourcePath != r.SourcePath) return l.SourcePath < r.SourcePath;
	return l.Macros < r.Macros;
}

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