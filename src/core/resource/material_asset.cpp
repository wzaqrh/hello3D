#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
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
private:
	struct Visitor {
		int GetMacroValue(const std::string& key) const {
			return 0;
		}
	public:
		const bool JustInclude;
		const MaterialLoadParam& LoadParam;
		ShaderNode& shaderNode;
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
	void VisitInclude(const std::string& shaderName) {
		auto find_iter = mIncludeByName.find(shaderName);
		if (find_iter == mIncludeByName.end()) {
			ShaderNode shaderNode;
			ParseShaderFile(shaderName, shaderNode);
			mIncludeByName.insert(std::make_pair(shaderName, std::move(shaderNode)));
		}
	}

	void VisitAttributes(const PropertyTreePath& nodeProgram, ProgramNode& programNode) {
		for (auto& it : boost::make_iterator_range(nodeProgram->equal_range("UseAttribute"))) {
			std::string refName = it.second.data();
			auto find_iter = mAttrByName.find(refName);
			if (find_iter != mAttrByName.end()) {
				programNode.Attrs.Add(find_iter->second);
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

			programNode.Attrs.Add(std::move(attribute));
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
	void VisitUniforms(const PropertyTreePath& nodeProgram, ProgramNode& programNode) {
		for (auto& it : boost::make_iterator_range(nodeProgram->equal_range("UseUniform"))) {
			std::string refName = it.second.data();
			auto find_iter = mUniformByName.find(refName);
			if (find_iter != mUniformByName.end()) {
				int refSlot = it.second.get<int>("<xmlattr>.Slot", find_iter->second.GetSlot());
				programNode.Uniforms.AddOrSet(find_iter->second, refSlot);
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

			programNode.Uniforms.AddOrSet(std::move(uniform), uniform.GetSlot());
			++index;
		}
	}
	static bool CheckCondition(boost_property_tree::ptree& node) {
		std::string condition = node.get<std::string>("<xmlattr>.Condition", "");
		if (!condition.empty()) {
			std::vector<std::string> strArr;
			boost::split(strArr, condition, boost::is_any_of("<>="), boost::token_compress_on);
			if (strArr.size() >= 3) {
				const std::string& macro_name = strArr[0];
				std::string compare = strArr[1];
				for (size_t i = 2; i < strArr.size() - 1; ++i)
					compare += strArr[i];
				const std::string& macro_value = strArr.back();

			}
		}
		return true;
	}
	void VisitSamplers(const PropertyTreePath& nodeProgram, ProgramNode& programNode) {
		for (auto& it : boost::make_iterator_range(nodeProgram->equal_range("UseTexture"))) {
			std::string refName = it.second.data();
			auto find_iter = mSamplerSetByName.find(refName);
			if (find_iter != mSamplerSetByName.end()) {
				programNode.Samplers.Merge<true>(find_iter->second);
			}
		}

		int index = 0;
		for (auto& tit : boost::make_iterator_range(nodeProgram->equal_range("Texture"))) {
			auto& node_sampler = tit.second;
			SamplerNode samplerSet;

			for (auto& it : boost::make_iterator_range(node_sampler.equal_range("UseTexture"))) {
				std::string refName = it.second.data();
				auto find_iter = mSamplerSetByName.find(refName);
				if (find_iter != mSamplerSetByName.end()) {
					samplerSet.Merge<true>(find_iter->second);
				}
			}

			for (auto& it : boost::make_iterator_range(node_sampler.equal_range("Element"))) {
				auto& node_element = it.second;
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

			programNode.Samplers.Merge<true>(std::move(samplerSet));
			++index;
		}
	}
	static void ParseProgramMacrosTopo(const boost_property_tree::ptree& nodeProgram, ProgramNode& programNode) {//no_override
		programNode.Topo = static_cast<PrimitiveTopology>(nodeProgram.get<int>("Topology", programNode.Topo));

		auto& vertexScd = programNode.VertexSCD;
		vertexScd.SourcePath = nodeProgram.get<std::string>("FileName", vertexScd.SourcePath);
		vertexScd.EntryPoint = nodeProgram.get<std::string>("VertexEntry", vertexScd.EntryPoint);

		auto& pixelScd = programNode.PixelSCD;
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
	void VisitProgram(const PropertyTreePath& nodeProgram, ProgramNode& programNode) {
		ParseProgramMacrosTopo(nodeProgram.Node, programNode);
		VisitAttributes(nodeProgram, programNode);
		VisitUniforms(nodeProgram, programNode);
		VisitSamplers(nodeProgram, programNode);
		BOOST_ASSERT(programNode.Validate());
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
	void VisitSubShader(const PropertyTreePath& nodeTechnique, ConstVisitorRef vis, CategoryNode& categNode) {
		TechniqueNode& techniqueNode = categNode.Emplace();
		int index = 0;
		for (auto& it : boost::make_iterator_range(nodeTechnique->equal_range("Pass"))) {
			auto& node_pass = it.second;

			std::string strRepeat = node_pass.get<std::string>("<xmlattr>.Repeat", "1");
			int repeat = __max(categNode.GetMacrosDefinition(vis.LoadParam, strRepeat), atoi(strRepeat.c_str()));
			for (int i = 0; i < repeat; ++i) {
				PassNode pass;

				pass.LightMode = LIGHTMODE_FORWARD_BASE;
				auto find_tags = node_pass.find("Tags");
				if (find_tags != node_pass.not_found()) {
					auto& node_tag = find_tags->second;
					pass.LightMode = node_tag.get<std::string>("LightMode", pass.LightMode);
				}

				pass.ShortName = node_pass.get<std::string>("ShortName", node_pass.get<std::string>("Name", ""));
				pass.Name = node_pass.get<std::string>("Name", boost::lexical_cast<std::string>(index));

				pass.Program = categNode.Program;
				auto find_program = node_pass.find("PROGRAM");
				if (find_program != node_pass.not_found()) {
					auto& node_program = find_program->second;
					ParseProgramMacrosTopo(node_program, pass.Program);
				}

				pass.GrabOut.Name = MakeLoopVarName(node_pass.get<std::string>("GrabPass", ""), i, repeat);
				if (!pass.GrabOut.Name.empty()) {
					std::vector<std::string> strArr;
					boost::split(strArr, pass.GrabOut.Name, boost::is_any_of("{,}"), boost::token_compress_on); 
					if (strArr.back().empty()) strArr.pop_back();

					pass.GrabOut.Name = strArr[0];
					pass.GrabOut.Format.resize(strArr.size() - 1);
					for (size_t i = 1; i < strArr.size(); ++i)
						pass.GrabOut.Format[i-1] = (ResourceFormat)std::stoi(strArr[i]);
				}
				
				pass.GrabIn.TextureSlot = 0;
				pass.GrabIn.Name = node_pass.get<std::string>("UseGrab", "");
				if (!pass.GrabIn.Name.empty()) {
					std::vector<std::string> strArr;
					boost::split(strArr, pass.GrabIn.Name, boost::is_any_of("{=}"), boost::token_compress_on);
					if (strArr.back().empty()) strArr.pop_back();

					pass.GrabIn.Name = strArr[0];
					if (strArr.size() > 1) pass.GrabIn.TextureSlot = std::stoi(strArr[1]);
					if (strArr.size() > 2) pass.GrabIn.AttachIndex = std::stoi(strArr[2]);
				}

				techniqueNode.Add(std::move(pass));
			}
			++index;
		}
	}

	void VisitCategory(const PropertyTreePath& nodeCategory, ConstVisitorRef vis, CategoryNode& categNode) {
		int index = 0;
		for (auto& it : boost::make_iterator_range(nodeCategory->equal_range("PROGRAM"))) {
			VisitProgram(PropertyTreePath(nodeCategory, it.second, index++), categNode.Program);
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
			VisitInclude(it.second.data());
		}
		VisitCategory(nodeShader, vis, shaderNode[0]);

		int index = 0;
		for (auto& it : boost::make_iterator_range(nodeShader->equal_range("Category"))) {
			VisitCategory(PropertyTreePath(nodeShader, it.second, index++), vis, shaderNode.Emplace());
		}
	}
	
	void BuildShaderNode(ShaderNode& shaderNode) {
		for (auto& categNode : shaderNode) {
			categNode.Program.Build();
			for (auto& techNode : categNode) {
				for (auto& passNode : techNode) {
					//passNode.Program.Merge<false>(categNode.Program);
					//passNode.Program.Build();
				}
			}
		}
	}
	static bool GetShaderAssetPath(const MaterialLoadParam& loadParam, std::string& filepath) {
		boost_filesystem::path path(loadParam.ShaderVariantName);
		if (path.has_extension()) path = boost_filesystem::system_complete(path); 
		else path = boost_filesystem::system_complete("shader/" + loadParam.ShaderVariantName + ".Shader");
		filepath = path.string();
		return boost_filesystem::exists(path);
	}
	bool ParseShaderFile(const MaterialLoadParam& loadParam, ShaderNode& shaderNode) {
		bool result;
		auto find_iter = mShaderByParam.find(loadParam);
		if (find_iter == mShaderByParam.end()) {
			std::string filepath;
			if (GetShaderAssetPath(loadParam, filepath)) {
				boost_property_tree::ptree pt;
				boost_property_tree::read_xml(filepath, pt);
				VisitShader(pt.get_child("Shader"), Visitor{ false, loadParam, shaderNode }, shaderNode);
				BuildShaderNode(shaderNode);
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
				BuildShaderNode(shaderNode);
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
};

class MaterialNodeManager {
	typedef ShaderNodeManager::Visitor Visitor;
	typedef ShaderNodeManager::PropertyTreePath PropertyTreePath;
public:
	MaterialNodeManager(std::shared_ptr<ShaderNodeManager> shaderMng) :mShaderMng(shaderMng) {}
	bool GetMaterialNode(const MaterialLoadParam& loadParam, MaterialNode& materialNode) {
		return ParseMaterialFile(loadParam, materialNode);
	}
private:
	void VisitProperties(const PropertyTreePath& nodeProperties, MaterialNode& materialNode) {
		for (auto& nodeProp : nodeProperties.Node) {
			materialNode.Shader.ForEachProgram([&nodeProp, &materialNode](const ProgramNode& prog) {
				size_t index = prog.Samplers.IndexByName(nodeProp.first);
				if (index != prog.Samplers.IndexNotFound()) {
					auto& texProp = materialNode.TextureProperies[nodeProp.first];
					texProp.ImagePath = nodeProp.second.data();
					texProp.Slot = index;
				}
				else {
					materialNode.UniformProperies.insert(std::make_pair(nodeProp.first, nodeProp.second.data()));
				}
			});
		}
	}
	bool VisitMaterial(const MaterialLoadParam& loadParam, const PropertyTreePath& nodeMaterial, MaterialNode& materialNode) {
		MaterialLoadParamBuilder paramBuilder = MaterialLoadParamBuilder(loadParam);
		auto find_macros = nodeMaterial->find("Macros");
		if (find_macros != nodeMaterial->not_found()) {
			auto& node_macros = find_macros->second;
			for (auto& it : node_macros)
				paramBuilder[it.first] = boost::lexical_cast<int>(it.second.data());
		}

		auto find_useShader = nodeMaterial->find("UseShader");
		if (find_useShader == nodeMaterial->not_found()) return false;

		materialNode.LoadParam = paramBuilder.Build();
		materialNode.LoadParam.ShaderVariantName = find_useShader->second.data();
		if (!mShaderMng->GetShaderNode(materialNode.LoadParam, materialNode.Shader)) return false;
		
		for (auto& it : boost::make_iterator_range(nodeMaterial->equal_range("Properties"))) {
			VisitProperties(it.second, materialNode);
		}
		return true;
	}

	void BuildMaterialNode(MaterialNode& materialNode) {
		mShaderMng->BuildShaderNode(materialNode.Shader);
	}
	static bool GetMaterialAssetPath(const MaterialLoadParam& loadParam, std::string& filepath) {
		boost_filesystem::path path(loadParam.ShaderVariantName);
		if (path.has_extension()) path = boost_filesystem::system_complete(path);
		else path = boost_filesystem::system_complete("shader/" + loadParam.ShaderVariantName + ".Material");
		filepath = path.string();
		return boost_filesystem::exists(path);
	}
	bool ParseMaterialFile(const MaterialLoadParam& loadParam, MaterialNode& materialNode) {
		bool result;
		auto find_iter = mMaterialByPath.find(loadParam);
		if (find_iter == mMaterialByPath.end()) {
			boost_property_tree::ptree pt;
			std::string filepath;
			if (GetMaterialAssetPath(loadParam, filepath)) {
				boost_property_tree::read_xml(filepath, pt);
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
				materialNode.MaterialFilePath = filepath;
				mMaterialByPath.insert(std::make_pair(loadParam, materialNode));
			}
		}
		else {
			materialNode = find_iter->second;
			result = true;
		}
		return result;
	}
private:
	std::shared_ptr<ShaderNodeManager> mShaderMng;
	std::map<MaterialLoadParam, MaterialNode> mMaterialByPath;
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

}
}
}