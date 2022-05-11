#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <regex>
#include "core/base/stl.h"
#include "core/base/tpl/atomic_map.h"
#include "core/base/base_type.h"
#include "core/base/macros.h"
#include "core/base/tpl/vector.h"
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
		bool CheckCondition(const ProgramNode& progNode, const boost_property_tree::ptree& node) const {
			bool result = true;
			std::string condition = node.get<std::string>("<xmlattr>.Condition", "");
			if (!condition.empty()) {
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
					return boost::lexical_cast<T>(inode.data());
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
	static void ParseProgramMacrosTopo(const boost_property_tree::ptree& nodeProgram, ConstVisitorRef vis, ProgramNode& progNode) {//no_override
		if (!vis.CheckCondition(progNode, nodeProgram))
			return;

		for (auto& it : boost::make_iterator_range(nodeProgram.equal_range("PredMacros"))) {
			std::string refName = it.second.data();
			if (!refName.empty()) {
				ParseMacrosFile(refName, vis);
			}
		}

		progNode.Topo = static_cast<PrimitiveTopology>(vis.ConditionGetValue<int>(progNode, nodeProgram, "Topology", progNode.Topo));

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
				auto& layoutJ = attribute.Layout[j];
				layoutJ = LayoutInputElement{
					element.second.get<std::string>("<xmlattr>.SemanticName"),
					element.second.get<uint32_t>("<xmlattr>.SemanticIndex", 0),
					static_cast<ResourceFormat>(element.second.get<uint32_t>("<xmlattr>.Format")),
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
	static CbElementType GetCBElementTypeByString(const std::string& str) {
		CbElementType result = kCBElementMax;
		if (str == "bool") result = kCBElementBool;
		else if (str == "int") result = kCBElementInt;
		else if (str == "int2") result = kCBElementInt2;
		else if (str == "int3") result = kCBElementInt3;
		else if (str == "int4") result = kCBElementInt4;
		else if (str == "float") result = kCBElementFloat;
		else if (str == "float2") result = kCBElementFloat2;
		else if (str == "float3") result = kCBElementFloat3;
		else if (str == "float4") result = kCBElementFloat4;
		else if (str == "matrix") result = kCBElementMatrix;
		else BOOST_ASSERT(false);
		return result;
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
				std::string name = element.second.get<std::string>("<xmlattr>.Name"/*, ""*/);
				CbElementType type = GetCBElementTypeByString(element.second.get<std::string>("<xmlattr>.Type"));
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
			uniBuilder.Slot() = node_uniform.get<int>("<xmlattr>.Slot", -1);
			BOOST_ASSERT(uniBuilder.Slot() >= 0);
			uniBuilder.ShareMode() = static_cast<CBufferShareMode>(node_uniform.get<int>("<xmlattr>.ShareMode", kCbShareNone)) ;
			uniBuilder.IsReadOnly() = node_uniform.get<bool>("<xmlattr>.IsReadOnly", false);
			BOOST_ASSERT(!uniBuilder.IsReadOnly() || uniBuilder.ShareMode() != kCbShareNone);

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
			if (! vis.CheckCondition(progNode, node_sampler))
				continue;

			for (auto& it : boost::make_iterator_range(node_sampler.equal_range("UseTexture"))) {
				std::string refName = it.second.data();
				auto find_iter = mSamplerSetByName.find(refName);
				if (find_iter != mSamplerSetByName.end()) {
					samplerSet.Merge<true>(find_iter->second);
				}
			}

			for (auto& it : boost::make_iterator_range(node_sampler.equal_range("Element"))) {
				auto& node_element = it.second;
				if (!vis.CheckCondition(progNode, node_element))
					continue;

				CompareFunc cmpFunc = static_cast<CompareFunc>(node_element.get<int>("<xmlattr>.CompFunc", kCompareNever));

				SamplerFilterMode filter = (cmpFunc != kCompareNever) ? kSamplerFilterCmpMinMagLinearMipPoint : kSamplerFilterMinMagMipLinear;
				filter = static_cast<SamplerFilterMode>(node_element.get<int>("<xmlattr>.Filter", filter));

				int address = (cmpFunc != kCompareNever) ? kAddressBorder : kAddressClamp;
				address = node_element.get<int>("<xmlattr>.Address", address);

				int slot = node_element.get<int>("<xmlattr>.Slot", -1);
				samplerSet.AddOrSet(SamplerDescEx::Make(
					filter,
					cmpFunc,
					static_cast<AddressMode>(node_element.get<int>("<xmlattr>.AddressU", address)),
					static_cast<AddressMode>(node_element.get<int>("<xmlattr>.AddressV", address)),
					static_cast<AddressMode>(node_element.get<int>("<xmlattr>.AddressW", address)),
					it.second.data()
				), slot);
			}

			std::string shortName = node_sampler.get<std::string>("<xmlattr>.Name", "");
			shortName = node_sampler.get<std::string>("<xmlattr>.ShortName", shortName);
			if (!shortName.empty()) mSamplerSetByName.insert(std::make_pair(shortName, samplerSet));

		#if ENABLE_USEXXX_FULLPATH
			shortName = PropertyTreePath(nodeProgram, node_sampler, index).Path.string();
			mSamplerSetByName.insert(std::make_pair(shortName, samplerSet));
		#endif

			progNode.Samplers.Merge<true>(std::move(samplerSet));
			++index;
		}
	}
	void VisitProgram(const PropertyTreePath& nodeProgram, ConstVisitorRef vis, ProgramNode& progNode) {
		ParseProgramMacrosTopo(nodeProgram.Node, vis, progNode);
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
		static void SplitString(std::vector<std::string>& strArr, const std::string& str, const std::string& strAny) {
			strArr.clear();
			boost::split(strArr, str, boost::is_any_of(strAny), boost::token_compress_on);
			if (strArr.back().empty()) strArr.pop_back();
		}
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
			int repeat = __max(vis.GetMacroValue(categNode.Program, strRepeat), atoi(strRepeat.c_str()));
			for (int i = 0; i < repeat; ++i) {
				PassNode pass;
				auto& pprop = *pass.Property;

				pprop.LightMode = LIGHTMODE_FORWARD_BASE;
				for (auto& it : boost::make_iterator_range(node_pass.equal_range("Tags"))) {
					auto& node_tag = it.second;
					pprop.LightMode = node_tag.get<std::string>("LightMode", pprop.LightMode);
				}

				pprop.ShortName = node_pass.get<std::string>("ShortName", node_pass.get<std::string>("Name", ""));
				pprop.Name = node_pass.get<std::string>("Name", boost::lexical_cast<std::string>(index));

				pass.Program = categNode.Program;
				for (auto& it : boost::make_iterator_range(node_pass.equal_range("PROGRAM"))) {
					ParseProgramMacrosTopo(it.second, vis, pass.Program);
				}
				pprop.TopoLogy = pass.Program.Topo;
				pprop.Relate2Parameter = pass.Program.Relate2Parameter;

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
		MaterialLoadParamBuilder paramBuilder = MaterialLoadParamBuilder(loadParam);
		for (auto& mit : boost::make_iterator_range(nodeMaterial->equal_range("Macros"))) {
			auto& node_macros = mit.second;
			for (auto& it : node_macros)
				paramBuilder[it.first] = boost::lexical_cast<int>(it.second.data());
		}

		auto find_useShader = nodeMaterial->find("UseShader");
		if (find_useShader == nodeMaterial->not_found()) return false;

		materialNode.LoadParam = paramBuilder.Build();
		materialNode.LoadParam.ShaderVariantName = find_useShader->second.data();
		if (!mShaderMng->GetShaderNode(materialNode.LoadParam, materialNode.Shader)) return false;
		materialNode.Property->DependSrc.Shaders = materialNode.Shader.DependShaders.Shaders;

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