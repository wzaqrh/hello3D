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
#include "core/rendersys/render_system.h"
#include "core/rendersys/interface_type.h"
#include "core/renderable/post_process.h"
#include "core/base/utility.h"

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
public:
	MaterialBuilder(bool addTechPass = true) {
		mMaterial = std::make_shared<Material>();
		if (addTechPass) {
			AddTechnique();
			AddPass(E_PASS_FORWARDBASE, "");
		}
	}
	MaterialBuilder(MaterialPtr material) {
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
	MaterialBuilder& CloneTechnique(IRenderSystem& pRenderSys, const std::string& name) {
		mCurTech = mCurTech->Clone(pRenderSys);
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
		mMaterial->AddDependency(inputLayout->AsRes());
		return *this;
	}
	MaterialBuilder& SetTopology(D3D11_PRIMITIVE_TOPOLOGY topology) {
		mCurPass->mTopoLogy = topology;
		return *this;
	}
	IProgramPtr SetProgram(IProgramPtr program) {
		mCurPass->mProgram = program;
		mMaterial->AddDependency(program->AsRes());
		return program;
	}
	MaterialBuilder& AddSampler(ISamplerStatePtr sampler, int count = 1) {
		while (count-- > 0)
			mCurPass->AddSampler(sampler);
		return *this;
	}
	MaterialBuilder& AddSamplerToTech(ISamplerStatePtr sampler, int count = 1) {
		while (count-- > 0)
			mCurTech->AddSampler(sampler);
		return *this;
	}
	MaterialBuilder& ClearSamplersToTech() {
		mCurTech->ClearSamplers();
		return *this;
	}
	MaterialBuilder& AddConstBuffer(IContantBufferPtr buffer, const std::string& name = "", bool isUnique = true) {
		mCurPass->AddConstBuffer(CBufferEntry::Make(buffer, name, isUnique));
		return *this;
	}
	MaterialBuilder& AddConstBufferToTech(IContantBufferPtr buffer, const std::string& name = "", bool isUnique = true) {
		mCurTech->AddConstBuffer(CBufferEntry::Make(buffer, name, isUnique));
		return *this;
	}
	MaterialBuilder& SetRenderTarget(IRenderTexturePtr target) {
		mCurPass->mRenderTarget = target;
		return *this;
	}
	MaterialBuilder& AddIterTarget(IRenderTexturePtr target) {
		mCurPass->AddIterTarget(target);
		return *this;
	}
	MaterialBuilder& SetTexture(size_t slot, ITexturePtr texture) {
		mCurPass->mTextures[slot] = texture;
		mMaterial->AddDependency(texture->AsRes());
		return *this;
	}
	MaterialPtr Build() {
		mMaterial->CheckAndSetLoaded();
		return mMaterial;
	}
};

/********** MaterialAssetManager **********/
struct XmlAttributeInfo {
	std::vector<D3D11_INPUT_ELEMENT_DESC> Layout;
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
	std::vector<std::pair<int, int>> Samplers;
	void Add(const XmlSamplersInfo& other) {
		Samplers.insert(Samplers.end(), other.Samplers.begin(), other.Samplers.end());
	}
	size_t size() const { return Samplers.size(); }
	const std::pair<int, int>& operator[](size_t pos) const { return Samplers[pos]; }
};
struct XmlProgramInfo {
	D3D_PRIMITIVE_TOPOLOGY Topo;
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
	MaterialPtr LoadMaterial(RenderSystem& renderSys,
		const std::string& shaderName,
		const std::string& variantName) {
		XmlShaderInfo shaderInfo;
		if (!variantName.empty()) ParseShaderVariantXml(shaderName, variantName, shaderInfo);
		else ParseShaderXml(shaderName, shaderInfo);
		return CreateMaterial(renderSys, shaderName, shaderInfo);
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
				layoutJ = D3D11_INPUT_ELEMENT_DESC{
					attribute.LayoutStr[j].c_str(),
					element.second.get<UINT>("<xmlattr>.SemanticIndex", 0),
					(DXGI_FORMAT)element.second.get<UINT>("<xmlattr>.Format"),
					element.second.get<UINT>("<xmlattr>.InputSlot", 0),
					element.second.get<UINT>("<xmlattr>.ByteOffset", byteOffset),
					D3D11_INPUT_PER_VERTEX_DATA,
					0
				};
				byteOffset = layoutJ.AlignedByteOffset + D3dEnumConvert::GetWidth(layoutJ.Format);
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
					element.second.get<int>("<xmlattr>.Slot", 0),
					element.second.get<int>("<xmlattr>.Filter", D3D11_FILTER_MIN_MAG_MIP_LINEAR)
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
		vis.shaderInfo.Program.Topo = static_cast<D3D_PRIMITIVE_TOPOLOGY>(
			nodeProgram->get<int>("Topology", D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
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
				shaderInfo.Program.Topo = static_cast<D3D_PRIMITIVE_TOPOLOGY>(
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

	MaterialPtr CreateMaterial(RenderSystem& renderSys,
		const std::string& name,
		XmlShaderInfo& shaderInfo) {
		MaterialBuilder builder(false);
		builder.AddTechnique("d3d11");

		for (size_t i = 0; i < shaderInfo.SubShaders.size(); ++i) {
			auto& subShaderInfo = shaderInfo.SubShaders[i];
			for (size_t j = 0; j < subShaderInfo.Passes.size(); ++j) {
				auto& passInfo = subShaderInfo.Passes[j];

				builder.AddPass(passInfo.LightMode, passInfo.ShortName/*, i == 0*/);
				builder.SetTopology(shaderInfo.Program.Topo);

				IProgramPtr program = builder.SetProgram(renderSys.CreateProgram(
					MAKE_MAT_NAME(shaderInfo.Program.FxName),
					shaderInfo.Program.VsEntry.c_str(),
					passInfo.PSEntry.c_str()));
				builder.SetInputLayout(renderSys.CreateLayout(program,
					&shaderInfo.Program.Attr.Layout[0],
					shaderInfo.Program.Attr.Layout.size()));

				for (size_t k = 0; k < shaderInfo.Program.Samplers.size(); ++k) {
					auto& elem = shaderInfo.Program.Samplers[k];
					builder.AddSampler(renderSys.CreateSampler(
						D3D11_FILTER(elem.first),
						D3D11_COMPARISON_NEVER)
					);
				}
			}
		}

		for (size_t i = 0; i < shaderInfo.Program.Uniforms.size(); ++i) {
			auto& uniformI = shaderInfo.Program.Uniforms[i];
			builder.AddConstBufferToTech(renderSys.CreateConstBuffer(uniformI.Decl, &uniformI.Data[0]),
				uniformI.ShortName, uniformI.IsUnique);
		}

		builder.CloneTechnique(renderSys, "d3d9");
		builder.ClearSamplersToTech();
		builder.AddSamplerToTech(renderSys.CreateSampler(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_COMPARISON_NEVER));

		return builder.Build();
	}
};

/********** TMaterialFactory **********/
MaterialFactory::MaterialFactory(RenderSystem& renderSys)
	:mRenderSys(renderSys)
{
	mMatAssetMng = std::make_shared<MaterialAssetManager>();
}

MaterialPtr MaterialFactory::GetMaterial(const std::string& matName, bool readonly) {
	if (mMaterials.find(matName) == mMaterials.end())
		mMaterials.insert(std::make_pair(matName, CreateStdMaterial(matName)));

	MaterialPtr material = readonly ? mMaterials[matName] : mMaterials[matName]->Clone(mRenderSys);
	return material;
}

#if defined MATERIAL_FROM_XML
MaterialPtr MaterialFactory::CreateStdMaterial(const std::string& matName) {
	auto entry = mMatAssetMng->MatNameToAsset()(matName);
	return mMatAssetMng->LoadMaterial(mRenderSys, entry.ShaderName, entry.VariantName);
}
#else
void SetCommonField(MaterialBuilder& builder, RenderSystem* pRenderSys) {
	builder.SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	builder.AddConstBuffer(pRenderSys->CreateConstBuffer(MAKE_CBDESC(cbGlobalParam)));
	builder.AddSampler(pRenderSys->CreateSampler(D3D11_FILTER_MIN_MAG_MIP_LINEAR));
	builder.AddSampler(pRenderSys->CreateSampler(D3D11_FILTER_ANISOTROPIC));
	builder.AddSampler(pRenderSys->CreateSampler(D3D11_FILTER_MIN_MAG_MIP_POINT));
}

void SetCommonField2(MaterialBuilder& builder, RenderSystem* pRenderSys) {
	builder.SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	builder.AddConstBuffer(pRenderSys->CreateConstBuffer(MAKE_CBDESC(cbGlobalParam)));
}

void AddD3D9Technique(MaterialBuilder& builder, RenderSystem* pRenderSys) {
	builder.CloneTechnique(pRenderSys, "d3d9");
	builder.ClearSamplersToTech();
	builder.AddSamplerToTech(pRenderSys->CreateSampler(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_COMPARISON_ALWAYS), 8);
}

MaterialPtr MaterialFactory::CreateStdMaterial(const std::string& name) {
	TIME_PROFILE2(CreateStdMaterial, name);

	MaterialPtr material;
	MaterialBuilder builder;
	if (name == E_MAT_SPRITE || name == E_MAT_LAYERCOLOR || name == E_MAT_LABEL) {
		SetCommonField(builder, mRenderSys);
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 3 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 7 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		auto program = builder.SetProgram(mRenderSys->CreateProgram(MAKE_MAT_NAME(name)));
		builder.SetInputLayout(mRenderSys->CreateLayout(program, layout, ARRAYSIZE(layout)));

		AddD3D9Technique(builder, mRenderSys);
	}
	else if (name == E_MAT_MODEL) {
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 3 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, 6 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 9 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 11 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 15 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 2, DXGI_FORMAT_R32G32B32_FLOAT, 0, 19 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		//*//pass E_PASS_FORWARDBASE
		builder.SetPassName(E_PASS_FORWARDBASE, "ForwardBase");
		SetCommonField(builder, mRenderSys);
		auto program = builder.SetProgram(mRenderSys->CreateProgram(MAKE_MAT_NAME("Model")));
		builder.SetInputLayout(mRenderSys->CreateLayout(program, layout, ARRAYSIZE(layout)));

		//*//pass E_PASS_FORWARDADD
		builder.AddPass(E_PASS_FORWARDADD, "ForwardAdd");
		SetCommonField(builder, mRenderSys);
		program = builder.SetProgram(mRenderSys->CreateProgram(MAKE_MAT_NAME("Model"), nullptr, "PSAdd"));
		builder.SetInputLayout(mRenderSys->CreateLayout(program, layout, ARRAYSIZE(layout)));

		builder.AddConstBufferToTech(mRenderSys->CreateConstBuffer(MAKE_CBDESC(cbWeightedSkin)), MAKE_CBNAME(cbWeightedSkin), false);

		//*//pass E_PASS_SHADOWCASTER
		builder.AddPass(E_PASS_SHADOWCASTER, "ShadowCaster");
		SetCommonField(builder, mRenderSys);
		program = builder.SetProgram(mRenderSys->CreateProgram(MAKE_MAT_NAME("Model"), "VSShadowCaster", "PSShadowCaster"));
		builder.SetInputLayout(mRenderSys->CreateLayout(program, layout, ARRAYSIZE(layout)));
		builder.AddConstBuffer(mRenderSys->CreateConstBuffer(MAKE_CBDESC(cbWeightedSkin)), MAKE_CBNAME(cbWeightedSkin), false);

		AddD3D9Technique(builder, mRenderSys);
	}
	else if (name == E_MAT_MODEL_PBR) {
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 3 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, 6 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 9 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 11 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 15 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 2, DXGI_FORMAT_R32G32B32_FLOAT, 0, 19 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		//*//pass E_PASS_FORWARDBASE
		builder.SetPassName(E_PASS_FORWARDBASE, "ForwardBase");
		SetCommonField(builder, mRenderSys);
		auto program = builder.SetProgram(mRenderSys->CreateProgram(MAKE_MAT_NAME("ModelPbr")));
		builder.SetInputLayout(mRenderSys->CreateLayout(program, layout, ARRAYSIZE(layout)));

		//*//pass E_PASS_FORWARDADD
		builder.AddPass(E_PASS_FORWARDADD, "ForwardAdd");
		SetCommonField(builder, mRenderSys);
		program = builder.SetProgram(mRenderSys->CreateProgram(MAKE_MAT_NAME("ModelPbr"), nullptr, "PSAdd"));
		builder.SetInputLayout(mRenderSys->CreateLayout(program, layout, ARRAYSIZE(layout)));

		builder.AddConstBufferToTech(mRenderSys->CreateConstBuffer(MAKE_CBDESC(cbWeightedSkin)), MAKE_CBNAME(cbWeightedSkin), false);
		cbUnityMaterial cbUnityMat;
		//cbUnityMat._Color = XMFLOAT4(0,0,0,0);
		//cbUnityMat._SpecLightOff = TRUE;
		builder.AddConstBufferToTech(mRenderSys->CreateConstBuffer(MAKE_CBDESC(cbUnityMaterial), &cbUnityMat), MAKE_CBNAME(cbUnityMaterial));
		cbUnityGlobal cbUnityGlb;
		builder.AddConstBufferToTech(mRenderSys->CreateConstBuffer(MAKE_CBDESC(cbUnityGlobal), &cbUnityGlb), MAKE_CBNAME(cbUnityGlobal));

		//*//pass E_PASS_SHADOWCASTER
		builder.AddPass(E_PASS_SHADOWCASTER, "ShadowCaster");
		SetCommonField(builder, mRenderSys);
		program = builder.SetProgram(mRenderSys->CreateProgram(MAKE_MAT_NAME("ModelPbr"), "VSShadowCaster", "PSShadowCaster"));
		builder.SetInputLayout(mRenderSys->CreateLayout(program, layout, ARRAYSIZE(layout)));
		builder.AddConstBuffer(mRenderSys->CreateConstBuffer(MAKE_CBDESC(cbWeightedSkin)), MAKE_CBNAME(cbWeightedSkin), false);

		AddD3D9Technique(builder, mRenderSys);
	}
	else if (name == E_MAT_MODEL_SHADOW) {
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 3 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, 6 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 9 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 11 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 15 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 2, DXGI_FORMAT_R32G32B32_FLOAT, 0, 19 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		//pass E_PASS_FORWARDBASE
		SetCommonField(builder, mRenderSys);
		auto program = builder.SetProgram(mRenderSys->CreateProgram(MAKE_MAT_NAME("ShadowMap")));
		builder.SetInputLayout(mRenderSys->CreateLayout(program, layout, ARRAYSIZE(layout)));

		//pass E_PASS_SHADOWCASTER
		builder.AddPass(E_PASS_SHADOWCASTER, "ShadowCaster");
		SetCommonField(builder, mRenderSys);
		program = builder.SetProgram(mRenderSys->CreateProgram(MAKE_MAT_NAME("ShadowDepth")));
		builder.SetInputLayout(mRenderSys->CreateLayout(program, layout, ARRAYSIZE(layout)));

		builder.AddConstBufferToTech(mRenderSys->CreateConstBuffer(MAKE_CBDESC(cbWeightedSkin)), MAKE_CBNAME(cbWeightedSkin), false);

		AddD3D9Technique(builder, mRenderSys);
	}
	else if (name == E_MAT_SKYBOX) {
		SetCommonField2(builder, mRenderSys);
		D3D11_INPUT_ELEMENT_DESC layout[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		auto program = builder.SetProgram(mRenderSys->CreateProgram(MAKE_MAT_NAME("Skybox")));
		builder.SetInputLayout(mRenderSys->CreateLayout(program, layout, ARRAYSIZE(layout)));
		builder.SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		builder.AddSampler(mRenderSys->CreateSampler(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_COMPARISON_ALWAYS));

		AddD3D9Technique(builder, mRenderSys);
	}
	else if (name == E_MAT_POSTPROC_BLOOM) {
#define NUM_TONEMAP_TEXTURES  10
#define NUM_BLOOM_TEXTURES    2
		std::vector<IRenderTexturePtr> TexToneMaps(NUM_TONEMAP_TEXTURES);
		int nSampleLen = 1;
		for (size_t i = 0; i < NUM_TONEMAP_TEXTURES; i++) {
			TexToneMaps[i] = mRenderSys->CreateRenderTexture(nSampleLen, nSampleLen, DXGI_FORMAT_R16G16B16A16_UNORM);
			SET_DEBUG_NAME(TexToneMaps[i]->mDepthStencilView, "TexToneMaps" + i);
			nSampleLen *= 2;
		}
		IRenderTexturePtr TexBrightPass = mRenderSys->CreateRenderTexture(mRenderSys->GetWinSize().x / 8, mRenderSys->GetWinSize().y / 8, DXGI_FORMAT_B8G8R8A8_UNORM);
		SET_DEBUG_NAME(TexBrightPass->mDepthStencilView, "TexBrightPass");
		std::vector<IRenderTexturePtr> TexBlooms(NUM_BLOOM_TEXTURES);
		for (size_t i = 0; i < NUM_BLOOM_TEXTURES; i++) {
			TexBlooms[i] = mRenderSys->CreateRenderTexture(mRenderSys->GetWinSize().x / 8, mRenderSys->GetWinSize().y / 8, DXGI_FORMAT_R16G16B16A16_UNORM);
			SET_DEBUG_NAME(TexBlooms[i]->mDepthStencilView, "TexBlooms" + i);
		}

		//pass DownScale2x2
		builder.SetPassName(E_PASS_POSTPROCESS, "DownScale2x2");
		SetCommonField(builder, mRenderSys);
		D3D11_INPUT_ELEMENT_DESC layout[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		auto program = builder.SetProgram(mRenderSys->CreateProgram(MAKE_MAT_NAME("Bloom"), "VS", "DownScale2x2"));
		builder.SetInputLayout(mRenderSys->CreateLayout(program, layout, ARRAYSIZE(layout)));
		builder.SetRenderTarget(TexToneMaps[NUM_TONEMAP_TEXTURES - 1]);
		builder.AddConstBuffer(mRenderSys->CreateConstBuffer(MAKE_CBDESC(cbBloom)), MAKE_CBNAME(cbBloom));
		builder.mCurPass->OnBind = [](Pass* pass, IRenderSystem* pRenderSys, TextureBySlot& textures) {
			auto mainTex = textures[0];
			cbBloom bloom = cbBloom::CreateDownScale2x2Offsets(mainTex->GetWidth(), mainTex->GetHeight());
			pass->UpdateConstBufferByName(pRenderSys, MAKE_CBNAME(cbBloom), Data::Make(bloom));
		};

		//pass DownScale3x3
		builder.AddPass(E_PASS_POSTPROCESS, "DownScale3x3");
		SetCommonField(builder, mRenderSys);
		program = builder.SetProgram(mRenderSys->CreateProgram(MAKE_MAT_NAME("Bloom"), "VS", "DownScale3x3"));
		builder.SetInputLayout(mRenderSys->CreateLayout(program, layout, ARRAYSIZE(layout)));
		builder.SetRenderTarget(TexToneMaps[0]);
		for (int i = 1; i < NUM_TONEMAP_TEXTURES - 1; ++i) {
			builder.AddIterTarget(TexToneMaps[i]);
		}
		builder.SetTexture(0, TexToneMaps[NUM_TONEMAP_TEXTURES - 1]->GetColorTexture());
		builder.AddConstBuffer(mRenderSys->CreateConstBuffer(MAKE_CBDESC(cbBloom)), MAKE_CBNAME(cbBloom));
		builder.mCurPass->OnBind = [](Pass* pass, IRenderSystem* pRenderSys, TextureBySlot& textures) {
			auto mainTex = textures[0];
			cbBloom bloom = cbBloom::CreateDownScale3x3Offsets(mainTex->GetWidth(), mainTex->GetHeight());
			pass->UpdateConstBufferByName(pRenderSys, MAKE_CBNAME(cbBloom), Data::Make(bloom));
		};

		//pass DownScale3x3_BrightPass
		builder.AddPass(E_PASS_POSTPROCESS, "DownScale3x3_BrightPass");
		SetCommonField(builder, mRenderSys);
		program = builder.SetProgram(mRenderSys->CreateProgram(MAKE_MAT_NAME("Bloom"), "VS", "DownScale3x3_BrightPass"));
		builder.SetInputLayout(mRenderSys->CreateLayout(program, layout, ARRAYSIZE(layout)));
		builder.SetRenderTarget(TexBrightPass);
		builder.SetTexture(1, TexToneMaps[0]->GetColorTexture());
		builder.AddConstBuffer(mRenderSys->CreateConstBuffer(MAKE_CBDESC(cbBloom)), MAKE_CBNAME(cbBloom));
		builder.mCurPass->OnBind = [](Pass* pass, IRenderSystem* pRenderSys, TextureBySlot& textures) {
			auto mainTex = textures[0];
			cbBloom bloom = cbBloom::CreateDownScale3x3Offsets(mainTex->GetWidth(), mainTex->GetHeight());
			pass->UpdateConstBufferByName(pRenderSys, MAKE_CBNAME(cbBloom), Data::Make(bloom));
		};

		//pass Bloom
		builder.AddPass(E_PASS_POSTPROCESS, "Bloom");
		SetCommonField(builder, mRenderSys);
		program = builder.SetProgram(mRenderSys->CreateProgram(MAKE_MAT_NAME("Bloom"), "VS", "BloomPS"));
		builder.SetInputLayout(mRenderSys->CreateLayout(program, layout, ARRAYSIZE(layout)));
		builder.SetRenderTarget(TexBlooms[0]);
		for (int i = 1; i < NUM_BLOOM_TEXTURES; ++i) {
			builder.AddIterTarget(TexBlooms[i]);
		}
		builder.SetTexture(1, TexBrightPass->GetColorTexture());
		builder.AddConstBuffer(mRenderSys->CreateConstBuffer(MAKE_CBDESC(cbBloom)), MAKE_CBNAME(cbBloom));
		builder.mCurPass->OnBind = [](Pass* pass, IRenderSystem* pRenderSys, TextureBySlot& textures) {
			auto mainTex = textures[0];
			cbBloom bloom = cbBloom::CreateBloomOffsets(mainTex->GetWidth(), 3.0f, 1.25f);
			pass->UpdateConstBufferByName(pRenderSys, MAKE_CBNAME(cbBloom), Data::Make(bloom));
		};

		//pass FinalPass
		builder.AddPass(E_PASS_POSTPROCESS, "FinalPass");
		SetCommonField(builder, mRenderSys);
		program = builder.SetProgram(mRenderSys->CreateProgram(MAKE_MAT_NAME("Bloom"), "VS", "FinalPass"));
		builder.SetInputLayout(mRenderSys->CreateLayout(program, layout, ARRAYSIZE(layout)));
		builder.SetTexture(1, TexToneMaps[0]->GetColorTexture());
		builder.SetTexture(2, TexBlooms[0]->GetColorTexture());

		AddD3D9Technique(builder, mRenderSys);
	}

	material = builder.Build();
	return material;
}
#endif
}