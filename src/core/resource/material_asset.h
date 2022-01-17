#pragma once
#include <boost/noncopyable.hpp>
#include "core/base/material_load_param.h"

namespace mir {
namespace res {
namespace mat_asset {

struct AttributeNode {
	bool IsValid() const { return !Layout.empty(); }
public:
	std::vector<LayoutInputElement> Layout;
};
struct UniformNode {
	bool IsValid() const { return !Data.empty(); }
public:
	ConstBufferDecl Decl;
	std::string ShortName;
	std::vector<float> Data;
	bool IsUnique;
	int Slot;
};
struct SamplerNode : public VectorAdapterEx<SamplerDesc> {
	SamplerNode() :VectorAdapterEx<SamplerDesc>([](const SamplerDesc& v) {return v.CmpFunc != kCompareUnkown; }) {}
};

struct ShaderCompileDescEx : public ShaderCompileDesc {
	void AddOrSetMacro(const ShaderCompileMacro& macro) {
		auto find_it = std::find_if(this->Macros.begin(), this->Macros.end(), [&macro](const ShaderCompileMacro& elem)->bool {
			return elem.Name == macro.Name;
		});
		if (find_it == this->Macros.end()) this->Macros.push_back(macro);
		else find_it->Definition = macro.Definition;
	}
	void AddOrSetMacros(const std::vector<ShaderCompileMacro>& macros) {
		for (const auto& it : macros)
			AddOrSetMacro(it);
	}
	void AddMacro(const ShaderCompileMacro& macro) {
		auto find_it = std::find_if(this->Macros.begin(), this->Macros.end(), [&macro](const ShaderCompileMacro& elem)->bool {
			return elem.Name == macro.Name;
		});
		if (find_it == this->Macros.end()) this->Macros.push_back(macro);
	}
	void AddMacros(const std::vector<ShaderCompileMacro>& macros) {
		for (const auto& it : macros)
			AddMacro(it);
	}
	void MergeNoOverride(const ShaderCompileDesc& other) {
		if (EntryPoint.empty()) EntryPoint = other.EntryPoint;
		if (ShaderModel.empty()) ShaderModel = other.ShaderModel;
		if (SourcePath.empty()) SourcePath = other.SourcePath;
		AddMacros(other.Macros);
	}
};
struct AttributeNodeVector : public VectorAdapterEx<AttributeNode> {
	AttributeNodeVector() :VectorAdapterEx<AttributeNode>([](const AttributeNode& v) {return v.IsValid(); }) {}
};
struct UniformNodeVector : public VectorAdapterEx<UniformNode> {
	UniformNodeVector() :VectorAdapterEx<UniformNode>([](const UniformNode& v) {return v.IsValid(); }) {}
};
struct ProgramNode {
	void MergeNoOverride(const ProgramNode& other) {
		Topo = other.Topo;
		Attrs.MergeNoOverride(other.Attrs);
		Uniforms.MergeNoOverride(other.Uniforms);
		Samplers.MergeNoOverride(other.Samplers);
		VertexSCD.MergeNoOverride(other.VertexSCD);
		PixelSCD.MergeNoOverride(other.PixelSCD);
	}
	void Build() {
		if (Topo == kPrimTopologyUnkown)
			Topo = kPrimTopologyTriangleList;
	}
public:
	PrimitiveTopology Topo = kPrimTopologyUnkown;
	AttributeNodeVector Attrs;
	UniformNodeVector Uniforms;
	SamplerNode Samplers;
	ShaderCompileDescEx VertexSCD, PixelSCD;
};

struct PassNode {
	ProgramNode Program;
	std::string LightMode, Name, ShortName;
};
struct TechniqueNode : public VectorAdapter<PassNode> {};
struct CategoryNode : public VectorAdapter<TechniqueNode> {
	ProgramNode Program;
};
struct ShaderNode : public VectorAdapter<CategoryNode> {
	ShaderNode() {
		Add(CategoryNode());
	}
};
struct MaterialAsset {
	ShaderNode Shader;
};

class ShaderNodeManager;
class MaterialAssetManager : boost::noncopyable
{
public:
	MaterialAssetManager();
	bool GetShaderNode(const ShaderLoadParam& loadParam, ShaderNode& shaderNode);
private:
	std::shared_ptr<ShaderNodeManager> mShaderNodeMng;
};

}
}
}