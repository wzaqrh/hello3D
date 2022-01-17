#pragma once
#include <boost/lexical_cast.hpp>
#include "core/base/stl.h"
#include "core/base/base_type.h"

namespace mir {

struct ShaderLoadParam {
	ShaderLoadParam(const char* shaderName)
		:ShaderLoadParam(shaderName, "") {}
	ShaderLoadParam(const std::string& shaderName, const std::string& variantName = "")
		:ShaderLoadParam(shaderName, variantName, std::vector<ShaderCompileMacro>{}) {}
	ShaderLoadParam(const std::string& shaderName, const std::string& variantName, const std::vector<ShaderCompileMacro>& macros)
		:ShaderName(shaderName), VariantName(variantName), Macros(macros) {}
public:
	bool IsVariant() const { return !VariantName.empty() || !Macros.empty(); }
	std::string CalcVariantName() const {
		std::string name = VariantName;
		if (!Macros.empty()) {
			name += "(";
			int count = 0;
			for (auto& it : Macros) {
				if (count++ != 0) name += " ";
				name += it.Name + "=" + it.Definition;
			}
			name += ")";
		}
		return name;
	}
	int operator[](const std::string& macroName) const {
		int result = 0;
		auto find_it = std::find_if(Macros.begin(), Macros.end(), [&](const ShaderCompileMacro& v) { return v.Name == macroName; });
		if (find_it != Macros.end()) result = boost::lexical_cast<int>(find_it->Definition);
		return result;
	}
public:
	std::string ShaderName;
	std::string VariantName;
	std::vector<ShaderCompileMacro> Macros;
};
inline bool operator==(const ShaderLoadParam& l, const ShaderLoadParam& r) {
	return l.ShaderName == r.ShaderName
		&& l.VariantName == r.VariantName
		&& l.Macros == r.Macros;
}
inline bool operator<(const ShaderLoadParam& l, const ShaderLoadParam& r) {
	if (l.ShaderName != r.ShaderName) return l.ShaderName < r.ShaderName;
	if (l.VariantName != r.VariantName) return l.VariantName < r.VariantName;
	return l.Macros < r.Macros;
}

struct ShaderLoadParamBuilder {
	ShaderLoadParamBuilder(const std::string& shaderName, const std::string& variantName = "")
		:LoadParam(shaderName, variantName) {}
	int& operator[](const std::string& macroName) {
		return MacroMap[macroName];
	}
	std::string& ShaderName() {
		return LoadParam.ShaderName;
	}
	std::string& VariantName() {
		return LoadParam.VariantName;
	}
	operator ShaderLoadParam() {
		return Build();
	}
	const ShaderLoadParam& Build() {
		LoadParam.Macros.clear();
		for (auto& it : MacroMap)
			LoadParam.Macros.push_back(ShaderCompileMacro{ it.first, boost::lexical_cast<std::string>(it.second) });
		return LoadParam;
	}
private:
	ShaderLoadParam LoadParam;
	std::map<std::string, int> MacroMap;
};

}