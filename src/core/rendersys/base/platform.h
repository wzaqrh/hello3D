#pragma once
#include <string>

namespace mir {

enum PlatformType 
{
	kPlatformDirectx,
	kPlatformOpengl
};

struct Platform
{
public:
	std::string Name() const;
	std::string ShaderExtension() const;
	bool IsNDCDepth01() const;
	bool SupportMTResCreation() const;
	bool SupportShaderIncMacroAndMultiEntry() const;
public:
	PlatformType Type;
	int Version;
};

}