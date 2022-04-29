#pragma once
#include <boost/noncopyable.hpp>
#include "core/base/cppcoro.h"
#include "core/base/declare_macros.h"
#include "core/base/tpl/binary.h"
#include "core/base/material_load_param.h"
#include "core/rendersys/hardware_buffer.h"
#include "core/rendersys/sampler.h"
#include "core/rendersys/input_layout.h"
#include "core/resource/material_parameter.h"

namespace mir {
namespace res {
namespace mat_asset {

#if defined _DEBUG
inline bool ValidateResult(bool res) {
	BOOST_ASSERT(res);
	return res;
}
#define VR(V) ValidateResult(V)
#else
#define VR(V) V
#endif

struct AttributeNode {
	bool IsValid() const { return !Layout.empty(); }
public:
	std::vector<LayoutInputElement> Layout;
};
using UniformNode = UniformParameters;
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
struct SamplerNode : public tpl::Vector<SamplerDescEx> {};
}
}

template <> struct tpl::has_function_valid<res::mat_asset::AttributeNode> : public std::true_type {};
template <> struct tpl::has_function_valid<res::mat_asset::SamplerDescEx> : public std::true_type {};
template <> struct tpl::has_function_name<res::mat_asset::SamplerDescEx> : public std::true_type {};

namespace res {
namespace mat_asset {
struct ShaderCompileDescEx : public ShaderCompileDesc {
	template<bool Override> void AddMacro(const ShaderCompileMacro& macro) {
		auto find_it = std::find_if(this->Macros.begin(), this->Macros.end(), [&macro](const ShaderCompileMacro& elem)->bool {
			return elem.Name == macro.Name;
		});
		if (find_it == this->Macros.end()) {
			this->Macros.push_back(macro);
		}
		else {
			if constexpr (Override) find_it->Definition = macro.Definition;
		}
	}

	template<bool Override> void AddMacros(const std::vector<ShaderCompileMacro>& macros) {
		for (const auto& it : macros)
			AddMacro<Override>(it);
	}

	template<bool Override> void Merge(const ShaderCompileDesc& other) { 
		EntryPoint = other.EntryPoint;
		ShaderModel = other.ShaderModel;
		SourcePath = other.SourcePath;
		AddMacros<true>(other.Macros);
	}
	template<> void Merge<false>(const ShaderCompileDesc& other) {
		if (EntryPoint.empty()) EntryPoint = other.EntryPoint;
		if (ShaderModel.empty()) ShaderModel = other.ShaderModel;
		if (SourcePath.empty()) SourcePath = other.SourcePath;
		AddMacros<false>(other.Macros);
	}

	bool Validate() const {
		return VR(!EntryPoint.empty() 
			//&& !ShaderModel.empty() 
			&& !SourcePath.empty());
	}
	int operator[](const std::string& key) const {
		int value = 0;
		auto find_it = std::find_if(this->Macros.begin(), this->Macros.end(), [&key](const ShaderCompileMacro& elem)->bool {
			return elem.Name == key;
		});
		if (find_it != this->Macros.end()) return atoi(find_it->Definition.c_str());
		else return 0;
	}
};
struct AttributeNodeVector : public tpl::Vector<AttributeNode> {};
struct UniformNodeVector : public tpl::Vector<UniformNode> {
	template<class Visitor> void ForEachUniform(Visitor vis) const {
		for (const auto& uniform : *this)
			vis(uniform);
	}
};

struct ProgramNode {
	template<bool Override> void Merge(const ProgramNode& other) { static_assert(false, ""); }
	template<> void Merge<true>(const ProgramNode& other) {
		Topo = other.Topo;
		Attrs.Adds(other.Attrs);
		Uniforms.Merge<true>(other.Uniforms);
		Samplers.Merge<true>(other.Samplers);
		VertexSCD.Merge<true>(other.VertexSCD);
		PixelSCD.Merge<true>(other.PixelSCD);
	}
	void Build() {
		if (Topo == kPrimTopologyUnkown)
			Topo = kPrimTopologyTriangleList;
	}
	bool Validate() const {
		for (auto& attr : Attrs)
			if (!VR(attr.IsValid()))
				return false;
	#if 0
		return VR((Topo != kPrimTopologyUnkown) && 
			(Attrs.Count() > 0) &&
			VertexSCD.Validate() &&
			PixelSCD.Validate());
	#else
		return VR(Attrs.Count() > 0);
	#endif
	}
public:
	PrimitiveTopology Topo = kPrimTopologyTriangleList;
	AttributeNodeVector Attrs;
	UniformNodeVector Uniforms;
	SamplerNode Samplers;
	ShaderCompileDescEx VertexSCD, PixelSCD;
};
struct PassNode {
	bool Validate() const {
		return VR(Program.Validate()
			&& !LightMode.empty()
			&& !Name.empty()
		/* && !ShortName.empty() */);
	}
public:
	ProgramNode Program;
	std::string LightMode, Name, ShortName;
	struct GrabOutputNode {
		std::string Name;
		std::vector<ResourceFormat> Format;
	} GrabOut;
	struct GrabInputNode {
		std::string Name;
		int AttachIndex = 0;
		int TextureSlot = 0;
	} GrabIn;
};
struct TechniqueNode : public tpl::Vector<PassNode> {
	template<class Visitor> void ForEachPass(Visitor vis) const {
		for (const auto& pass : *this)
			vis(pass);
	}
	template<class Visitor> void ForEachProgram(Visitor vis) const {
		for (const auto& pass : *this)
			vis(pass.Program);
	}
	bool Validate() const {
		for (const auto& pass : *this)
			if (!VR(pass.Validate()))
				return false;
		return true;
	}
};
struct CategoryNode : public tpl::Vector<TechniqueNode> {
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
	bool Validate() const {
		if (!VR(Program.Validate())) 
			return false;
		for (const auto& tech : *this)
			if (!VR(tech.Validate()))
				return false;
		return true;
	}
	int GetMacrosDefinition(const MaterialLoadParam& loadParam, const std::string& key) const {
		int value = 0; loadParam[key];
		if (value == 0) {
			ForEachProgram([&key, &value](const ProgramNode& prog) {
				value = __max(value, prog.VertexSCD[key]);
			});
		}
		return value;
	}
};
struct ShaderNode : public tpl::Vector<CategoryNode> {
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
	bool Validate() const {
		for (const auto& categ : *this)
			if (!VR(categ.Validate()))
				return false;
		return VR(!ShortName.empty());
	}
public:
	std::string ShortName;
};
struct TextureProperty {
	std::string ImagePath;
	int Slot;
};
struct MaterialNode {
	MaterialLoadParam LoadParam;
	std::string MaterialFilePath;
	ShaderNode Shader;
	std::map<std::string, TextureProperty> TextureProperies;
	std::map<std::string, std::string> UniformProperies; 
};

class ShaderNodeManager;
class MaterialNodeManager;
class MaterialAssetManager : boost::noncopyable
{
public:
	MaterialAssetManager();
	bool GetShaderNode(const MaterialLoadParam& loadParam, ShaderNode& shaderNode) ThreadSafe;
	bool GetMaterialNode(const MaterialLoadParam& loadParam, MaterialNode& materialNode) ThreadSafe;
private:
	std::shared_ptr<ShaderNodeManager> mShaderNodeMng;
	std::shared_ptr<MaterialNodeManager> mMaterialNodeMng;
};

}
}
}