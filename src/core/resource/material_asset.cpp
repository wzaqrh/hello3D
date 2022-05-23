#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <regex>
#include "core/mir_config_macros.h"
#include "core/base/stl.h"
#include "core/base/tpl/atomic_map.h"
#include "core/base/tpl/vector.h"
#include "core/base/macros.h"
#include "core/base/d3d.h"
#include "core/resource/material_name.h"
#include "core/resource/material_asset.h"

namespace boost_filesystem = boost::filesystem;
namespace boost_property_tree = boost::property_tree;

namespace mir {
namespace res {
namespace mat_asset {

/********** ShaderNodeManager **********/
class ShaderNodeManager
{
	friend class MaterialNodeManager;
public:
	ShaderNodeManager() {}
	bool GetShaderNode(const MaterialLoadParam& loadParam, ShaderNode& shaderNode) ThreadSafe {
		bool result = GetShaderVariantNode(loadParam, shaderNode);
		BOOST_ASSERT(shaderNode[0].Program.Topo != kPrimTopologyUnkown);
		return result;
	}
	bool PurgeOutOfDates() ThreadSafe {
		tpl::AutoLock lck(mShaderVariantByParam._GetLock());
		for (auto& it : mShaderByParam) {
			if (it.second.DependShaders.CheckOutOfDate()) {
				mShaderVariantByParam._Clear();
				mShaderByParam.clear();
				mIncludeByName.clear();
				mAttrByName.clear();
				mUniformByName.clear();
				mSamplerSetByName.clear();
				mIncludeFiles.Clear();
				return true;
			}
		}
		return false;
	}
private:
	struct Visitor {
		int GetMacroValue(const ProgramNode& progNode, const std::string& key) const {
			int value = LoadParam[key];
			value = IF_AND_OR(value, value, PredMacros[key]);
			value = IF_AND_OR(value, value, progNode.VertexSCD[key]);
			return value;
		}
		bool CheckCondition(const ProgramNode& progNode, const boost_property_tree::ptree& node, bool* pHasCondition = nullptr) const {
			bool result = true;
			std::string condition = node.get<std::string>("<xmlattr>.Condition", "");
			if (!condition.empty()) {
				if (pHasCondition)
					*pHasCondition = true;
				const std::regex exp_regex("([a-zA-Z0-9_]+)([<>=]+)([0-9]+)");
				std::smatch exp_match;
				if (std::regex_match(condition, exp_match, exp_regex) && exp_match.size() == 4) {
					std::string macro_name = exp_match[1];
					int left_value = GetMacroValue(progNode, macro_name);

					std::string compare = exp_match[2];

					std::string str_right_value = exp_match[3];
					int right_value = atoi(str_right_value.c_str());

					if (compare == "==") result = (left_value == right_value);
					else if (compare == "<=") result = (left_value <= right_value);
					else if (compare == ">=") result = (left_value >= right_value);
					else if (compare == "<") result = (left_value < right_value);
					else if (compare == ">") result = (left_value > right_value);
				}
			}
			return result;
		}
		template<typename T> T ConditionGetValue(const ProgramNode& progNode, const boost_property_tree::ptree& node, const std::string& name, T defvalue = T()) const {
			for (auto& it : boost::make_iterator_range(node.equal_range(name))) {
				auto& inode = it.second;
				if (CheckCondition(progNode, inode)) {
					return static_cast<T>(std::stoi(inode.data()));
				}
			}
			return defvalue;
		}
		template<> std::string ConditionGetValue<std::string>(const ProgramNode& progNode, const boost_property_tree::ptree& node, const std::string& name, std::string defvalue) const {
			for (auto& it : boost::make_iterator_range(node.equal_range(name))) {
				auto& inode = it.second;
				if (CheckCondition(progNode, inode)) {
					return inode.data();
				}
			}
			return defvalue;
		}
	public:
		const bool JustInclude;
		const MaterialLoadParam& LoadParam;
		MaterialLoadParamBuilder& PredMacros;
	};
	using ConstVisitorRef = const Visitor&;
	
	struct PropertyTreePath {
		boost_property_tree::ptree Node;
		boost_filesystem::path Path;
		int Index;
		PropertyTreePath(const boost_property_tree::ptree& node) :Node(node), Index(0) {
			Path = node.get<std::string>("<xmlattr>.Name", "0");
		}
		PropertyTreePath(const PropertyTreePath& parent, const boost_property_tree::ptree& node, int index) :Node(node), Index(index) {
			Path = parent.Path;
			Path.append(node.get<std::string>("<xmlattr>.Name", boost::lexical_cast<std::string>(index)));
		}
		const boost_property_tree::ptree* operator->() const {
			return &Node;
		}
	};

	struct IncludeFiles {
		void Clear() {
			mDic.clear();
		}
		void GetFileDependecies(const std::string& shadername, MaterialProperty::SourceFilesDependency& cgincs) {
			boost_filesystem::path filepath = boost_filesystem::system_complete("shader/" + shadername + ".hlsl");
			const auto& incs = GetFileIncludes(filepath);
			for (const auto& it : incs)
				cgincs.AddShader(it);
		}
		const std::set<MaterialProperty::SingleFileDependency>& GetFileIncludes(const boost_filesystem::path& filepath) {
			std::string pathstr = filepath.string();
			auto find_iter = mDic.find(pathstr);
			if (find_iter == mDic.end()) {
				std::set<MaterialProperty::SingleFileDependency> includes;
				bool exist = boost_filesystem::is_regular_file(filepath);
				includes.insert(MaterialProperty::SingleFileDependency{ pathstr, exist ? boost::filesystem::last_write_time(filepath) : 0 });

				if (exist) {
					std::ifstream fs;
					fs.open(pathstr, std::ios::in);
					std::string line;
					const std::regex exp_regex("#include\\s+\"([a-zA-Z0-9_\\.]+)\".*");
					while (fs.peek() != EOF) {
						std::getline(fs, line);
						std::smatch exp_match;
						if (std::regex_match(line, exp_match, exp_regex) && exp_match.size() == 2) {
							std::string incname = exp_match[1].str();
							boost_filesystem::path incpath = boost_filesystem::system_complete("shader/" + incname);
							const auto& incincs = GetFileIncludes(incpath);
							for (const auto& it : incincs)
								includes.insert(it);
						}
					}
					fs.close();
				}

				mDic.insert(std::make_pair(pathstr, std::move(includes)));
				find_iter = mDic.find(pathstr);
			}
			return find_iter->second;
		}
		std::map<std::string, std::set<MaterialProperty::SingleFileDependency>> mDic;
	};

	template<typename T> static T GetNodeAttribute(const std::string& str, T defValue, const std::vector<std::tuple<std::string, std::string, int>>& patterns) {
		if ((str.size() >= 2 && str[0] == '0' && str[1] == 'x' && std::all_of(str.begin() + 2, str.end(), isxdigit)) || (!str.empty() && std::all_of(str.begin(), str.end(), isdigit))) {
			return static_cast<T>(std::stoi(str));
		}
		else {
			auto iter = std::find_if(patterns.begin(), patterns.end(), [&](const std::tuple<std::string, std::string, int>& item) {
				return std::get<1>(item) == str;
			});
			if (iter != patterns.end()) return static_cast<T>(std::get<2>(*iter));
			else return defValue;
		}
	}
	template<typename T> static T GetNodeAttribute(const boost_property_tree::ptree& node, const std::string& attrName, T defValue, const std::vector<std::tuple<std::string, std::string, int>>& patterns) {
		std::string str = node.get<std::string>(attrName, "");
		if (!str.empty()) {
			return GetNodeAttribute<T>(str, defValue, patterns);
		}
		else {
			bool hasValue = false;
			int value = 0;
			for (auto& it : node) {
				auto iter = std::find_if(patterns.begin(), patterns.end(), [&](const std::tuple<std::string, std::string, int>& item) {
					return (std::get<0>(item) == it.first) && (std::get<1>(item) == it.second.data());
					});
				if (iter != patterns.end()) {
					value |= std::get<2>(*iter);
					hasValue = true;
				}
			}
			return IF_AND_OR(hasValue, static_cast<T>(value), defValue);
		}
	}
private:
	ShaderNode& VisitInclude(const std::string& shaderName) {
		auto find_iter = mIncludeByName.find(shaderName);
		if (find_iter == mIncludeByName.end()) {
			ShaderNode shaderNode;
			MaterialLoadParam loadParam(shaderName);
			ParseShaderFile(loadParam, shaderNode);
			mIncludeByName.insert(std::make_pair(shaderName, std::move(shaderNode)));
		}
		return mIncludeByName[shaderName];
	}

	static bool ParseMacrosFile(const std::string& macroName, ConstVisitorRef vis) {
		bool result = false;
		boost_filesystem::path path = boost_filesystem::system_complete("shader/" + macroName + ".cginc");
		if (boost::filesystem::is_regular_file(path)) {
			std::ifstream fs;
			fs.open(path.string(), std::ios::in);
			std::string line;
			const std::regex exp_regex(R"(#define\s+([a-zA-Z0-9_]+)\s+([0-9]+).*)");
			while (fs.peek() != EOF) {
				std::getline(fs, line);
				std::smatch exp_match;
				if (std::regex_match(line, exp_match, exp_regex) && exp_match.size() == 3) {
					std::string key = exp_match[1].str(), value = exp_match[2].str();
					vis.PredMacros[key] = std::stoi(value);
				}
			}
			fs.close();
			result = true;
		}
		return result;
	}
	static void SplitString(std::vector<std::string>& strArr, const std::string& str, const std::string& strAny) {
		strArr.clear();
		boost::split(strArr, str, boost::is_any_of(strAny), boost::token_compress_on);
		if (strArr.back().empty()) strArr.pop_back();
	}
	static void ParseProgram(const boost_property_tree::ptree& nodeProgram, ConstVisitorRef vis, ProgramNode& progNode) {//no_override
		if (!vis.CheckCondition(progNode, nodeProgram))
			return;

		for (auto& it : boost::make_iterator_range(nodeProgram.equal_range("PredMacros"))) {
			std::string refName = it.second.data();
			if (!refName.empty()) {
				ParseMacrosFile(refName, vis);
			}
		}

		auto& vertexScd = progNode.VertexSCD;
		vertexScd.SourcePath = nodeProgram.get<std::string>("FileName", vertexScd.SourcePath);
		vertexScd.EntryPoint = nodeProgram.get<std::string>("VertexEntry", vertexScd.EntryPoint);

		auto& pixelScd = progNode.PixelSCD;
		pixelScd.SourcePath = vertexScd.SourcePath;
		pixelScd.EntryPoint = nodeProgram.get<std::string>("PixelEntry", pixelScd.EntryPoint);

		auto find_macros = nodeProgram.find("Macros");
		if (find_macros != nodeProgram.not_found()) {
			auto& node_macros = find_macros->second;
			for (auto& it : node_macros) {
				vertexScd.AddMacro<true>(ShaderCompileMacro{ it.first, it.second.data() });
				pixelScd.AddMacro<true>(ShaderCompileMacro{ it.first, it.second.data() });
			}
		}

		{
			std::string strTopo = vis.ConditionGetValue<std::string>(progNode, nodeProgram, "Topology", "");
			if (!strTopo.empty()) {
				static std::vector<std::tuple<std::string, std::string, int>> topoPatterns = {
					{"", "PointList", kPrimTopologyPointList},
					{"", "LineList", kPrimTopologyLineList},
					{"", "TriangleList", kPrimTopologyTriangleList},

					{"", "LineListAdj", kPrimTopologyLineListAdj},
					{"", "TriangleListAdj", kPrimTopologyTriangleListAdj},

					{"", "LineStrip", kPrimTopologyLineStrip},
					{"", "TriangleStrip", kPrimTopologyTriangleStrip},

					{"", "LineStripAdj", kPrimTopologyLineStripAdj},
					{"", "TriangleStripAdj", kPrimTopologyTriangleStripAdj},
				};
				progNode.Topo = GetNodeAttribute<PrimitiveTopology>(strTopo, progNode.Topo, topoPatterns);
			}
			
			std::string strBlendFunc = nodeProgram.get<std::string>("BlendFunc", "");
			if (!strBlendFunc.empty()) {
				std::vector<std::string> strs;
				SplitString(strs, strBlendFunc, ",");
				if (strs.size() >= 2) {
					static std::vector<std::tuple<std::string, std::string, int>> blendPatterns = {
						{"", "One", kBlendZero},
						{"", "Zero", kBlendOne},
						{"", "Src_Color", kBlendSrcColor},
						{"", "One_Minus_SrcColor", kBlendInvSrcColor},
						{"", "Src_A", kBlendSrcAlpha},
						{"", "One_Minus_SrcA", kBlendInvSrcAlpha},
						{"", "Dst_A", kBlendDstAlpha},
						{"", "One_Minus_DstA", kBlendInvDstAlpha},
						{"", "Dst_Color", kBlendDstColor},
						{"", "One_Minus_DstColor", kBlendInvDstColor}
					};
					BlendFunc src = GetNodeAttribute<BlendFunc>(strs[0], kBlendOne, blendPatterns);
					BlendFunc dst = GetNodeAttribute<BlendFunc>(strs[1], kBlendZero, blendPatterns);
					progNode.Blend = BlendState::Make(src, dst);
				}
			}

			std::string strDepthBias = nodeProgram.get<std::string>("DepthBias", "");
			if (!strDepthBias.empty()) {
				std::vector<std::string> strs;
				SplitString(strs, strDepthBias, ",");
				if (strs.size() >= 2) {
					progNode.DepthBias = DepthBias::Make(std::stof(strs[0]), std::stof(strs[1]));
				}
			}

			std::string strDepthEnable = nodeProgram.get<std::string>("DepthEnable", "");
			std::string strDepthFunc = nodeProgram.get<std::string>("DepthFunc", "");
			std::string strDepthWriteMask = nodeProgram.get<std::string>("DepthWriteMask", "");
			if (!strDepthEnable.empty() || !strDepthFunc.empty() || !strDepthWriteMask.empty()) {
				static std::vector<std::tuple<std::string, std::string, int>> compFuncPatterns = {
					{"", "Never", kCompareNever},
					{"", "Less", kCompareLess},
					{"", "Equal", kCompareEqual},
					{"", "LessEqual", kCompareLessEqual},
					{"", "Greater", kCompareGreater},
					{"", "NotEqual", kCompareNotEqual},
					{"", "GreaterEqual", kCompareGreaterEqual},
					{"", "Always", kCompareAlways},
				};
				CompareFunc compFunc = GetNodeAttribute<CompareFunc>(strDepthFunc, kCompareLess, compFuncPatterns);

				static std::vector<std::tuple<std::string, std::string, int>> zWriteMaskPatterns = {
					{"", "All", kDepthWriteMaskAll},
					{"", "Zero", kDepthWriteMaskZero},
				};
				DepthWriteMask zWriteMask = GetNodeAttribute<DepthWriteMask>(strDepthWriteMask, kDepthWriteMaskAll, zWriteMaskPatterns);

				static std::vector<std::tuple<std::string, std::string, int>> boolPatterns = {
					{"", "True", TRUE},
					{"", "true", TRUE},
					{"", "False", FALSE},
					{"", "false", FALSE},
				};
				bool depthEnable = GetNodeAttribute<bool>(strDepthEnable, true, boolPatterns);

				progNode.Depth = DepthState::Make(compFunc, zWriteMask, depthEnable);
			}

			static std::vector<std::tuple<std::string, std::string, int>> fillPatterns = {
				{"", "WireFrame", kFillWireFrame},
				{"", "Solid", kFillSolid},
			};
			FillMode fillMode = GetNodeAttribute<FillMode>(nodeProgram, "FillMode", kFillUnkown, fillPatterns);
			if (fillMode != kFillUnkown) 
				progNode.Fill = fillMode;

			static std::vector<std::tuple<std::string, std::string, int>> cullPatterns = {
				{"", "None", kCullNone},
				{"", "Front", kCullFront},
				{"", "Back", kCullBack}
			};
			CullMode cullMode = GetNodeAttribute<CullMode>(nodeProgram, "CullMode", kCullUnkown, cullPatterns);
			if (cullMode != kCullUnkown) 
				progNode.Cull = cullMode;
		}
	}
	void VisitAttributes(const PropertyTreePath& nodeProgram, ConstVisitorRef vis, ProgramNode& progNode) {
		for (auto& it : boost::make_iterator_range(nodeProgram->equal_range("UseAttribute"))) {
			std::string refName = it.second.data();
			auto find_iter = mAttrByName.find(refName);
			if (find_iter != mAttrByName.end()) {
				progNode.Attrs.Add(find_iter->second);
			}
		}

		int index = 0;
		for (auto& it : boost::make_iterator_range(nodeProgram->equal_range("Attribute"))) {
			auto& node_attribute = it.second;
			AttributeNode attribute;

			int elementCount = node_attribute.count("Element");
			attribute.Layout.resize(elementCount);
			int byteOffset = 0, j = 0;
			for (auto& element : boost::make_iterator_range(node_attribute.equal_range("Element"))) {
				static std::vector<std::tuple<std::string, std::string, int>> formatPatterns = {
					//rgba
					{"", "rgba32f", kFormatR32G32B32A32Float},
					{"", "rgba32u", kFormatR32G32B32A32UInt},
					{"", "rgba32s", kFormatR32G32B32A32SInt},

					{"", "rgba16f", kFormatR16G16B16A16Float},
					{"", "rgba16un", kFormatR16G16B16A16UNorm},
					{"", "rgba16u", kFormatR16G16B16A16UInt},
					{"", "rgba16sn", kFormatR16G16B16A16SNorm},
					{"", "rgba16s", kFormatR16G16B16A16SInt},

					{"", "rgba8un", kFormatR8G8B8A8UNorm},
					{"", "rgba8un_srgb", kFormatR8G8B8A8UNormSRgb},
					{"", "rgba8u", kFormatR8G8B8A8UInt},
					{"", "rgba8sn", kFormatR8G8B8A8SNorm},
					{"", "rgba8s", kFormatR8G8B8A8SInt},

					//rgb
					{"", "rgb32f", kFormatR32G32B32Float},
					{"", "rgb32u", kFormatR32G32B32UInt},
					{"", "rgb32s", kFormatR32G32B32SInt},

					//rg
					{"", "rg32f", kFormatR32G32Float},
					{"", "rg32u", kFormatR32G32UInt},
					{"", "rg32s", kFormatR32G32SInt},

					{"", "rg16f", kFormatR16G16Float},
					{"", "rg16un", kFormatR16G16UNorm},
					{"", "rg16u", kFormatR16G16UInt},
					{"", "rg16sn", kFormatR16G16SNorm},
					{"", "rg16s", kFormatR16G16SInt},

					{"", "rg8un", kFormatR8G8UNorm},
					{"", "rg8u", kFormatR8G8UInt},
					{"", "rg8sn", kFormatR8G8SNorm},
					{"", "rg8s", kFormatR8G8SInt},

					//r
					{"", "r32f", kFormatR32Float},
					{"", "r32u", kFormatR32UInt},
					{"", "r32s", kFormatR32SInt},

					{"", "r16f", kFormatR16Float},
					{"", "r16un", kFormatR16UNorm},
					{"", "r16u", kFormatR16UInt},
					{"", "r16sn", kFormatR16SNorm},
					{"", "r16s", kFormatR16SInt},

					{"", "r8un", kFormatR8UNorm},
					{"", "r8u", kFormatR8UInt},
					{"", "r8sn", kFormatR8SNorm},
					{"", "r8s", kFormatR8SInt},
					
					//a
					{"", "a8un", kFormatA8UNorm},

					//bgra, bgrx
					{"", "bgr565un", kFormatB5G6R5UNorm},
					{"", "bgra5551un", kFormatB5G5R5A1UNorm},
					{"", "bgra8un", kFormatB8G8R8A8UNorm},
					{"", "bgrx8un", kFormatB8G8R8X8UNorm},
					{"", "bgra8un_srgb", kFormatB8G8R8A8UNormSRgb},
					{"", "bgrx8un_srgb", kFormatB8G8R8X8UNormSRgb},

					//depth
					{"", "d32f", kFormatD32Float},
					{"", "d16un", kFormatD16UNorm},
					{"", "d24un_s8u", kFormatD24UNormS8UInt},

					//compressed format
					{"", "bc1un", kFormatBC1UNorm},
					{"", "bc1un_srgb", kFormatBC1UNormSRgb},

					{"", "bc2un", kFormatBC2UNorm},
					{"", "bc2un_srgb", kFormatBC2UNormSRgb},

					{"", "bc3un", kFormatBC3UNorm},
					{"", "bc3un_srgb", kFormatBC3UNormSRgb},

					{"", "bc4un", kFormatBC4UNorm},
					{"", "bc4sn", kFormatBC4SNorm},

					{"", "bc5un", kFormatBC5UNorm},
					{"", "bc5sn", kFormatBC5SNorm},

					{"", "bc7un", kFormatBC7UNorm},
					{"", "bc7un_srgb", kFormatBC7UNormSRgb},
				};
				ResourceFormat format = GetNodeAttribute<ResourceFormat>(element.second, "<xmlattr>.Format", kFormatUnknown, formatPatterns);

				auto& layoutJ = attribute.Layout[j];
				layoutJ = LayoutInputElement{
					element.second.get<std::string>("<xmlattr>.SemanticName"),
					element.second.get<uint32_t>("<xmlattr>.SemanticIndex", 0),
					format,
					element.second.get<uint32_t>("<xmlattr>.InputSlot", 0),
					element.second.get<uint32_t>("<xmlattr>.ByteOffset", byteOffset),
					kLayoutInputPerVertexData,
					0
				};
				byteOffset = layoutJ.AlignedByteOffset + d3d::BytePerPixel(static_cast<DXGI_FORMAT>(layoutJ.Format));
				++j;
			}

			std::string shortName = node_attribute.get<std::string>("<xmlattr>.Name", "");
			shortName = node_attribute.get<std::string>("<xmlattr>.ShortName", shortName);
			if (!shortName.empty()) mAttrByName.insert(std::make_pair(shortName, attribute));
		
		#if ENABLE_USEXXX_FULLPATH
			shortName = PropertyTreePath(nodeProgram, node_attribute, index).Path.string();
			mAttrByName.insert(std::make_pair(shortName, attribute));
		#endif

			progNode.Attrs.Add(std::move(attribute));
			++index;
		}
	}
	void VisitUniforms(const PropertyTreePath& nodeProgram, ConstVisitorRef vis, ProgramNode& progNode) {
		for (auto& it : boost::make_iterator_range(nodeProgram->equal_range("UseUniform"))) {
			std::string refName = it.second.data();
			auto find_iter = mUniformByName.find(refName);
			if (find_iter != mUniformByName.end()) {
				int refSlot = it.second.get<int>("<xmlattr>.Slot", find_iter->second.GetSlot());
				progNode.Uniforms.AddOrSet(find_iter->second, refSlot);
			}
		}

		int index = 0;
		for (auto& it : boost::make_iterator_range(nodeProgram->equal_range("Uniform"))) {
			auto& node_uniform = it.second;
			UniformNode uniform;
			UniformParametersBuilder uniBuilder(uniform);
			for (auto& element : boost::make_iterator_range(node_uniform.equal_range("Element"))) {
				static std::vector<std::tuple<std::string, std::string, int>> typePatterns = {
					{"", "bool", (int)CbDeclElement::Type::Bool},
					{"", "int", (int)CbDeclElement::Type::Int},
					{"", "int2", (int)CbDeclElement::Type::Int2},
					{"", "int3", (int)CbDeclElement::Type::Int3},
					{"", "int4", (int)CbDeclElement::Type::Int4},
					{"", "float", (int)CbDeclElement::Type::Float},
					{"", "float2", (int)CbDeclElement::Type::Float2},
					{"", "float3", (int)CbDeclElement::Type::Float3},
					{"", "float4", (int)CbDeclElement::Type::Float4},
					{"", "matrix", (int)CbDeclElement::Type::Matrix},
				};
				CbDeclElement::Type type = GetNodeAttribute<CbDeclElement::Type>(element.second, "<xmlattr>.Type", CbDeclElement::Type::Max, typePatterns); BOOST_ASSERT(type != CbDeclElement::Type::Max);
				std::string name = element.second.get<std::string>("<xmlattr>.Name"/*, ""*/);
				size_t size = element.second.get<int>("<xmlattr>.Size", 0); BOOST_ASSERT(size % 4 == 0);
				size_t count = element.second.get<int>("<xmlattr>.Count", 0);
				size_t offset = element.second.get<int>("<xmlattr>.Offset", -1);
				std::string strDefault = element.second.get<std::string>("<xmlattr>.Default", "");
				uniBuilder.AddParameter(name, type, size, count, offset, strDefault);

				if (!name.empty() && name[0] == '_') {
					for (size_t slot = 0; slot < progNode.Samplers.Count(); ++slot) {
						const auto& sampler = progNode.Samplers[slot];
						std::string texSize = sampler.GetName() + "_TexelSize";
						if (texSize == name) {
							progNode.Relate2Parameter.TextureSizes[slot] = texSize;
							progNode.Relate2Parameter.HasTextureSize = true;
							break;
						}
					}
				}
			}
			uniBuilder.Slot() = node_uniform.get<int>("<xmlattr>.Slot", -1); BOOST_ASSERT(uniBuilder.Slot() >= 0);
			static std::vector<std::tuple<std::string, std::string, int>> sharePatterns = {
				{"", "PerInstance", kCbSharePerInstance},
				{"", "PerMaterial", kCbSharePerMaterial},
				{"", "PerFrame", kCbSharePerFrame},
			};
			uniBuilder.ShareMode() = GetNodeAttribute<CBufferShareMode>(node_uniform, "<xmlattr>.ShareMode", kCbSharePerInstance, sharePatterns);
			uniBuilder.IsReadOnly() = node_uniform.get<bool>("<xmlattr>.IsReadOnly", false);
			BOOST_ASSERT(!uniBuilder.IsReadOnly() || uniBuilder.ShareMode() != kCbSharePerInstance);

			auto& shortName = uniBuilder.ShortName();
			shortName = node_uniform.get<std::string>("<xmlattr>.Name", "");
			shortName = node_uniform.get<std::string>("<xmlattr>.ShortName", shortName);
			uniBuilder.Build();

			if (!shortName.empty()) mUniformByName.insert(std::make_pair(shortName, uniform));
		
		#if ENABLE_USEXXX_FULLPATH
			shortName = PropertyTreePath(nodeProgram, node_uniform, index).Path.string();
			mUniformByName.insert(std::make_pair(shortName, uniform));
		#endif

			progNode.Uniforms.AddOrSet(std::move(uniform), uniform.GetSlot());
			++index;
		}
	}
	void VisitSamplers(const PropertyTreePath& nodeProgram, ConstVisitorRef vis, ProgramNode& progNode) {
		for (auto& it : boost::make_iterator_range(nodeProgram->equal_range("UseTexture"))) {
			if (!vis.CheckCondition(progNode, it.second))
				continue;
			std::string refName = it.second.data();
			auto find_iter = mSamplerSetByName.find(refName);
			if (find_iter != mSamplerSetByName.end()) {
				progNode.Samplers.Merge<true>(find_iter->second);
			}
		}

		int index = 0;
		for (auto& tit : boost::make_iterator_range(nodeProgram->equal_range("Texture"))) {
			auto& node_sampler = tit.second;
			SamplerNode samplerSet;
			if (! vis.CheckCondition(progNode, node_sampler, nullptr))
				continue;

			bool hasCondition = false;
			for (auto& it : boost::make_iterator_range(node_sampler.equal_range("UseTexture"))) {
				if (!vis.CheckCondition(progNode, it.second, &hasCondition))
					continue;
				std::string refName = it.second.data();
				auto find_iter = mSamplerSetByName.find(refName);
				if (find_iter != mSamplerSetByName.end()) {
					samplerSet.Merge<true>(find_iter->second);
				}
			}

			for (auto& it : boost::make_iterator_range(node_sampler.equal_range("Element"))) {
				auto& node_element = it.second;
				if (!vis.CheckCondition(progNode, node_element, &hasCondition))
					continue;

				static std::vector<std::tuple<std::string, std::string, int>> compFuncPatterns = {
					{"", "Never", kCompareNever},
					{"", "Less", kCompareLess},
					{"", "Equal", kCompareEqual},
					{"", "LessEqual", kCompareLessEqual},
					{"", "Greater", kCompareGreater},
					{"", "NotEqual", kCompareNotEqual},
					{"", "GreaterEqual", kCompareGreaterEqual},
					{"", "Always", kCompareAlways},
				};
				CompareFunc cmpFunc = GetNodeAttribute<CompareFunc>(node_element, "<xmlattr>.CompFunc", kCompareNever, compFuncPatterns);

				static std::vector<std::tuple<std::string, std::string, int>> filterPatterns = {
					{"", "Point", kSFMMBase},
					{"", "MinLinear", kSFMMLinear_Min},
					{"", "MagLinear", kSFMMLinear_Mag},
					{"", "MipLinear", kSFMMLinear_Mip},
					{"", "MinMipLinear", kSFMMLinear_Min | kSFMMLinear_Mip},
					{"", "MagMipLinear", kSFMMLinear_Mag | kSFMMLinear_Mip},
					{"", "MinMagLinear", kSFMMLinear_Min | kSFMMLinear_Mag},
					{"", "MinMagMipLinear", kSFMMLinear_Min | kSFMMLinear_Mag | kSFMMLinear_Mip},
					{"", "Anisotropic", kSFMMAnisotropic}
				};
				SamplerFilterMode filter = (cmpFunc != kCompareNever) ? kSamplerFilterCmpMinMagLinearMipPoint : kSamplerFilterMinMagMipLinear;
				filter = GetNodeAttribute<SamplerFilterMode>(node_element, "<xmlattr>.Filter", filter, filterPatterns);

				static std::vector<std::tuple<std::string, std::string, int>> addrPatterns = {
					{"", "Wrap", kAddressWrap},
					{"", "Mirror", kAddressMirror},
					{"", "Clamp", kAddressClamp},
					{"", "Border", kAddressBorder},
					{"", "MirrorOnce", kAddressMirrorOnce},
				};
				AddressMode address = (cmpFunc != kCompareNever) ? kAddressBorder : kAddressClamp;
				address = GetNodeAttribute<AddressMode>(node_element, "<xmlattr>.Address", address, addrPatterns);
				AddressMode addrU = GetNodeAttribute<AddressMode>(node_element, "<xmlattr>.AddressU", address, addrPatterns);
				AddressMode addrV = GetNodeAttribute<AddressMode>(node_element, "<xmlattr>.AddressV", address, addrPatterns);
				AddressMode addrW = GetNodeAttribute<AddressMode>(node_element, "<xmlattr>.AddressW", address, addrPatterns);

				int slot = node_element.get<int>("<xmlattr>.Slot", -1);
				samplerSet.AddOrSet(SamplerDescEx::Make(filter, cmpFunc, addrU, addrV, addrW, it.second.data()), slot);
			}

			std::string shortName = node_sampler.get<std::string>("<xmlattr>.Name", "");
			shortName = node_sampler.get<std::string>("<xmlattr>.ShortName", shortName);
			if (!hasCondition && !shortName.empty()) 
				mSamplerSetByName.insert(std::make_pair(shortName, samplerSet));

		#if ENABLE_USEXXX_FULLPATH
			shortName = PropertyTreePath(nodeProgram, node_sampler, index).Path.string();
			mSamplerSetByName.insert(std::make_pair(shortName, samplerSet));
		#endif

			progNode.Samplers.Merge<true>(std::move(samplerSet));
			++index;
		}
	}
	void VisitProgram(const PropertyTreePath& nodeProgram, ConstVisitorRef vis, ProgramNode& progNode) {
		ParseProgram(nodeProgram.Node, vis, progNode);
		VisitAttributes(nodeProgram, vis, progNode);
		VisitSamplers(nodeProgram, vis, progNode);
		VisitUniforms(nodeProgram, vis, progNode);
		BOOST_ASSERT(progNode.Validate());
	}

	static std::string MakeLoopVarName(std::string name, int i, int maxI) {
		size_t pos = name.find("%");
		if (pos != std::string::npos) {
			std::string postfix = name.substr(pos + 1);
			int pfIdx = 0;
			if (pfIdx < postfix.size()) {
				if (postfix[pfIdx] == '^') {
					if (i == 0) return "";
					pfIdx++;
				}
				else if (postfix[pfIdx] == '$') {
					if (i == maxI - 1) return "";
					pfIdx++;
				}
			}

			int index = i;
			if (pfIdx < postfix.size()) {
				if (postfix[pfIdx] == '+')
					index += std::stoi(postfix.substr(pfIdx + 1));
				else if (postfix[pfIdx] == '-')
					index -= std::stoi(postfix.substr(pfIdx + 1));
			}
				
			name = name.substr(0, pos) + boost::lexical_cast<std::string>(index);
		}
		return name;
	};
	struct GrabString {
		bool Parse(const std::string& grabstr) {
			Dic.clear();
			Name.clear();

			if (grabstr.empty()) return false;
			SplitString(TmpStrKeyValues, grabstr, "{}");

			Name = TmpStrKeyValues[0];
			if (TmpStrKeyValues.size() > 1) {
				std::string strDic = TmpStrKeyValues[1];
				SplitString(TmpStrKeyValues, strDic, ";");
				for (auto& kv : TmpStrKeyValues) {
					SplitString(TmpStrTokens, kv, ":[,]");
					if (TmpStrTokens.size() >= 2) {
						const std::string& key = TmpStrTokens[0];
						auto& values = Dic[key];
						values.resize(TmpStrTokens.size() - 1);
						for (size_t i = 1; i < TmpStrTokens.size(); ++i)
							values[i - 1] = std::stof(TmpStrTokens[i]);
					}
				}
			}
			return ! Name.empty();
		}
	public:
		template<class T> T Get(const std::string& key, T defValue) {}
		template<> float Get<float>(const std::string& key, float defValue) {
			auto& values = Dic[key];
			return values.empty() ? defValue : values[0];
		}
		template<> int Get<int>(const std::string& key, int defValue) {
			auto& values = Dic[key];
			return values.empty() ? defValue : values[0];
		}
		bool Get(const std::string& key, std::vector<ResourceFormat>& fmts) {
			auto& values = Dic[key];
			fmts.clear();
			for (auto& v : values) {
				int iv = v;
				fmts.push_back(static_cast<ResourceFormat>(iv));
			}
			return ! fmts.empty();
		}
	public:
		std::string Name;
		std::map<std::string, std::vector<float>> Dic;
		std::vector<std::string> TmpStrKeyValues, TmpStrTokens;
	};
	void VisitSubShader(const PropertyTreePath& nodeTechnique, ConstVisitorRef vis, CategoryNode& categNode) {
		TechniqueNode& techniqueNode = categNode.Emplace();
		int index = 0;
		for (auto& pit : boost::make_iterator_range(nodeTechnique->equal_range("Pass"))) {
			auto& node_pass = pit.second;

			std::string strRepeat = node_pass.get<std::string>("<xmlattr>.Repeat", "1");
			int repeat = std::max(vis.GetMacroValue(categNode.Program, strRepeat), atoi(strRepeat.c_str()));
			for (int i = 0; i < repeat; ++i) {
				PassNode pass;
				auto& pprop = *pass.Property;

				pprop.LightMode = LIGHTMODE_FORWARD_BASE_;
				for (auto& it : boost::make_iterator_range(node_pass.equal_range("Tags"))) {
					auto& node_tag = it.second;
					pprop.LightMode = node_tag.get<std::string>("LightMode", pprop.LightMode);
				}
				int lightModeValue = GetLightModeValueByName(pprop.LightMode);
				pass.Program.VertexSCD.AddMacro<true>(ShaderCompileMacro{ "LIGHTMODE", boost::lexical_cast<std::string>(lightModeValue) });
				pass.Program.PixelSCD.AddMacro<true>(ShaderCompileMacro{ "LIGHTMODE", boost::lexical_cast<std::string>(lightModeValue) });

				pprop.ShortName = node_pass.get<std::string>("ShortName", node_pass.get<std::string>("Name", ""));
				pprop.Name = node_pass.get<std::string>("Name", boost::lexical_cast<std::string>(index));

				pass.Program = categNode.Program;
				for (auto& it : boost::make_iterator_range(node_pass.equal_range("PROGRAM"))) {
					ParseProgram(it.second, vis, pass.Program);
				}
				auto& passProg = pass.Program;
				pprop.TopoLogy = passProg.Topo;
				pprop.Blend = passProg.Blend;
				pprop.Depth = passProg.Depth;
				pprop.Fill = passProg.Fill;
				pprop.Cull = passProg.Cull;
				pprop.DepthBias = passProg.DepthBias;
				pprop.Relate2Parameter = passProg.Relate2Parameter;

				GrabString grabStr;
				if (grabStr.Parse(MakeLoopVarName(node_pass.get<std::string>("GrabPass", ""), i, repeat))) {
					pprop.GrabOut.Name = grabStr.Name;
					pprop.GrabOut.Size = grabStr.Get<float>("size", 1.0f);
					grabStr.Get("fmts", pprop.GrabOut.Formats);
				}
				for (auto& it : boost::make_iterator_range(node_pass.equal_range("UseGrab"))) {
					if (grabStr.Parse(it.second.data())) {
						pprop.GrabIn.emplace_back();
						auto& unit = pprop.GrabIn.back();
						unit.Name = grabStr.Name;
						unit.TextureSlot = grabStr.Get<int>("bind_slot", 0);
						unit.AttachIndex = grabStr.Get<int>("attach_index", 0);
					}
				}

				techniqueNode.Add(std::move(pass));
			}
			++index;
		}
	}
	void VisitCategory(const PropertyTreePath& nodeCategory, ConstVisitorRef vis, CategoryNode& categNode) {
		int index = 0;
		for (auto& it : boost::make_iterator_range(nodeCategory->equal_range("PROGRAM"))) {
			VisitProgram(PropertyTreePath(nodeCategory, it.second, index++), vis, categNode.Program);
		}

		if (!vis.JustInclude) {
			index = 0;
			for (auto& it : boost::make_iterator_range(nodeCategory->equal_range("SubShader"))) {
				VisitSubShader(PropertyTreePath(nodeCategory, it.second, index++), vis, categNode);
			}
		}
	}
	void VisitShader(const PropertyTreePath& nodeShader, ConstVisitorRef vis, ShaderNode& shaderNode) {
		shaderNode.ShortName = nodeShader->get<std::string>("<xmlattr>.Name"/*, ""*/);

		for (auto& it : boost::make_iterator_range(nodeShader->equal_range("UseShader"))) {
			ParseShaderFile(it.second.data(), shaderNode);
		}

		for (auto& it : boost::make_iterator_range(nodeShader->equal_range("Include"))) {
			auto& incShader = VisitInclude(it.second.data());
			vis.PredMacros.Merge(incShader.PredMacros);
			shaderNode.DependShaders.Merge(incShader.DependShaders);
		}
		VisitCategory(nodeShader, vis, shaderNode[0]);
		mIncludeFiles.GetFileDependecies(shaderNode[0].Program.VertexSCD.SourcePath, shaderNode.DependShaders);

		int index = 0;
		for (auto& it : boost::make_iterator_range(nodeShader->equal_range("Category"))) {
			auto& categNode = shaderNode.Emplace();
			VisitCategory(PropertyTreePath(nodeShader, it.second, index++), vis, categNode);
			mIncludeFiles.GetFileDependecies(categNode.Program.VertexSCD.SourcePath, shaderNode.DependShaders);
		}
	}
	
	static bool GetShaderAssetPath(const MaterialLoadParam& loadParam, std::string& filepath, time_t& fileTime) {
		boost_filesystem::path path(loadParam.ShaderVariantName);
		if (path.has_extension()) path = boost_filesystem::system_complete(path); 
		else path = boost_filesystem::system_complete("shader/" + loadParam.ShaderVariantName + ".Shader");
		
		bool exits = boost_filesystem::exists(path);
		filepath = path.string();
		fileTime = IF_AND_OR(exits, boost::filesystem::last_write_time(path), 0);
		return exits;
	}
	bool ParseShaderFile(const MaterialLoadParam& loadParam, ShaderNode& shaderNode) {
		bool result;
		auto find_iter = mShaderByParam.find(loadParam);
		if (find_iter == mShaderByParam.end()) {
			MaterialProperty::SingleFileDependency fdep;
			if (GetShaderAssetPath(loadParam, fdep.FilePath, fdep.FileTime)) {
				shaderNode.DependShaders.AddShader(fdep);

				boost_property_tree::ptree pt;
				boost_property_tree::read_xml(fdep.FilePath, pt);
				VisitShader(pt.get_child("Shader"), Visitor{ false, loadParam, shaderNode.PredMacros }, shaderNode);
				BOOST_ASSERT(shaderNode.Validate());
				
				mShaderByParam.insert(std::make_pair(loadParam, shaderNode));
				result = true;
			}
			else {
				result = false;
			}
		}
		else {
			shaderNode = find_iter->second;
			result = true;
		}
		return result;
	}
	bool GetShaderVariantNode(const MaterialLoadParam& loadParam, ShaderNode& shaderNode) ThreadSafe {
		bool result = true;
		mShaderVariantByParam.GetOrAdd(loadParam, [&]() {
			if (result = ParseShaderFile(loadParam, shaderNode)) {
				for (auto& categNode : shaderNode) {
					for (auto& techniqueNode : categNode) {
						for (auto& passNode : techniqueNode) {
							passNode.Program.VertexSCD.AddMacros<true>(loadParam.Macros);
							passNode.Program.PixelSCD.AddMacros<true>(loadParam.Macros);
						}
					}
				}
			}
			return shaderNode;
		}, shaderNode);
		return result;
	}
private:
	tpl::AtomicMap<MaterialLoadParam, ShaderNode> mShaderVariantByParam;
	std::map<MaterialLoadParam, ShaderNode> mShaderByParam;
	std::map<std::string, ShaderNode> mIncludeByName;
	std::map<std::string, AttributeNode> mAttrByName;
	std::map<std::string, UniformNode> mUniformByName;
	std::map<std::string, SamplerNode> mSamplerSetByName;
	IncludeFiles mIncludeFiles;
};

class MaterialNodeManager {
	typedef ShaderNodeManager::Visitor Visitor;
	typedef ShaderNodeManager::PropertyTreePath PropertyTreePath;
public:
	MaterialNodeManager(std::shared_ptr<ShaderNodeManager> shaderMng) :mShaderMng(shaderMng) {}
	bool GetMaterialNode(const MaterialLoadParam& loadParam, MaterialNode& materialNode) {
		return ParseMaterialFile(loadParam, materialNode);
	}
	bool PurgeOutOfDates() ThreadSafe {
		if (! mShaderMng->PurgeOutOfDates()) {
			tpl::AutoLock lck(mMaterialByPath._GetLock());
			for (auto& it : mMaterialByPath._GetDic()) {
				if (it.second.Property->DependSrc.CheckOutOfDate()) {
					mMaterialByPath._Clear();
					return true;
				}
			}
		}
		return false;
	}
private:
	void VisitProperties(const PropertyTreePath& nodeProperties, MaterialNode& materialNode) {
		auto& mprop = *materialNode.Property;
		for (auto& nodeProp : nodeProperties.Node) {
			materialNode.Shader.ForEachProgram([&mprop,&nodeProp, &materialNode](const ProgramNode& prog) {
				size_t index = prog.Samplers.IndexByName(nodeProp.first);
				if (index != prog.Samplers.IndexNotFound()) {
					auto& texProp = mprop.Textures[nodeProp.first];
					texProp.ImagePath = nodeProp.second.data();
					texProp.Slot = index;
				}
				else {
					mprop.UniformByName.insert(std::make_pair(nodeProp.first, nodeProp.second.data()));
				}
			});
		}
	}
	bool VisitMaterial(const MaterialLoadParam& loadParam, const PropertyTreePath& nodeMaterial, MaterialNode& materialNode) {
		auto find_useShader = nodeMaterial->find("UseShader");
		if (find_useShader == nodeMaterial->not_found()) return false;
		
		MaterialLoadParamBuilder paramBuilder = MaterialLoadParamBuilder(loadParam);
		for (auto& mit : boost::make_iterator_range(nodeMaterial->equal_range("Macros"))) {
			auto& node_macros = mit.second;
			for (auto& it : node_macros)
				paramBuilder[it.first] = std::stoi(it.second.data());
		}
		materialNode.LoadParam = paramBuilder;
		materialNode.LoadParam.ShaderVariantName = find_useShader->second.data();
		if (!mShaderMng->GetShaderNode(materialNode.LoadParam, materialNode.Shader)) 
			return false;
		materialNode.Property->DependSrc.Shaders = materialNode.Shader.DependShaders.Shaders;
		materialNode.LoadParam = loadParam;

		for (auto& it : boost::make_iterator_range(nodeMaterial->equal_range("Properties"))) {
			VisitProperties(it.second, materialNode);
		}
		return true;
	}

	void BuildMaterialNode(MaterialNode& materialNode) {}
	static bool GetMaterialAssetPath(const MaterialLoadParam& loadParam, std::string& filepath, time_t& fileTime) {
		boost_filesystem::path path(loadParam.ShaderVariantName);
		if (path.has_extension()) path = boost_filesystem::system_complete(path);
		else path = boost_filesystem::system_complete("shader/" + loadParam.ShaderVariantName + ".Material");
		
		bool exits = boost_filesystem::exists(path);
		filepath = path.string();
		fileTime = IF_AND_OR(exits, boost::filesystem::last_write_time(path), 0);
		return exits;
	}
	bool ParseMaterialFile(const MaterialLoadParam& loadParam, MaterialNode& materialNode) {
		bool result = true;
		mMaterialByPath.GetOrAdd(loadParam, [&]() {
			boost_property_tree::ptree pt;
			auto& fdep = materialNode.Property->DependSrc.Material;
			if (GetMaterialAssetPath(loadParam, fdep.FilePath, fdep.FileTime)) {
				boost_property_tree::read_xml(fdep.FilePath, pt);
			}
			else {
				std::stringstream ss;
				ss << "<Material>";
				std::string shaderName = boost_filesystem::path(loadParam.ShaderVariantName).stem().string();
				ss << "<UseShader>" << shaderName << "</UseShader>";
				ss << "</Material>";
				boost_property_tree::read_xml(ss, pt);
			}

			if (result = VisitMaterial(loadParam, pt.get_child("Material"), materialNode)) {
				BuildMaterialNode(materialNode);
			}
			return materialNode;
		}, materialNode);
		return result;
	}
private:
	std::shared_ptr<ShaderNodeManager> mShaderMng;
	tpl::AtomicMap<MaterialLoadParam, MaterialNode> mMaterialByPath;
};

/********** MaterialAssetManager **********/
MaterialAssetManager::MaterialAssetManager()
{
	mShaderNodeMng = CreateInstance<ShaderNodeManager>();
	mMaterialNodeMng = CreateInstance<MaterialNodeManager>(mShaderNodeMng);
}
bool MaterialAssetManager::GetShaderNode(const MaterialLoadParam& loadParam, ShaderNode& shaderNode) ThreadSafe
{
	return mShaderNodeMng->GetShaderNode(loadParam, shaderNode);
}
bool MaterialAssetManager::GetMaterialNode(const MaterialLoadParam& loadParam, MaterialNode& materialNode) ThreadSafe
{
	return mMaterialNodeMng->GetMaterialNode(loadParam, materialNode);
}

bool MaterialAssetManager::PurgeOutOfDates() ThreadSafe
{
	return mMaterialNodeMng->PurgeOutOfDates();
}

}
}
}