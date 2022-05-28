#pragma once
#include <boost/noncopyable.hpp>
#include <boost/filesystem.hpp>
#include "core/rendersys/base/primitive_topology.h"
#include "core/rendersys/base/blend_state.h"
#include "core/rendersys/base/depth_state.h"
#include "core/rendersys/base/rasterizer_state.h"

namespace mir {
namespace res {

class PassProperty 
{
public:
	operator bool() const { return (! LightMode.empty()) && (! Name.empty()); }
public:
	std::string LightMode, Name, ShortName;
	//int LightMode = -1;
	PrimitiveTopology TopoLogy;
	std::optional<BlendState> Blend;
	std::optional<DepthState> Depth;
	std::optional<FillMode> Fill;
	std::optional<CullMode> Cull;
	std::optional<DepthBias> DepthBias;

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

struct MaterialProperty 
{	
public:
	struct TextureProperty {
		std::string ImagePath;
		int Slot;
		bool GenMipmap = false;
	};
	std::map<std::string, TextureProperty> Textures;

	std::map<std::string, std::string> UniformByName;
	
	struct SingleFileDependency {
		bool CheckOutOfDate() const { 
			boost::filesystem::path path(FilePath);
			if (boost::filesystem::is_regular_file(path) && FileTime < boost::filesystem::last_write_time(path)) return true;
			else return false; 
		}
		bool operator<(const SingleFileDependency& other) const { return FilePath < other.FilePath; }
		std::string FilePath;
		time_t FileTime = 0;
	};
	struct SourceFilesDependency {
	public:
		void AddShader(const SingleFileDependency& shader) {
			Shaders.insert(shader);
		}
		void Merge(const SourceFilesDependency& other) {
			for (auto& it : other.Shaders)
				Shaders.insert(it);
		}
		bool CheckOutOfDate() const {
			if (Material.CheckOutOfDate()) 
				return true;
			for (auto& it : Shaders)
				if (it.CheckOutOfDate())
					return true;
			return false;
		}
	public:
		std::set<SingleFileDependency> Shaders;
		SingleFileDependency Material;
	} DependSrc;
};

}
}