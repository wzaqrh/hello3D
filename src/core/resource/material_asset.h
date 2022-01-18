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
	const std::string& GetName() const { return ShortName; }
public:
	ConstBufferDecl Decl;
	std::vector<float> Data;
	bool IsUnique;
	int Slot;
	std::string ShortName;
};

struct SamplerDescEx : public SamplerDesc {
	bool IsValid() const { return !ShortName.empty(); }
	const std::string& GetName() const { return ShortName; }
	static SamplerDescEx Make(SamplerFilterMode filter, CompareFunc cmpFunc,
		AddressMode addrU, AddressMode addrV, AddressMode addrW, const std::string& shortName) {
		SamplerDescEx sd;
		sd.Filter = filter;
		sd.CmpFunc = cmpFunc;
		sd.AddressU = addrU;
		sd.AddressV = addrV;
		sd.AddressW = addrW;
		sd.ShortName = shortName;
		return sd;
	}
public:
	std::string ShortName;
};
struct SamplerNode : public VectorAdapter<SamplerDescEx> {};
}
}

template <> struct has_function_valid_t<res::mat_asset::AttributeNode> : public std::true_type {};

template <> struct has_function_valid_t<res::mat_asset::UniformNode> : public std::true_type {};
template <> struct has_function_name_t<res::mat_asset::UniformNode> : public std::true_type {};

template <> struct has_function_valid_t<res::mat_asset::SamplerDescEx> : public std::true_type {};
template <> struct has_function_name_t<res::mat_asset::SamplerDescEx> : public std::true_type {};

namespace res {
namespace mat_asset {
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
struct AttributeNodeVector : public VectorAdapter<AttributeNode> {};
struct UniformNodeVector : public VectorAdapter<UniformNode> {
	template<class Visitor> void ForEachUniform(Visitor vis) const {
		for (const auto& uniform : *this)
			vis(uniform);
	}
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
struct TechniqueNode : public VectorAdapter<PassNode> {
	template<class Visitor> void ForEachPass(Visitor vis) const {
		for (const auto& pass : *this)
			vis(pass);
	}
	template<class Visitor> void ForEachProgram(Visitor vis) const {
		for (const auto& pass : *this)
			vis(pass.Program);
	}
};
struct CategoryNode : public VectorAdapter<TechniqueNode> {
	ProgramNode Program;
	template<class Visitor> void ForEachPass(Visitor vis) const {
		for (const auto& tech : *this)
			tech.ForEachPass(vis);
	}
	template<class Visitor> void ForEachProgram(Visitor vis) const {
		vis(Program);
		for (const auto& tech : *this)
			tech.ForEachProgram(vis);
	}
};
struct ShaderNode : public VectorAdapter<CategoryNode> {
	ShaderNode() {
		Add(CategoryNode());
	}
	template<class Visitor> void ForEachPass(Visitor vis) const {
		for (const auto& categ : *this)
			categ.ForEachPass(vis);
	}
	template<class Visitor> void ForEachProgram(Visitor vis) const {
		for (const auto& categ : *this)
			categ.ForEachProgram(vis);
	}
public:
	std::string ShortName;
};
struct TextureProperty {
	std::string ImagePath;
	int Slot;
};
struct UniformProperty {
	std::vector<float> Data;
	bool IsUnique;
	int Slot;
};
struct MaterialNode {
	ShaderNode Shader;
	std::map<std::string, TextureProperty> TextureProperies;
	std::map<std::string, UniformProperty> UniformProperies;
};

class ShaderNodeManager;
class MaterialNodeManager;
class MaterialAssetManager : boost::noncopyable
{
public:
	MaterialAssetManager();
	bool GetShaderNode(const ShaderLoadParam& loadParam, ShaderNode& shaderNode);
	bool GetMaterialNode(const std::string& materialPath, MaterialNode& materialNode);
private:
	std::shared_ptr<ShaderNodeManager> mShaderNodeMng;
	std::shared_ptr<MaterialNodeManager> mMaterialNodeMng;
};

}
}
}