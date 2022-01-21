#pragma once
#include <boost/lexical_cast.hpp>
#include "core/base/stl.h"
#include "core/rendersys/program.h"

namespace mir {

struct MaterialLoadParam 
{
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
inline bool operator==(const MaterialLoadParam& l, const MaterialLoadParam& r) {
	return l.ShaderName == r.ShaderName
		&& l.VariantName == r.VariantName
		&& l.Macros == r.Macros;
}
inline bool operator<(const MaterialLoadParam& l, const MaterialLoadParam& r) {
	if (l.ShaderName != r.ShaderName) return l.ShaderName < r.ShaderName;
	if (l.VariantName != r.VariantName) return l.VariantName < r.VariantName;
	return l.Macros < r.Macros;
}

struct MaterialLoadParamBuilder 
{
	MaterialLoadParamBuilder(const std::string& shaderName = "", const std::string& variantName = "") :LoadParam(shaderName, variantName) {}
	int& operator[](const std::string& macroName) { return MacroMap[macroName]; }
	std::string& ShaderName() { return LoadParam.ShaderName; }
	std::string& VariantName() { return LoadParam.VariantName; }
	const MaterialLoadParam& Build() {
		LoadParam.Macros.clear();
		for (auto& it : MacroMap)
			LoadParam.Macros.push_back(ShaderCompileMacro{ it.first, boost::lexical_cast<std::string>(it.second) });
		return LoadParam;
	}
	operator MaterialLoadParam() { return Build(); }
private:
	MaterialLoadParam LoadParam;
	std::map<std::string, int> MacroMap;
};

}