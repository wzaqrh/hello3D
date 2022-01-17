#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "core/base/stl.h"
#include "core/base/base_type.h"
#include "core/base/macros.h"
#include "core/base/template/container_adapter.h"
#include "core/base/d3d.h"
#include "core/resource/material_name.h"
#include "core/resource/material_asset.h"

namespace boost_filesystem = boost::filesystem;
namespace boost_property_tree = boost::property_tree;

namespace mir {
namespace res {
namespace mat_asset {

/********** ConstBufferDeclBuilder **********/
struct ConstBufferDeclBuilder
{
public:
	ConstBufferDeclBuilder(ConstBufferDecl& decl) :mDecl(decl) {}
	ConstBufferDeclBuilder& Add(const ConstBufferDeclElement& elem) {
		mDecl.Add(elem).Offset = mDecl.BufferSize;
		mDecl.BufferSize += elem.Size;
		return *this;
	}
	ConstBufferDeclBuilder& Add(const ConstBufferDeclElement& elem, const ConstBufferDecl& subDecl) {
		mDecl.Add(elem, subDecl).Offset = mDecl.BufferSize;
		mDecl.BufferSize += elem.Size;
		return *this;
	}
	ConstBufferDecl& Build() {
		return mDecl;
	}
private:
	ConstBufferDecl& mDecl;
};

/********** ShaderNodeManager **********/
class ShaderNodeManager
{
	std::map<ShaderLoadParam, ShaderNode> mShaderByParam, mShaderVariantByParam;
	std::map<std::string, ShaderNode> mIncludeByName;
	std::map<std::string, AttributeNode> mAttrByName;
	std::map<std::string, UniformNode> mUniformByName;
	std::map<std::string, SamplerNode> mSamplerSetByName;
public:
	ShaderNodeManager() {}
	bool GetShaderNode(const ShaderLoadParam& loadParam, ShaderNode& shaderNode) {
		bool result = GetShaderVariantNode(loadParam, shaderNode);
		BOOST_ASSERT(shaderNode[0].Program.Topo != kPrimTopologyUnkown);
		return result;
	}
private:
	struct Visitor {
		const bool JustInclude;
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

	static ConstBufferElementType ConvertStringToConstBufferElementType(
		const std::string& str, int count, int& size) {
		ConstBufferElementType result = kCBElementMax;
		if (str == "bool") result = kCBElementBool, size = sizeof(BOOL);
		else if (str == "int") result = kCBElementInt, size = sizeof(int);
		else if (str == "int2") result = kCBElementInt2, size = sizeof(int) * 2;
		else if (str == "int3") result = kCBElementInt3, size = sizeof(int) * 3;
		else if (str == "int4") result = kCBElementInt4, size = sizeof(int) * 4;
		else if (str == "float") result = kCBElementFloat, size = sizeof(float);
		else if (str == "float2") result = kCBElementFloat2, size = sizeof(float) * 2;
		else if (str == "float3") result = kCBElementFloat3, size = sizeof(float) * 3;
		else if (str == "float4") result = kCBElementFloat4, size = sizeof(float) * 4;
		else if (str == "matrix") result = kCBElementMatrix, size = sizeof(float) * 16;
		else if (str == "struct") result = kCBElementStruct;
		size *= std::max<int>(1, count);
		return result;
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

			std::string Name = node_attribute.get<std::string>("<xmlattr>.Name", boost::lexical_cast<std::string>(index));
			mAttrByName.insert(std::make_pair(Name, attribute));

			Name = PropertyTreePath(nodeProgram, node_attribute, index).Path.string();
			mAttrByName.insert(std::make_pair(Name, attribute));

			programNode.Attrs.Add(std::move(attribute));
			++index;
		}
	}
	void VisitUniforms(const PropertyTreePath& nodeProgram, ProgramNode& programNode) {
		for (auto& it : boost::make_iterator_range(nodeProgram->equal_range("UseUniform"))) {
			std::string refName = it.second.data();
			auto find_iter = mUniformByName.find(refName);
			if (find_iter != mUniformByName.end()) {
				int refSlot = it.second.get<int>("<xmlattr>.Slot", find_iter->second.Slot);
				programNode.Uniforms.AddOrSet(find_iter->second, refSlot);
			}
		}

		int index = 0;
		for (auto& it : boost::make_iterator_range(nodeProgram->equal_range("Uniform"))) {
			auto& node_uniform = it.second;
			UniformNode uniform;
			uniform.IsUnique = node_uniform.get<bool>("<xmlattr>.IsUnique", true);

			ConstBufferDeclBuilder builder(uniform.Decl);

			int byteOffset = 0;
			for (auto& element : boost::make_iterator_range(node_uniform.equal_range("Element"))) {
				int size = element.second.get<int>("<xmlattr>.Size", 0); BOOST_ASSERT(size % 4 == 0);
				int count = element.second.get<int>("<xmlattr>.Count", 0);
				int offset = element.second.get<int>("<xmlattr>.Offset", byteOffset);
				ConstBufferElementType uniformElementType = ConvertStringToConstBufferElementType(
					element.second.get<std::string>("<xmlattr>.Type"/*, "int"*/), count, size);
				BOOST_ASSERT(uniformElementType != kCBElementMax);

				std::string Name = element.second.get<std::string>("<xmlattr>.Name"/*, ""*/);
				builder.Add(ConstBufferDeclElement(Name.c_str(), uniformElementType, size, count, offset));
				byteOffset = offset + size;

				int dataSize = uniform.Data.size();
				uniform.Data.resize(dataSize + size / 4);
				void* pData = &uniform.Data[dataSize];

				std::string strDefault = element.second.get<std::string>("<xmlattr>.Default", "");
				if (!strDefault.empty()) {
					switch (uniformElementType) {
					case kCBElementBool:
					case kCBElementInt: {
						*static_cast<int*>(pData) = boost::lexical_cast<int>(strDefault);
					}break;
					case kCBElementInt2:
					case kCBElementInt3:
					case kCBElementInt4: {
						std::vector<boost::iterator_range<std::string::iterator>> lst;
						boost::split(lst, strDefault, boost::is_any_of(","));
						const int count = size / sizeof(float);
						for (int i = 0; i < lst.size() && i < count; ++i)
							static_cast<int*>(pData)[i] = boost::lexical_cast<int>(lst[i]);
					}break;
					case kCBElementFloat: {
						*static_cast<float*>(pData) = boost::lexical_cast<float>(strDefault);
					}break;
					case kCBElementFloat2:
					case kCBElementFloat3:
					case kCBElementFloat4:
					case kCBElementMatrix: {
						std::vector<boost::iterator_range<std::string::iterator>> lst;
						boost::split(lst, strDefault, boost::is_any_of(","));
						const int count = size / sizeof(float);
						for (int i = 0; i < lst.size() && i < count; ++i)
							static_cast<float*>(pData)[i] = boost::lexical_cast<float>(lst[i]);
					}break;
					default:break;
					}
				}
			}
			int dataSize = uniform.Data.size();
			if (dataSize & 15) uniform.Data.resize((dataSize + 15) / 16 * 16);

			uniform.Slot = node_uniform.get<int>("<xmlattr>.Slot", -1);

			std::string Name = node_uniform.get<std::string>("<xmlattr>.Name");
			uniform.ShortName = node_uniform.get<std::string>("<xmlattr>.ShortName", Name);

			Name = node_uniform.get<std::string>("<xmlattr>.Name", boost::lexical_cast<std::string>(index));
			mUniformByName.insert(std::make_pair(Name, uniform));

			Name = PropertyTreePath(nodeProgram, node_uniform, index).Path.string();
			mUniformByName.insert(std::make_pair(Name, uniform));

			programNode.Uniforms.AddOrSet(std::move(uniform), uniform.Slot);
			++index;
		}
	}
	void VisitSamplers(const PropertyTreePath& nodeProgram, ProgramNode& programNode) {
		for (auto& it : boost::make_iterator_range(nodeProgram->equal_range("UseTexture"))) {
			std::string refName = it.second.data();
			auto find_iter = mSamplerSetByName.find(refName);
			if (find_iter != mSamplerSetByName.end()) {
				programNode.Samplers.MergeOverride(find_iter->second);
			}
		}

		int index = 0;
		for (auto& it : boost::make_iterator_range(nodeProgram->equal_range("Texture"))) {
			auto& node_texture = it.second;
			SamplerNode samplerSet;
			for (auto& it : boost::make_iterator_range(node_texture.equal_range("Element"))) {
				auto& node_element = it.second;
				CompareFunc cmpFunc = static_cast<CompareFunc>(node_element.get<int>("<xmlattr>.CompFunc", kCompareNever));

				SamplerFilterMode filter = (cmpFunc != kCompareNever) ? kSamplerFilterCmpMinMagLinearMipPoint : kSamplerFilterMinMagMipLinear;
				filter = static_cast<SamplerFilterMode>(node_element.get<int>("<xmlattr>.Filter", filter));

				int address = (cmpFunc != kCompareNever) ? kAddressBorder : kAddressClamp;
				address = node_element.get<int>("<xmlattr>.Address", address);

				int slot = node_element.get<int>("<xmlattr>.Slot", -1);
				samplerSet.AddOrSet(SamplerDesc::Make(
					filter,
					cmpFunc,
					static_cast<AddressMode>(node_element.get<int>("<xmlattr>.AddressU", address)),
					static_cast<AddressMode>(node_element.get<int>("<xmlattr>.AddressV", address)),
					static_cast<AddressMode>(node_element.get<int>("<xmlattr>.AddressW", address))
				), slot);
			}

			std::string Name = node_texture.get<std::string>("<xmlattr>.Name", boost::lexical_cast<std::string>(index));
			mSamplerSetByName.insert(std::make_pair(Name, samplerSet));

			Name = PropertyTreePath(nodeProgram, node_texture, index).Path.string();
			mSamplerSetByName.insert(std::make_pair(Name, samplerSet));

			programNode.Samplers.MergeOverride(std::move(samplerSet));
			++index;
		}
	}
	static void ParseProgram(const boost_property_tree::ptree& nodeProgram, ProgramNode& programNode) {
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
			for (auto& it : node_macros)
				vertexScd.AddOrSetMacro(ShaderCompileMacro{ it.first, it.second.data() });
		}
		pixelScd.Macros = vertexScd.Macros;

	#define SCD_ADD_MACRO(MACRO_NAME) vertexScd.MergeMacro(ShaderCompileMacro{ #MACRO_NAME, boost::lexical_cast<std::string>(MACRO_NAME) });
	#if defined DEBUG_SHADOW_CASTER
		SCD_ADD_MACRO(DEBUG_SHADOW_CASTER);
	#endif
	#if defined DEBUG_PREPASS_BASE
		SCD_ADD_MACRO(DEBUG_PREPASS_BASE);
	#endif
	#if defined DEBUG_PREPASS_FINAL
		SCD_ADD_MACRO(DEBUG_PREPASS_FINAL);
	#endif
	}
	void VisitProgram(const PropertyTreePath& nodeProgram, ProgramNode& programNode) {
		ParseProgram(nodeProgram.Node, programNode);
		VisitAttributes(nodeProgram, programNode);
		VisitUniforms(nodeProgram, programNode);
		VisitSamplers(nodeProgram, programNode);
	}

	void VisitSubShader(const PropertyTreePath& nodeTechnique, TechniqueNode& techniqueNode) {
		int index = 0;
		for (auto& it : boost::make_iterator_range(nodeTechnique->equal_range("Pass"))) {
			auto& node_pass = it.second;
			PassNode pass;

			pass.LightMode = LIGHTMODE_FORWARD_BASE;
			auto find_tags = node_pass.find("Tags");
			if (find_tags != node_pass.not_found()) {
				auto& node_tag = find_tags->second;
				pass.LightMode = node_tag.get<std::string>("LightMode", pass.LightMode);
			}

			pass.ShortName = node_pass.get<std::string>("ShortName", node_pass.get<std::string>("Name", ""));
			pass.Name = node_pass.get<std::string>("Name", boost::lexical_cast<std::string>(index));

			auto find_program = node_pass.find("PROGRAM");
			if (find_program != node_pass.not_found()) {
				auto& node_program = find_program->second;
				ParseProgram(node_program, pass.Program);
			}

			techniqueNode.Add(std::move(pass));
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
				VisitSubShader(PropertyTreePath(nodeCategory, it.second, index++), categNode.Emplace());
			}
		}
	}
	void VisitShader(const PropertyTreePath& nodeShader, ConstVisitorRef vis, ShaderNode& shaderNode) {
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
					passNode.Program.MergeNoOverride(categNode.Program);
					passNode.Program.Build();
				}
			}
		}
	}
	bool ParseShaderFile(const ShaderLoadParam& loadParam, ShaderNode& shaderNode) {
		bool result;
		auto find_iter = mShaderByParam.find(loadParam);
		if (find_iter == mShaderByParam.end()) {
			std::string filename = "shader/" + loadParam.ShaderName;
			if (!loadParam.VariantName.empty()) filename += "-" + loadParam.VariantName;
			filename += ".Shader";
			if (boost_filesystem::exists(boost_filesystem::system_complete(filename))) {
				boost_property_tree::ptree pt;
				boost_property_tree::read_xml(filename, pt);
				Visitor visitor{ false };
				VisitShader(pt.get_child("Shader"), visitor, shaderNode);
				BuildShaderNode(shaderNode);
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
	bool GetShaderVariantNode(const ShaderLoadParam& loadParam, ShaderNode& shaderNode) {
		bool result;
		auto find_iter = mShaderVariantByParam.find(loadParam);
		if (find_iter == mShaderVariantByParam.end()) {
			if (result = ParseShaderFile(loadParam, shaderNode)) {
				for (auto& categNode : shaderNode) {
					for (auto& techniqueNode : categNode) {
						for (auto& passNode : techniqueNode) {
							passNode.Program.VertexSCD.AddOrSetMacros(loadParam.Macros);
							passNode.Program.PixelSCD.AddOrSetMacros(loadParam.Macros);
						}
					}
				}
				BuildShaderNode(shaderNode);
				mShaderVariantByParam.insert(std::make_pair(loadParam, shaderNode));
			}
		}
		else {
			shaderNode = find_iter->second;
			result = true;
		}
		return result;
	}
};

/********** MaterialAssetManager **********/
MaterialAssetManager::MaterialAssetManager()
{
	mShaderNodeMng = CreateInstance<ShaderNodeManager>();
}
bool MaterialAssetManager::GetShaderNode(const ShaderLoadParam& loadParam, ShaderNode& shaderNode)
{
	return mShaderNodeMng->GetShaderNode(loadParam, shaderNode);
}

}
}
}