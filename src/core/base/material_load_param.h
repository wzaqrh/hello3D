#pragma once
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "core/base/stl.h"
#include "core/rendersys/program.h"

namespace mir {

struct MaterialLoadParam 
{
	typedef std::string Hash;
	MaterialLoadParam(const char* shaderName) :ShaderVariantName(shaderName) {};
	MaterialLoadParam(const std::string& shaderName = "") :ShaderVariantName(shaderName) {}
	Hash MakeHash(const std::string& shaderVarName) const { return shaderVarName + ":" + GetVariantDesc(); }
	Hash GetHash() const { return ShaderVariantName + ":" + GetVariantDesc(); }
	std::string GetVariantDesc() const { return GetVariantName() + GetMacrosDesc(); }
	std::string GetMacrosDesc() const {
		std::string name;
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
	std::string GetVariantName() const {
		std::vector<boost::iterator_range<std::string::iterator>> strArr;
		boost::split(strArr, std::string(ShaderVariantName), boost::is_any_of("-"));
		if (strArr.size() >= 2) return boost::lexical_cast<std::string>(strArr[1]);
		else return "";
	}
	std::string GetShaderName() const { return boost::trim_right_copy_if(ShaderVariantName, boost::is_any_of("-")); }
	int operator[](const std::string& macroName) const {
		int result = 0;
		auto find_it = std::find_if(Macros.begin(), Macros.end(), [&](const ShaderCompileMacro& v) { return v.Name == macroName; });
		if (find_it != Macros.end()) result = std::stoi(find_it->Definition);
		return result;
	}
public:
	std::string ShaderVariantName;
	std::vector<ShaderCompileMacro> Macros;
};
inline bool operator==(const MaterialLoadParam& l, const MaterialLoadParam& r) {
	return l.ShaderVariantName == r.ShaderVariantName && l.Macros == r.Macros;
}
inline bool operator<(const MaterialLoadParam& l, const MaterialLoadParam& r) {
	if (l.ShaderVariantName != r.ShaderVariantName) return l.ShaderVariantName < r.ShaderVariantName;
	return l.Macros < r.Macros;
}

struct MaterialLoadParamBuilder 
{
	MaterialLoadParamBuilder(const char* shaderName = "") :LoadParam(shaderName) {}
	explicit MaterialLoadParamBuilder(const MaterialLoadParam& other) {
		LoadParam = other;
		for (const auto& it : other.Macros)
			MacroMap[it.Name] = std::stoi(it.Definition);
	}
	int& operator[](const std::string& macroName) { return MacroMap[macroName]; }
	void Merge(const MaterialLoadParamBuilder& other) {
		for (auto& it : other.MacroMap)
			MacroMap[it.first] = it.second;
	}

	const MaterialLoadParam& Build() const {
		LoadParam.Macros.clear();
		for (auto& it : MacroMap)
			LoadParam.Macros.push_back(ShaderCompileMacro{ it.first, boost::lexical_cast<std::string>(it.second) });
		return LoadParam;
	}
	operator MaterialLoadParam() const { return Build(); }
	std::string& ShaderVariantName() { return LoadParam.ShaderVariantName; }
private:
	mutable MaterialLoadParam LoadParam;
	std::map<std::string, int> MacroMap;
};

}