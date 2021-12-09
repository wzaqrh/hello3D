#pragma once
#include "core/base/stl.h"

namespace mir {

struct MaterialLoadParam {
	MaterialLoadParam(const std::string& shaderName, const std::string& variantName = "")
		:ShaderName(shaderName), VariantName(variantName) {}
	MaterialLoadParam(const char* shaderName)
		:MaterialLoadParam(shaderName, "") {}
public:
	std::string ShaderName;
	std::string VariantName;
};
inline bool operator==(const MaterialLoadParam& l, const MaterialLoadParam& r) {
	return l.ShaderName == r.ShaderName
		&& l.VariantName == r.VariantName;
}
inline bool operator<(const MaterialLoadParam& l, const MaterialLoadParam& r) {
	if (l.ShaderName != r.ShaderName) return l.ShaderName < r.ShaderName;
	return l.VariantName < r.VariantName;
}

}