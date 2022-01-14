#include <unordered_map>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "core/base/d3d.h"
#include "core/base/macros.h"
#include "core/base/template/container_adapter.h"
#include "core/base/rendersys_debug.h"
#include "core/resource/material_factory.h"
#include "core/resource/resource_manager.h"

namespace boost_filesystem = boost::filesystem;
namespace boost_property_tree = boost::property_tree;

namespace mir {

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

/********** MaterialBuilder **********/
struct MaterialBuilder
{
public:
	MaterialBuilder(ResourceManager& resMng, Launch launchMode, MaterialPtr mat = nullptr)
		:mResourceMng(resMng), mLaunchMode(launchMode) {
		mMaterial = IF_OR(mat, CreateInstance<Material>());
		mMaterial->SetPrepared();
	}
	MaterialBuilder& AddTechnique() {
		mCurTech = CreateInstance<Technique>();
		mMaterial->AddTechnique(mCurTech);
		return *this;
	}
	MaterialBuilder& CloneTechnique(MaterialFactory& matFac) {
		mCurTech = matFac.CloneTechnique(mLaunchMode, mResourceMng, *mCurTech);
		mMaterial->AddTechnique(mCurTech);
		return *this;
	}
	MaterialBuilder& AddPass(const std::string& lightMode, const std::string& passName) {
		mCurPass = CreateInstance<Pass>(lightMode, passName);
		mCurTech->AddPass(mCurPass);
		return *this;
	}
	MaterialBuilder& SetPassName(const std::string& lightMode, const std::string& passName) {
		mCurPass->mLightMode = lightMode;
		mCurPass->mName = passName;
		return *this;
	}

	MaterialBuilder& SetInputLayout(IInputLayoutPtr inputLayout) {
		mCurPass->mInputLayout = inputLayout;
		mResourceMng.AddResourceDependency(mMaterial, inputLayout);
		return *this;
	}
	MaterialBuilder& SetTopology(PrimitiveTopology topology) {
		mCurPass->mTopoLogy = topology;
		return *this;
	}
	IProgramPtr SetProgram(IProgramPtr program) {
		mCurPass->mProgram = program;
		mResourceMng.AddResourceDependency(mMaterial, program);
		return program;
	}
	MaterialBuilder& AddSampler(ISamplerStatePtr sampler) {
		mCurPass->AddSampler(sampler);
		mResourceMng.AddResourceDependency(mMaterial, sampler);
		return *this;
	}
	MaterialBuilder& AddSamplerToTech(ISamplerStatePtr sampler) {
		mCurTech->AddSampler(sampler);
		mResourceMng.AddResourceDependency(mMaterial, sampler);
		return *this;
	}
	MaterialBuilder& ClearSamplersToTech() {
		mCurTech->ClearSamplers();
		return *this;
	}
	MaterialBuilder& AddConstBuffer(IContantBufferPtr buffer,
		const std::string& name = "", bool isUnique = true, int slot = -1) {
		mCurPass->AddConstBuffer(CBufferEntry::Make(buffer, name, isUnique), slot);
		mResourceMng.AddResourceDependency(mMaterial, buffer);
		return *this;
	}
	MaterialBuilder& AddConstBufferToTech(IContantBufferPtr buffer,
		const std::string& name = "", bool isUnique = true, int slot = -1) {
		mCurTech->AddConstBuffer(CBufferEntry::Make(buffer, name, isUnique), slot);
		mResourceMng.AddResourceDependency(mMaterial, buffer);
		return *this;
	}
	MaterialBuilder& SetFrameBuffer(IFrameBufferPtr target) {
		mCurPass->mRenderTarget = target;
		mResourceMng.AddResourceDependency(mMaterial, target);
		return *this;
	}
	MaterialBuilder& AddIterTarget(IFrameBufferPtr target) {
		mCurPass->AddIterTarget(target);
		mResourceMng.AddResourceDependency(mMaterial, target);
		return *this;
	}
	MaterialBuilder& SetTexture(size_t slot, ITexturePtr texture) {
		mCurPass->mTextures[slot] = texture;
		mResourceMng.AddResourceDependency(mMaterial, texture);
		return *this;
	}
	MaterialPtr Build() {
		if (mLaunchMode == LaunchSync)
			mMaterial->SetLoaded();
		return mMaterial;
	}
private:
	MaterialPtr mMaterial;
	TechniquePtr mCurTech;
	PassPtr mCurPass;
	ResourceManager& mResourceMng;
	Launch mLaunchMode;
};

/********** MaterialAssetManager **********/
namespace mat_asset {
struct AttributeNode {
	std::vector<LayoutInputElement> Layout;
};
struct UniformNode {
	bool IsValid() const { return !Data.empty(); }
public:
	ConstBufferDecl Decl;
	std::string ShortName;
	std::vector<float> Data;
	bool IsUnique;
	int Slot;
};
struct SamplerNode : public VectorAdapterEx<SamplerDesc> {
	SamplerNode() :VectorAdapterEx<SamplerDesc>([](const SamplerDesc& v) {return v.CmpFunc != kCompareUnkown; }) {}
};

struct ShaderCompileDescEx : public ShaderCompileDesc {
	void MergeMacro(const ShaderCompileMacro& macro) {
		auto find_it = std::find_if(this->Macros.begin(), this->Macros.end(), [&macro](const ShaderCompileMacro& elem)->bool {
			return elem.Name == macro.Name;
		});
		if (find_it == this->Macros.end()) this->Macros.push_back(macro);
		else find_it->Definition = macro.Definition;
	}
	void MergeMacros(const std::vector<ShaderCompileMacro>& macros) {
		for (const auto& it : macros)
			MergeMacro(it);
	}
	void MergeMacros(const ShaderCompileDesc& other) {
		MergeMacros(other.Macros);
	}
};
struct AttributeNodeVector : public VectorAdapter<AttributeNode> {};
struct UniformNodeVector : public VectorAdapter<UniformNode> {};
struct ProgramNode {
	void MergeOverride(const ProgramNode& other) {
		Topo = other.Topo;
		Attrs = other.Attrs;
		Uniforms = other.Uniforms;
		Samplers = other.Samplers;
		VertexSCD.MergeMacros(other.VertexSCD);
		PixelSCD.MergeMacros(other.PixelSCD);
	}
public:
	PrimitiveTopology Topo = kPrimTopologyTriangleList;
	AttributeNodeVector Attrs;
	UniformNodeVector Uniforms;
	SamplerNode Samplers;
	ShaderCompileDescEx VertexSCD, PixelSCD;
};

struct PassNode {
	ProgramNode Program;
	std::string LightMode, Name, ShortName;
};
struct TechniqueNode : public VectorAdapter<PassNode> {};
struct CategoryNode : public VectorAdapter<TechniqueNode> {
	ProgramNode Program;
};
struct ShaderNode : public VectorAdapter<CategoryNode> {
	ShaderNode() {
		Add(CategoryNode());
	}
};
struct MaterialAsset {
	ShaderNode Shader;
};

struct MaterialAssetEntry {
	std::string ShaderName;
	std::string VariantName;
};
class MaterialNameToAssetMapping : boost::noncopyable {
public:
	bool InitFromXmlFile(const std::string& xmlFilePath) {
		bool result = false;
		std::string filename = xmlFilePath;
		if (boost_filesystem::exists(boost_filesystem::system_complete(filename))) {
			boost_property_tree::ptree pt;
			boost_property_tree::read_xml(filename, pt);
			VisitConfig(pt);
			result = true;
		}
		return result;
	}

	MaterialAssetEntry MaterialAssetEntryByMatName(const std::string& matName) const {
		auto find_iter = mMatEntryByMatName.find(matName);
		if (find_iter != mMatEntryByMatName.end()) {
			return find_iter->second;
		}
		else {
			return MaterialAssetEntry{ matName, "" };
		}
	}
	MaterialAssetEntry operator()(const std::string& matName) const {
		return MaterialAssetEntryByMatName(matName);
	}
private:
	void Clear() {
		mMatEntryByMatName.clear();
	}
	void VisitMaterial(const boost_property_tree::ptree& nodeMaterial) {
		for (auto& it : nodeMaterial) {
			MaterialAssetEntry entry;
			entry.ShaderName = it.second.get<std::string>("ShaderName", it.first);
			entry.VariantName = it.second.get<std::string>("ShaderVariantName", "");
			mMatEntryByMatName.insert(std::make_pair(it.first, entry));
		}
	}
	void VisitConfig(const boost_property_tree::ptree& nodeConfig) {
		for (auto& it : boost::make_iterator_range(nodeConfig.equal_range("Config.Material"))) {
			VisitMaterial(it.second);
		}
	}
private:
	std::unordered_map<std::string, MaterialAssetEntry> mMatEntryByMatName;
};

class MaterialAssetManager
{
	std::map<std::string, ShaderNode> mIncludeByName, mShaderByName, mShaderVariantByName;
	std::map<std::string, AttributeNode> mAttrByName;
	std::map<std::string, UniformNode> mUniformByName;
	std::map<std::string, SamplerNode> mSamplerSetByName;
	std::shared_ptr<MaterialNameToAssetMapping> mMatNameToAsset;
public:
	MaterialAssetManager() {
		mMatNameToAsset = CreateInstance<MaterialNameToAssetMapping>();
		mMatNameToAsset->InitFromXmlFile("shader/Config.xml");
	}
	bool GetMaterialAsset(Launch launchMode, ResourceManager& resourceMng, const MaterialLoadParam& matParam, 
		MaterialAsset& materialAsset) {
		if (matParam.IsVariant()) return ParseShaderVariantXml(matParam, materialAsset.Shader);
		else return ParseShaderXml(matParam.ShaderName, materialAsset.Shader);
	}
public:
	const MaterialNameToAssetMapping& MatNameToAsset() const {
		return *mMatNameToAsset;
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
			ParseShaderXml(shaderName, shaderNode);
			mIncludeByName.insert(std::make_pair(shaderName, shaderNode));
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
			if (dataSize & 15) uniform.Data.resize((dataSize + 15)/16*16);

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
				vertexScd.MergeMacro(ShaderCompileMacro{ it.first, it.second.data() });
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

	void VisitSubShader(const PropertyTreePath& nodeTechnique, const CategoryNode& categNode, TechniqueNode& techniqueNode) {
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

			pass.Program = categNode.Program;
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
				VisitSubShader(PropertyTreePath(nodeCategory, it.second, index++), categNode, categNode.Emplace());
			}
		}
	}
	void VisitShader(const PropertyTreePath& nodeShader, ConstVisitorRef vis, ShaderNode& shaderNode) {
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
		for (auto& categ : shaderNode) {
			for (auto& tech : categ) {
				for (auto& pass : tech) {
					pass.Program.VertexSCD.MergeMacros(categ.Program.VertexSCD);
					pass.Program.PixelSCD.MergeMacros(categ.Program.PixelSCD);
				}
			}
		}
	}
	void VisitVariant(const PropertyTreePath& nodeVariant, ShaderNode& shaderNode) {
		for (auto& it : boost::make_iterator_range(nodeVariant->equal_range("UseShader"))) {
			ParseShaderXml(it.second.data(), shaderNode);
		}

		VisitShader(nodeVariant, Visitor{false}, shaderNode);
	}
	void VisitShaderVariants(const PropertyTreePath& nodeShaderVariant, const std::string& shaderName, const std::string& variantName, ShaderNode& shaderNode) {
		for (auto& it : boost::make_iterator_range(nodeShaderVariant->equal_range("Variant"))) {
			auto& node_variant = it.second;
			std::string useShader = node_variant.get<std::string>("UseShader");
			if (useShader == shaderName) {
				std::string name = node_variant.get<std::string>("<xmlattr>.Name");
				if (name == variantName) {
					VisitVariant(node_variant, shaderNode);
					return;
				}

				for (auto& it : boost::make_iterator_range(node_variant.equal_range("AliasName"))) {
					if (it.second.data() == variantName) {
						VisitVariant(node_variant, shaderNode);
						return;
					}
				}
			}
		}
	}

	bool ParseShaderXml(const std::string& shaderName, ShaderNode& shaderNode) {
		bool result;
		auto find_iter = mShaderByName.find(shaderName);
		if (find_iter == mShaderByName.end()) {
			std::string filename = "shader/" + shaderName + ".xml";
			if (boost_filesystem::exists(boost_filesystem::system_complete(filename))) {
				boost_property_tree::ptree pt;
				boost_property_tree::read_xml(filename, pt);
				Visitor visitor{ false };
				VisitShader(pt.get_child("Shader"), visitor, shaderNode);
				BuildShaderNode(shaderNode);
				mShaderByName.insert(std::make_pair(shaderName, shaderNode));
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
	bool ParseShaderVariantXml(const MaterialLoadParam& matParam, ShaderNode& shaderNode) {
		bool result;
		std::string strKey = matParam.ShaderName + "/" + matParam.CalcVariantName();
		auto find_iter = mShaderVariantByName.find(strKey);
		if (find_iter == mShaderVariantByName.end()) {
			if (!matParam.VariantName.empty()) {
				std::string filename = "shader/Variants.xml";
				if (result = boost_filesystem::exists(boost_filesystem::system_complete(filename))) {
					boost_property_tree::ptree pt;
					boost_property_tree::read_xml(filename, pt);
					VisitShaderVariants(pt.get_child("ShaderVariants"), matParam.ShaderName, matParam.VariantName, shaderNode);
					BuildShaderNode(shaderNode);
					mShaderVariantByName.insert(std::make_pair(strKey, shaderNode));
				}
			}
			else {
				if (result = ParseShaderXml(matParam.ShaderName, shaderNode)) { 
					for (auto& categNode : shaderNode) {
						for (auto& techniqueNode : categNode) {
							for (auto& passNode : techniqueNode) {
								passNode.Program.VertexSCD.MergeMacros(matParam.Macros);
								passNode.Program.PixelSCD.MergeMacros(matParam.Macros);
							}
						}
					}
					BuildShaderNode(shaderNode);
					mShaderVariantByName.insert(std::make_pair(strKey, shaderNode));
				}
			}
		}
		else {
			shaderNode = find_iter->second;
			result = true;
		}
		return result;
	}
};
}

/********** TMaterialFactory **********/
MaterialFactory::MaterialFactory()
{
	mMatAssetMng = CreateInstance<mat_asset::MaterialAssetManager>();
}

MaterialPtr MaterialFactory::CreateMaterialByMaterialAsset(Launch launchMode, 
	ResourceManager& resourceMng, const mat_asset::MaterialAsset& matAsset, MaterialPtr matRes)
{
	MaterialBuilder builder(resourceMng, launchMode, matRes);

	const auto& shaderNode = matAsset.Shader;
	for (const auto& categNode : shaderNode) {
		for (const auto& techniqueNode : categNode) {
			builder.AddTechnique();

			for (const auto& passNode : techniqueNode) {
				const auto& passProgram = passNode.Program;
				builder.AddPass(passNode.LightMode, passNode.ShortName/*, i == 0*/);
				builder.SetTopology(categNode.Program.Topo);

				IProgramPtr program = builder.SetProgram(resourceMng.CreateProgram(launchMode,
					categNode.Program.VertexSCD.SourcePath, passProgram.VertexSCD, passProgram.PixelSCD));

				BOOST_ASSERT(categNode.Program.Attrs.Count() >= 0);
				if (categNode.Program.Attrs.Count() == 1) {
					builder.SetInputLayout(resourceMng.CreateLayout(launchMode,
						program, categNode.Program.Attrs[0].Layout));
				}
				else if (categNode.Program.Attrs.Count() > 1) {
					auto layout_compose = categNode.Program.Attrs[0].Layout;
					int slot = 1;
					for (const auto& attrNode : boost::make_iterator_range(categNode.Program.Attrs.Range(1))) {
						for (const auto& element_slot : attrNode.Layout) {
							layout_compose.push_back(element_slot);
							layout_compose.back().InputSlot = slot;
						}
						builder.SetInputLayout(resourceMng.CreateLayout(launchMode, program, layout_compose));
						++slot;
					}
				}

				for (const auto& sampler : categNode.Program.Samplers) {
					builder.AddSampler(sampler.CmpFunc != kCompareUnkown
						? resourceMng.CreateSampler(launchMode, sampler)
						: nullptr);
				}

				for (auto& uniform : categNode.Program.Uniforms) {
					builder.AddConstBuffer(resourceMng.CreateConstBuffer(launchMode,
						uniform.Decl, kHWUsageDynamic, Data::Make(uniform.Data)),
						uniform.ShortName, uniform.IsUnique, uniform.Slot);
				}
			}//for techniqueNode.Passes
		}//for shaderNode.SubShaders
	}//for shaderNode.Categories
	return builder.Build();
}

MaterialPtr MaterialFactory::CreateMaterial(Launch launchMode, ResourceManager& resourceMng, 
	const MaterialLoadParam& matParam, MaterialPtr matRes) {
	mat_asset::MaterialAsset matAsset;
	if (mMatAssetMng->GetMaterialAsset(launchMode, resourceMng, matParam, matAsset)) {
		return CreateMaterialByMaterialAsset(launchMode, resourceMng, matAsset, matRes);
	}
	else {
		matRes = IF_OR(matRes, CreateInstance<Material>());
		matRes->SetLoaded(false);
		return matRes;
	}
}

}