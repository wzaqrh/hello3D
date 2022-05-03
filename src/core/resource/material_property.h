#pragma once
#include <boost/noncopyable.hpp>
#include "core/predeclare.h"
#include "core/base/base_type.h"

namespace mir {
namespace res {

class PassProperty 
{
public:
	operator bool() const { return !LightMode.empty() && !Name.empty(); }
public:
	std::string LightMode, Name, ShortName;
	PrimitiveTopology TopoLogy;
	struct GrabOutput {
		operator bool() const { return !Name.empty(); }
		std::string Name;
		std::vector<ResourceFormat> Formats;
		float Size = 1.0f;
	} GrabOut;
	struct GrabInputUnit {
		operator bool() const { return !Name.empty(); }
		std::string Name;
		int AttachIndex = 0;
		int TextureSlot = 0;
	};
	typedef std::vector<GrabInputUnit> GrabInput;
	GrabInput GrabIn;
	struct ParameterRelation {
		ParameterRelation() :TextureSizes(16), HasTextureSize(false) {}
		std::vector<std::string> TextureSizes;
		bool HasTextureSize;
	} Relate2Parameter;
};

}
}