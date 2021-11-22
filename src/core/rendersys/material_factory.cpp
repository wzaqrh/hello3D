#include <unordered_map>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "core/rendersys/material_factory.h"
#include "core/rendersys/material.h"
#include "core/rendersys/material_cb.h"
#include "core/rendersys/resource_manager.h"
#include "core/rendersys/interface_type.h"
#include "core/renderable/post_process.h"
#include "core/base/d3d.h"

namespace boost_filesystem = boost::filesystem;
namespace boost_property_tree = boost::property_tree;

namespace mir {

/********** ConstBufferDeclBuilder **********/
struct ConstBufferDeclBuilder
{
	ConstBufferDecl& mDecl;
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
};
#define MAKE_CBDESC(CB) (CB::GetDesc())

/********** MaterialBuilder **********/
struct MaterialBuilder
{
	MaterialPtr mMaterial;
	TechniquePtr mCurTech;
	PassPtr mCurPass;
	ResourceManager& mResourceMng;
	Launch mLaunchMode;
public:
	MaterialBuilder(ResourceManager& resMng, Launch launchMode, bool addTechPass = true) 
		:mResourceMng(resMng), mLaunchMode(launchMode) {
		mMaterial = std::make_shared<Material>();
		if (addTechPass) {
			AddTechnique();
			AddPass(E_PASS_FORWARDBASE, "");
		}
	}
	MaterialBuilder(ResourceManager& resMng, MaterialPtr material) :mResourceMng(resMng) {
		mMaterial = material;
		mCurTech = material->CurTech();
		mCurPass = mCurTech->mPasses.empty() ? nullptr : mCurTech->mPasses[mCurTech->mPasses.size() - 1];
	}
	MaterialBuilder& AddTechnique(const std::string& name = "d3d11") {
		mCurTech = std::make_shared<Technique>();
		mCurTech->mName = name;
		mMaterial->AddTechnique(mCurTech);
		return *this;
	}
	MaterialBuilder& CloneTechnique(MaterialFactory& matFac, const std::string& name) {
		mCurTech = matFac.CloneTechnique(mLaunchMode, mResourceMng, *mCurTech);
		mCurTech->mName = name;
		mMaterial->AddTechnique(mCurTech);
		return *this;
	}
	MaterialBuilder& AddPass(const std::string& lightMode, const std::string& passName) {
		mCurPass = std::make_shared<Pass>(lightMode, passName);
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
	MaterialBuilder& AddSampler(ISamplerStatePtr sampler, int count = 1) {
		while (count-- > 0)
			mCurPass->AddSampler(sampler);
		mResourceMng.AddResourceDependency(mMaterial, sampler);
		return *this;
	}
	MaterialBuilder& AddSamplerToTech(ISamplerStatePtr sampler, int count = 1) {
		while (count-- > 0)
			mCurTech->AddSampler(sampler);
		mResourceMng.AddResourceDependency(mMaterial, sampler);
		return *this;
	}
	MaterialBuilder& ClearSamplersToTech() {
		mCurTech->ClearSamplers();
		return *this;
	}
	MaterialBuilder& AddConstBuffer(IContantBufferPtr buffer, const std::string& name = "", bool isUnique = true) {
		mCurPass->AddConstBuffer(CBufferEntry::Make(buffer, name, isUnique));
		mResourceMng.AddResourceDependency(mMaterial, buffer);
		return *this;
	}
	MaterialBuilder& AddConstBufferToTech(IContantBufferPtr buffer, const std::string& name = "", bool isUnique = true) {
		mCurTech->AddConstBuffer(CBufferEntry::Make(buffer, name, isUnique));
		mResourceMng.AddResourceDependency(mMaterial, buffer);
		return *this;
	}
	MaterialBuilder& SetRenderTarget(IRenderTexturePtr target) {
		mCurPass->mRenderTarget = target;
		mResourceMng.AddResourceDependency(mMaterial, target);
		return *this;
	}
	MaterialBuilder& AddIterTarget(IRenderTexturePtr target) {
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
		if (mLaunchMode == Launch::Async) mMaterial->SetPrepared();
		else mMaterial->SetLoaded();
		return mMaterial;
	}
};

/********** MaterialAssetManager **********/
struct XmlAttributeInfo {
	std::vector<LayoutInputElement> Layout;
	std::vector<std::string> LayoutStr;
public:
	XmlAttributeInfo() {}
	XmlAttributeInfo(const XmlAttributeInfo& other) {
		LayoutStr = other.LayoutStr;
		Layout = other.Layout;
		for (size_t i = 0; i < Layout.size(); ++i)
			Layout[i].SemanticName = LayoutStr[i].c_str();
	}
	XmlAttributeInfo& operator=(const XmlAttributeInfo& other) {
		LayoutStr = other.LayoutStr;
		Layout = other.Layout;
		for (size_t i = 0; i < Layout.size(); ++i)
			Layout[i].SemanticName = LayoutStr[i].c_str();
		return *this;
	}
};
struct XmlUniformInfo {
	ConstBufferDecl Decl;
	std::string ShortName;
	std::vector<float> Data;
	bool IsUnique;
};
struct XmlSamplersInfo {
	std::vector<std::pair<SamplerFilterMode, int>> Samplers;
	void Add(const XmlSamplersInfo& other) {
		Samplers.insert(Samplers.end(), other.Samplers.begin(), other.Samplers.end());
	}
	size_t size() const { return Samplers.size(); }
	const std::pair<SamplerFilterMode, int>& operator[](size_t pos) const { return Samplers[pos]; }
};
struct XmlProgramInfo {
	PrimitiveTopology Topo;
	XmlAttributeInfo Attr;
	std::vector<XmlUniformInfo> Uniforms;
	XmlSamplersInfo Samplers;
	std::string FxName, VsEntry;
public:
	void AddUniform(const XmlUniformInfo& uniform) {
		Uniforms.push_back(uniform);
	}
	void AddAttribute(const XmlAttributeInfo& attr) {
		Attr = attr;
	}
	void AddSamplers(const XmlSamplersInfo& samplers) {
		Samplers.Add(samplers);
	}
};
struct XmlPassInfo {
	std::string LightMode, Name, ShortName, PSEntry;
public:
	XmlPassInfo() {}
};
struct XmlSubShaderInfo {
	std::vector<XmlPassInfo> Passes;
public:
	void AddPass(XmlPassInfo&& pass) {
		Passes.push_back((pass));
	}
};
struct XmlShaderInfo {
	XmlProgramInfo Program;
	std::vector<XmlSubShaderInfo> SubShaders;
public:
	void AddSubShader(XmlSubShaderInfo&& subShader) {
		SubShaders.push_back((subShader));
	}
};

struct MaterialAssetEntry {
	std::string ShaderName;
	std::string VariantName;
};
class MaterialNameToAssetMapping : boost::noncopyable {
	std::unordered_map<std::string, MaterialAssetEntry> mMatEntryByMatName;
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
};

struct MaterialAsset {
	XmlShaderInfo ShaderInfo;
};

class MaterialAssetManager
{
	std::map<std::string, XmlShaderInfo> mIncludeByName, mShaderByName, mShaderVariantByName;
	std::map<std::string, XmlAttributeInfo> mAttrByName;
	std::map<std::string, XmlUniformInfo> mUniformByName;
	std::map<std::string, XmlSamplersInfo> mSamplersByName;
	std::shared_ptr<MaterialNameToAssetMapping> mMatNameToAsset;
public:
	MaterialAssetManager() {
		mMatNameToAsset = std::make_shared<MaterialNameToAssetMapping>();
		mMatNameToAsset->InitFromXmlFile("shader/Config.xml");
	}
	bool GetMaterialAsset(Launch launchMode,
		ResourceManager& resourceMng,
		const std::string& shaderName,
		const std::string& variantName,
		MaterialAsset& materialAsset) {
		if (!variantName.empty()) return ParseShaderVariantXml(shaderName, variantName, materialAsset.ShaderInfo);
		else return ParseShaderXml(shaderName, materialAsset.ShaderInfo);
	}
public:
	const MaterialNameToAssetMapping& MatNameToAsset() const {
		return *mMatNameToAsset;
	}
private:
	struct Visitor {
		const bool JustInclude;
		XmlShaderInfo& shaderInfo;
	};
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
			XmlShaderInfo shaderInfo;
			ParseShaderXml(shaderName, shaderInfo);
			mIncludeByName.insert(std::make_pair(shaderName, shaderInfo));
		}
	}

	static ConstBufferElementType ConvertStringToConstBufferElementType(
		const std::string& str, int count, int& size) {
		ConstBufferElementType result = kCBElementMax;
		if (str == "int") result = kCBElementInt, size = 4;
		else if (str == "float") result = kCBElementFloat, size = 4;
		else if (str == "float4") result = kCBElementFloat4, size = 16;
		else if (str == "matrix") result = kCBElementMatrix, size = 64;
		else if (str == "struct") result = kCBElementStruct;
		size *= std::max<int>(1, count);
		return result;
	}
	void VisitAttributes(const PropertyTreePath& nodeProgram, Visitor& vis) {
		for (auto& it : boost::make_iterator_range(nodeProgram->equal_range("UseAttribute"))) {
			std::string refName = it.second.data();
			auto find_iter = mAttrByName.find(refName);
			if (find_iter != mAttrByName.end()) {
				vis.shaderInfo.Program.AddAttribute(find_iter->second);
			}
		}

		int index = 0;
		for (auto& it : boost::make_iterator_range(nodeProgram->equal_range("Attribute"))) {
			XmlAttributeInfo attribute;

			int elementCount = it.second.count("Element");
			attribute.Layout.resize(elementCount);
			attribute.LayoutStr.resize(elementCount);
			int byteOffset = 0, j = 0;
			for (auto& element : boost::make_iterator_range(it.second.equal_range("Element"))) {
				attribute.LayoutStr[j] = element.second.get<std::string>("<xmlattr>.SemanticName");
				auto& layoutJ = attribute.Layout[j];
				layoutJ = LayoutInputElement{
					attribute.LayoutStr[j],
					element.second.get<UINT>("<xmlattr>.SemanticIndex", 0),
					static_cast<ResourceFormat>(element.second.get<UINT>("<xmlattr>.Format")),
					element.second.get<UINT>("<xmlattr>.InputSlot", 0),
					element.second.get<UINT>("<xmlattr>.ByteOffset", byteOffset),
					kLayoutInputPerVertexData,
					0
				};
				byteOffset = layoutJ.AlignedByteOffset + d3d::BytePerPixel(static_cast<DXGI_FORMAT>(layoutJ.Format));
				++j;
			}

			std::string Name = it.second.get<std::string>("<xmlattr>.Name", boost::lexical_cast<std::string>(index));
			mAttrByName.insert(std::make_pair(Name, attribute));

			Name = PropertyTreePath(nodeProgram, it.second, index).Path.string();
			mAttrByName.insert(std::make_pair(Name, attribute));

			vis.shaderInfo.Program.AddAttribute(attribute);
			++index;
		}
	}
	void VisitUniforms(const PropertyTreePath& nodeProgram, Visitor& vis) {
		for (auto& it : boost::make_iterator_range(nodeProgram->equal_range("UseUniform"))) {
			std::string refName = it.second.data();
			auto find_iter = mUniformByName.find(refName);
			if (find_iter != mUniformByName.end()) {
				vis.shaderInfo.Program.AddUniform(find_iter->second);
			}
		}

		int index = 0;
		for (auto& it : boost::make_iterator_range(nodeProgram->equal_range("Uniform"))) {
			XmlUniformInfo uniform;
			uniform.IsUnique = it.second.get<bool>("<xmlattr>.IsUnique", true);

			ConstBufferDeclBuilder builder(uniform.Decl);

			int byteOffset = 0;
			for (auto& element : boost::make_iterator_range(it.second.equal_range("Element"))) {
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

				if (element.second.find("<xmlattr>.Default") != element.second.not_found()) {
					std::string strDefault = element.second.get<std::string>("<xmlattr>.Default");
					switch (uniformElementType) {
					case kCBElementInt: {
						*static_cast<int*>(pData) = boost::lexical_cast<int>(strDefault);
					}break;
					case kCBElementFloat: {
						*static_cast<float*>(pData) = boost::lexical_cast<float>(strDefault);
					}break;
					case kCBElementFloat4:
					case kCBElementMatrix: {
						std::vector<boost::iterator_range<std::string::iterator>> lst;
						boost::split(lst, strDefault, boost::is_any_of(","));
						int count = (uniformElementType == kCBElementFloat4) ? 4 : 16;
						for (int i = 0; i < lst.size() && i < count; ++i)
							static_cast<float*>(pData)[i] = boost::lexical_cast<float>(lst[i]);
					}break;
					default:break;
					}
				}
			}

			std::string Name = it.second.get<std::string>("<xmlattr>.Name");
			uniform.ShortName = it.second.get<std::string>("<xmlattr>.ShortName", Name);

			Name = it.second.get<std::string>("<xmlattr>.Name", boost::lexical_cast<std::string>(index));
			mUniformByName.insert(std::make_pair(Name, uniform));

			Name = PropertyTreePath(nodeProgram, it.second, index).Path.string();
			mUniformByName.insert(std::make_pair(Name, uniform));

			vis.shaderInfo.Program.AddUniform(uniform);
			++index;
		}
	}
	void VisitSamplers(const PropertyTreePath& nodeProgram, Visitor& vis) {
		int index = 0;
		for (auto& it : boost::make_iterator_range(nodeProgram->equal_range("Sampler"))) {
			XmlSamplersInfo sampler;
			for (auto& element : it.second.get_child("Element")) {
				sampler.Samplers.emplace_back(std::make_pair(
					static_cast<SamplerFilterMode>(element.second.get<int>("<xmlattr>.Slot", 0)),
					element.second.get<int>("<xmlattr>.Filter", kSamplerFilterMinMagMipLinear)
				));
			}

			std::string Name = it.second.get<std::string>("<xmlattr>.Name", boost::lexical_cast<std::string>(index));
			mSamplersByName.insert(std::make_pair(Name, sampler));

			Name = PropertyTreePath(nodeProgram, it.second, index).Path.string();
			mSamplersByName.insert(std::make_pair(Name, sampler));

			vis.shaderInfo.Program.AddSamplers(sampler);
			++index;
		}
	}
	void VisitProgram(const PropertyTreePath& nodeProgram, Visitor& vis) {
		vis.shaderInfo.Program.FxName = nodeProgram->get<std::string>("FileName", "");
		vis.shaderInfo.Program.VsEntry = nodeProgram->get<std::string>("VertexEntry", "");
		vis.shaderInfo.Program.Topo = static_cast<PrimitiveTopology>(
			nodeProgram->get<int>("Topology", kPrimTopologyTriangleList));
		VisitAttributes(nodeProgram, vis);
		VisitUniforms(nodeProgram, vis);
		VisitSamplers(nodeProgram, vis);
	}

	void VisitSubShader(const PropertyTreePath& nodeTechnique, Visitor& vis) {
		XmlSubShaderInfo subShader;
		int index = 0;
		for (auto& it : boost::make_iterator_range(nodeTechnique->equal_range("Pass"))) {
			auto& node_pass = it.second;
			XmlPassInfo pass;

			pass.LightMode = E_PASS_FORWARDBASE;
			auto find_tags = node_pass.find("Tags");
			if (find_tags != node_pass.not_found()) {
				auto& node_tag = find_tags->second;
				pass.LightMode = node_tag.get<std::string>("LightMode", pass.LightMode);
			}

			pass.ShortName = node_pass.get<std::string>("ShortName", node_pass.get<std::string>("Name", ""));
			pass.Name = node_pass.get<std::string>("Name", boost::lexical_cast<std::string>(index));

			pass.PSEntry = "PS";
			auto find_program = node_pass.find("PROGRAM");
			if (find_program != node_pass.not_found()) {
				auto& node_program = find_program->second;
				pass.PSEntry = node_program.get<std::string>("PixelEntry", pass.PSEntry);
			}

			subShader.AddPass(std::move(pass));
			++index;
		}
		vis.shaderInfo.AddSubShader(std::move(subShader));
	}
	void VisitShader(const PropertyTreePath& nodeShader, Visitor& vis) {
		for (auto& it : boost::make_iterator_range(nodeShader->equal_range("Include"))) {
			VisitInclude(it.second.data());
		}

		int index = 0;
		for (auto& it : boost::make_iterator_range(nodeShader->equal_range("PROGRAM"))) {
			VisitProgram(PropertyTreePath(nodeShader, it.second, index++), vis);
		}

		if (!vis.JustInclude) {
			index = 0;
			for (auto& it : boost::make_iterator_range(nodeShader->equal_range("SubShader"))) {
				VisitSubShader(PropertyTreePath(nodeShader, it.second, index++), vis);
			}
		}
	}
	void VisitShaderVariant(const PropertyTreePath& nodeVariant,
		const std::string& variantName,
		XmlShaderInfo& shaderInfo) {
		for (auto& it : boost::make_iterator_range(nodeVariant->equal_range("UseShader"))) {
			ParseShaderXml(it.second.data(), shaderInfo);
		}

		for (auto& it : boost::make_iterator_range(nodeVariant->equal_range("Variant"))) {
			std::string name = it.second.get<std::string>("<xmlattr>.Name");
			if (name == variantName) {
				shaderInfo.Program.Topo = static_cast<PrimitiveTopology>(
					it.second.get<int>("Topology"), shaderInfo.Program.Topo);
			}
		}
	}

	bool ParseShaderXml(const std::string& shaderName, XmlShaderInfo& shaderInfo) {
		bool result;
		auto find_iter = mShaderByName.find(shaderName);
		if (find_iter == mShaderByName.end()) {
			std::string filename = "shader/" + shaderName + ".xml";
			if (boost_filesystem::exists(boost_filesystem::system_complete(filename))) {
				boost_property_tree::ptree pt;
				boost_property_tree::read_xml(filename, pt);
				Visitor visitor{ false, shaderInfo };
				VisitShader(pt.get_child("Shader"), visitor);
				mShaderByName.insert(std::make_pair(shaderName, visitor.shaderInfo));
				result = true;
			}
			else {
				result = false;
			}
		}
		else {
			shaderInfo = find_iter->second;
			result = true;
		}
		return result;
	}
	bool ParseShaderVariantXml(const std::string& shaderName,
		const std::string& variantName,
		XmlShaderInfo& shaderInfo) {
		bool result;
		std::string strKey = shaderName + "/" + variantName;
		auto find_iter = mShaderVariantByName.find(strKey);
		if (find_iter == mShaderVariantByName.end()) {
			std::string filename = "shader/" + shaderName + "Variant.xml";
			if (boost_filesystem::exists(boost_filesystem::system_complete(filename))) {
				boost_property_tree::ptree pt;
				boost_property_tree::read_xml(filename, pt);
				VisitShaderVariant(pt.get_child("Variant"), variantName, shaderInfo);
				mShaderVariantByName.insert(std::make_pair(strKey, shaderInfo));
				result = true;
			}
			else {
				result = false;
			}
		}
		else {
			shaderInfo = find_iter->second;
			result = true;
		}
		return result;
	}
};

/********** TMaterialFactory **********/
MaterialFactory::MaterialFactory()
{
	mMatAssetMng = std::make_shared<MaterialAssetManager>();
}

MaterialPtr MaterialFactory::CreateMaterialByMaterialAsset(Launch launchMode, 
	ResourceManager& resourceMng, const MaterialAsset& matAsset) 
{
	MaterialBuilder builder(resourceMng, launchMode, false);
	builder.AddTechnique("d3d11");

	const auto& shaderInfo = matAsset.ShaderInfo;
	for (size_t i = 0; i < shaderInfo.SubShaders.size(); ++i) {
		auto& subShaderInfo = shaderInfo.SubShaders[i];
		for (size_t j = 0; j < subShaderInfo.Passes.size(); ++j) {
			auto& passInfo = subShaderInfo.Passes[j];

			builder.AddPass(passInfo.LightMode, passInfo.ShortName/*, i == 0*/);
			builder.SetTopology(shaderInfo.Program.Topo);

			IProgramPtr program = builder.SetProgram(resourceMng.CreateProgram(
				launchMode, shaderInfo.Program.FxName, shaderInfo.Program.VsEntry, passInfo.PSEntry));
			builder.SetInputLayout(resourceMng.CreateLayout(
				launchMode, program, shaderInfo.Program.Attr.Layout));

			for (size_t k = 0; k < shaderInfo.Program.Samplers.size(); ++k) {
				const auto& elem = shaderInfo.Program.Samplers[k];
				builder.AddSampler(resourceMng.CreateSampler(
					launchMode, elem.first, kCompareNever));
			}
		}
	}

	for (size_t i = 0; i < shaderInfo.Program.Uniforms.size(); ++i) {
		auto& uniformI = shaderInfo.Program.Uniforms[i];
		builder.AddConstBufferToTech(resourceMng.CreateConstBuffer(
			launchMode, uniformI.Decl, (void*)&uniformI.Data[0]), uniformI.ShortName, uniformI.IsUnique);
	}

	builder.CloneTechnique(*this, "d3d9");
	builder.ClearSamplersToTech();
	builder.AddSamplerToTech(resourceMng.CreateSampler(launchMode, kSamplerFilterMinMagMipLinear, kCompareNever));

	return builder.Build();
}

MaterialPtr MaterialFactory::CreateMaterial(Launch launchMode, ResourceManager& resourceMng, const std::string& matName) {
	MaterialAsset matAsset;
	auto entry = mMatAssetMng->MatNameToAsset()(matName);
	if (mMatAssetMng->GetMaterialAsset(launchMode, resourceMng, entry.ShaderName, entry.VariantName, matAsset))
		return CreateMaterialByMaterialAsset(launchMode, resourceMng, matAsset);
	else {
		MaterialPtr mat = std::make_shared<Material>();
		mat->SetLoaded(false);
		return mat;
	}
}

}