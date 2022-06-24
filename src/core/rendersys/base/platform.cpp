#include <boost/lexical_cast.hpp>
#include "core/base/macros.h"
#include "core/rendersys/base/platform.h"

namespace mir {

std::string Platform::Name() const
{
	return IF_AND_OR(Type == kPlatformOpengl, "ogl", "d3d") + boost::lexical_cast<std::string>(Version);
}

std::string Platform::ShaderExtension() const
{
	return IF_AND_OR(Type == kPlatformOpengl, ".glsl", ".hlsl");
}

}