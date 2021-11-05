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
#include "core/rendersys/const_buffer_decl.h"
#include "core/renderable/post_process.h"
#include "core/base/utility.h"

namespace boost_filesystem = boost::filesystem;
namespace boost_property_tree = boost::property_tree;

namespace mir {

	/********** XmlParserMaterialLoader **********/
	struct XmlAttributeInfo {
		std::vector<D3D11_INPUT_ELEMENT_DESC> Layout;
		std::vector<std::string> LayoutStr;
	};
	struct XmlUniformInfo {
		TConstBufferDecl Decl;
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
		/*XmlPassInfo(XmlPassInfo&& other) {
			LightMode = std::move(other.LightMode);
			Name	  = std::move(other.Name);
			ShortName = std::move(other.ShortName);
			PSEntry   = std::move(other.PSEntry);
		}
		XmlPassInfo& operator=(XmlPassInfo&& other) {
			LightMode = std::move(other.LightMode);
			Name	  = std::move(other.Name);
			ShortName = std::move(other.ShortName);
			PSEntry   = std::move(other.PSEntry);
			return *this;
		}*/
	};
	struct XmlSubShaderInfo {
		std::vector<XmlPassInfo> Passes;
	public:
		/*XmlSubShaderInfo(XmlSubShaderInfo&& other) {
			Passes = std::move(other.Passes);
		}
		XmlSubShaderInfo& operator=(XmlSubShaderInfo&& other) {
			Passes = std::move(other.Passes);
			return *this;
		}*/
		void AddPass(XmlPassInfo&& pass) {
			Passes.push_back(std::move(pass));
		}
	};
	struct XmlShaderInfo {
		XmlProgramInfo Program;
		std::vector<XmlSubShaderInfo> SubShaders;
	public:
		void AddSubShader(XmlSubShaderInfo&& subShader) {
			SubShaders.push_back(std::move(subShader));
		}
	};

	class ParseXmlAndCreateMaterial
	{
		std::map<std::string, XmlShaderInfo> mIncludeByName, mShaderByName, mShaderVariantByName;
		std::map<std::string, XmlAttributeInfo> mAttrByName;
		std::map<std::string, XmlUniformInfo> mUniformByName;
		std::map<std::string, XmlSamplersInfo> mSamplersByName;
	public:
		TMaterialPtr Execute(TRenderSystem* renderSys,
			const std::string& shaderName,
			const std::string& variantName) {
			XmlShaderInfo shaderInfo;
			if (!variantName.empty()) ParseShaderVariantXml(shaderName, variantName, shaderInfo);
			else ParseShaderXml(shaderName, shaderInfo);
			return CreateMaterial(renderSys, shaderName, shaderInfo);
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
			PropertyTreePath(const boost_property_tree::ptree& node) :Node(node),Index(0) {
				Path = node.get<std::string>("<xmlattr>.Name", "0");
			}
			PropertyTreePath(const PropertyTreePath& parent, const boost_property_tree::ptree& node, int index) :Node(node),Index(index) {
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
		
		static EConstBufferElementType ConvertStringToConstBufferElementType(const std::string& str,
			int count,
			int& size) {
			EConstBufferElementType result = E_CONSTBUF_ELEM_MAX;
			if (str == "int") result = E_CONSTBUF_ELEM_INT, size = 4;
			else if (str == "float") result = E_CONSTBUF_ELEM_FLOAT, size = 4;
			else if (str == "float4") result = E_CONSTBUF_ELEM_FLOAT4, size = 16;
			else if (str == "matrix") result = E_CONSTBUF_ELEM_MATRIX, size = 64;
			else if (str == "struct") result = E_CONSTBUF_ELEM_STRUCT;
			size *= std::max<int>(1, count);
			return result;
		}
		void VisitAttributes(const PropertyTreePath& nodeProgram, Visitor& vis) {
			for (auto& it : nodeProgram->get_child("PROGRAM.UseAttribute")) {
				std::string refName = it.second.data();
				auto find_iter = mAttrByName.find(refName);
				if (find_iter != mAttrByName.end()) {
					vis.shaderInfo.Program.AddAttribute(find_iter->second);
				}
			}

			int index = 0;
			for (auto& it : nodeProgram->get_child("PROGRAM.Attribute")) {
				XmlAttributeInfo attribute;
				int elementCount = it.second.count("Attribute.Element");
				attribute.Layout.resize(elementCount);
				attribute.LayoutStr.resize(elementCount);

				int byteOffset = 0, j = 0;
				for (auto& element : it.second.get_child("Attribute.Element")) {
					attribute.LayoutStr[j] = element.second.get<std::string>("<xmlattr>.SemanticName");
					attribute.Layout[j] = D3D11_INPUT_ELEMENT_DESC{
						attribute.LayoutStr[j].c_str(),
						element.second.get<UINT>("<xmlattr>.SemanticIndex"),
						(DXGI_FORMAT)element.second.get<UINT>("<xmlattr>.Format"),
						element.second.get<UINT>("<xmlattr>.InputSlot"),
						element.second.get<UINT>("<xmlattr>.ByteOffset"),
						D3D11_INPUT_PER_VERTEX_DATA,
						0
					};
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
			for (auto& it : nodeProgram->get_child("PROGRAM.UseUniform")) {
				std::string refName = it.second.data();
				auto find_iter = mUniformByName.find(refName);
				if (find_iter != mUniformByName.end()) {
					vis.shaderInfo.Program.AddUniform(find_iter->second);
				}
			}

			int index = 0;
			for (auto& it : nodeProgram->get_child("PROGRAM.Uniform")) {
				XmlUniformInfo uniform;
				uniform.IsUnique = it.second.get<int>("<xmlattr>.IsUnique", true);

				TConstBufferDeclBuilder builder(uniform.Decl);

				int byteOffset = 0;
				for (auto& element : it.second.get_child("Uniform.Element")) {
					int size = element.second.get<int>("<xmlattr>.Size", 0); BOOST_ASSERT(size % 4 == 0);
					int count = element.second.get<int>("<xmlattr>.Count", 0);
					int offset = element.second.get<int>("<xmlattr>.Offset", byteOffset);
					EConstBufferElementType uniformElementType = ConvertStringToConstBufferElementType(element.second.get<std::string>("<xmlattr>.Type"), count, size);
					BOOST_ASSERT(uniformElementType != E_CONSTBUF_ELEM_MAX);
					std::string name = element.second.get<std::string>("<xmlattr>.Name");

					builder.Add(TConstBufferDeclElement(name.c_str(), uniformElementType, size, count, offset));
					byteOffset = offset + size;

					int dataSize = uniform.Data.size();
					uniform.Data.resize(dataSize + size / 4);
					void* pData = &uniform.Data[dataSize];

					if (element.second.find("<xmlattr>.Default") != element.second.not_found()) {
						std::string strDefault = element.second.get<std::string>("<xmlattr>.Default");
						switch (uniformElementType) {
						case E_CONSTBUF_ELEM_INT: {
							*static_cast<int*>(pData) = boost::lexical_cast<int>(strDefault);
						}break;
						case E_CONSTBUF_ELEM_FLOAT: {
							*static_cast<float*>(pData) = boost::lexical_cast<float>(strDefault);
						}break;
						case E_CONSTBUF_ELEM_FLOAT4:
						case E_CONSTBUF_ELEM_MATRIX: {
							std::vector<boost::iterator_range<std::string::iterator>> lst;
							boost::split(lst, strDefault, boost::is_any_of(","));
							int count = (uniformElementType == E_CONSTBUF_ELEM_FLOAT4) ? 4 : 16;
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
			for (auto& it : nodeProgram->get_child("PROGRAM.Sampler")) {
				XmlSamplersInfo sampler;
				sampler.Samplers.resize(it.second.count("Sampler.Element"));
				int j = 0;
				for (auto& element : it.second.get_child("Sampler.Element")) {
					sampler.Samplers[j].first = element.second.get<int>("<xmlattr>.Slot");
					sampler.Samplers[j].second = element.second.get<int>("<xmlattr>.Filter", D3D11_FILTER_MIN_MAG_MIP_LINEAR);
					++j;
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
			vis.shaderInfo.Program.FxName = nodeProgram->get<std::string>("PROGRAM.FileName");
			vis.shaderInfo.Program.VsEntry = nodeProgram->get<std::string>("PROGRAM.VertexEntry");
			vis.shaderInfo.Program.Topo = static_cast<D3D_PRIMITIVE_TOPOLOGY>(nodeProgram->get<int>("PROGRAM.Topology"), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			VisitAttributes(nodeProgram, vis);
			VisitUniforms(nodeProgram, vis);
			VisitSamplers(nodeProgram, vis);
		}
		
		void VisitSubShader(const PropertyTreePath& nodeTechnique, Visitor& vis) {
			int index = 0;
			for (auto& it : nodeTechnique->get_child("SubShader.Pass")) {
				auto& node_pass = it.second;
				XmlPassInfo pass;

				pass.LightMode = E_PASS_FORWARDBASE;
				auto find_tags = node_pass.find("SubShader.Tags");
				if (find_tags != node_pass.not_found()) {
					auto& node_tag = find_tags->second;
					pass.LightMode = node_tag.get<std::string>("Tags.LightMode", pass.LightMode);
				}

				pass.ShortName = node_pass.get<std::string>("Pass.ShortName", node_pass.get<std::string>("Pass.Name"));
				pass.Name = node_pass.get<std::string>("Pass.Name", boost::lexical_cast<std::string>(index));

				pass.PSEntry = "PS";
				auto find_program = node_pass.find("SubShader.PROGRAM");
				if (find_program != node_pass.not_found()) {
					auto& node_program = find_program->second;
					pass.PSEntry = node_program.get<std::string>("PROGRAM.PixelEntry", pass.PSEntry);
				}
				++index;
			}
		}
		void VisitShader(const PropertyTreePath& nodeShader, Visitor& vis) {
			for (auto& it : nodeShader->get_child("Shader.Include")) {
				VisitInclude(it.second.data());
			}

			int index = 0;
			for (auto& it : nodeShader->get_child("Shader.PROGRAM")) {
				VisitProgram(PropertyTreePath(nodeShader, it.second, index++), vis);
			}

			if (!vis.JustInclude) {
				index = 0;
				for (auto& it : nodeShader->get_child("Shader.SubShader")) {
					VisitSubShader(PropertyTreePath(nodeShader, it.second, index++), vis);
				}
			}
		}
		void VisitShaderVariant(const PropertyTreePath& nodeVariant,
			const std::string& variantName,
			XmlShaderInfo& shaderInfo) {
			for (auto& it : nodeVariant->get_child("ShaderVariant.UseShader")) {
				ParseShaderXml(it.second.data(), shaderInfo);
			}

			for (auto& it : nodeVariant->get_child("ShaderVariant.Variant")) {
				std::string name = it.second.get<std::string>("<xmlattr>.Name");
				if (name == variantName) {
					shaderInfo.Program.Topo = static_cast<D3D_PRIMITIVE_TOPOLOGY>(it.second.get<int>("Variant.Topology"));
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
					VisitShader(pt, visitor);
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
					VisitShaderVariant(pt, variantName, shaderInfo);
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

		TMaterialPtr CreateMaterial(TRenderSystem* renderSys,
			const std::string& name,
			XmlShaderInfo& shaderInfo) {
			TMaterialBuilder builder(false);
			builder.AddTechnique("d3d11");

			for (size_t i = 0; i < shaderInfo.SubShaders.size(); ++i) {
				auto& subShaderInfo = shaderInfo.SubShaders[i];
				for (size_t j = 0; j < subShaderInfo.Passes.size(); ++j) {
					auto& passInfo = subShaderInfo.Passes[j];

					builder.AddPass(passInfo.LightMode, passInfo.ShortName/*, i == 0*/);
					builder.SetTopology(shaderInfo.Program.Topo);

					IProgramPtr program = builder.SetProgram(renderSys->CreateProgram(MAKE_MAT_NAME(shaderInfo.Program.FxName), shaderInfo.Program.VsEntry.c_str(), passInfo.PSEntry.c_str()));
					builder.SetInputLayout(renderSys->CreateLayout(program, &shaderInfo.Program.Attr.Layout[0], shaderInfo.Program.Attr.Layout.size()));

					for (size_t k = 0; k < shaderInfo.Program.Samplers.size(); ++k) {
						auto& elem = shaderInfo.Program.Samplers[k];
						builder.AddSampler(renderSys->CreateSampler(D3D11_FILTER(elem.first), D3D11_COMPARISON_NEVER));
					}
				}
			}

			for (size_t i = 0; i < shaderInfo.Program.Uniforms.size(); ++i) {
				auto& uniformI = shaderInfo.Program.Uniforms[i];
				builder.AddConstBufferToTech(renderSys->CreateConstBuffer(uniformI.Decl, &uniformI.Data[0]), uniformI.ShortName, uniformI.IsUnique);
			}

			builder.CloneTechnique(renderSys, "d3d9");
			builder.ClearSamplersToTech();
			builder.AddSamplerToTech(renderSys->CreateSampler(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_COMPARISON_NEVER));

			return builder.Build();
		}
	};

	/********** TMaterialBuilder **********/
	TMaterialBuilder::TMaterialBuilder(bool addTechPass)
	{
		mMaterial = std::make_shared<TMaterial>();
		if (addTechPass) {
			AddTechnique();
			AddPass(E_PASS_FORWARDBASE, "");
		}
	}

	TMaterialBuilder::TMaterialBuilder(TMaterialPtr material)
	{
		mMaterial = material;
		mCurTech = material->CurTech();
		mCurPass = mCurTech->mPasses.empty() ? nullptr : mCurTech->mPasses[mCurTech->mPasses.size()-1];
	}

	TMaterialBuilder& TMaterialBuilder::AddTechnique(const std::string& name)
	{
		mCurTech = std::make_shared<TTechnique>();
		mCurTech->mName = name;
		mMaterial->AddTechnique(mCurTech);
		return *this;
	}

	TMaterialBuilder& TMaterialBuilder::CloneTechnique(IRenderSystem* pRenderSys, const std::string& name)
	{
		mCurTech = mCurTech->Clone(pRenderSys);
		mCurTech->mName = name;
		mMaterial->AddTechnique(mCurTech);
		return *this;
	}

	TMaterialBuilder& TMaterialBuilder::AddPass(const std::string& lightMode, const std::string& passName)
	{
		mCurPass = std::make_shared<TPass>(lightMode, passName);
		mCurTech->AddPass(mCurPass);
		return *this;
	}

	TMaterialBuilder& TMaterialBuilder::SetPassName(const std::string& lightMode, const std::string& passName)
	{
		mCurPass->mLightMode = lightMode;
		mCurPass->mName = passName;
		return *this;
	}

	TMaterialBuilder& TMaterialBuilder::SetInputLayout(IInputLayoutPtr inputLayout)
	{
		mCurPass->mInputLayout = inputLayout;
		mMaterial->AddDependency(inputLayout->AsRes());
		return *this;
	}

	TMaterialBuilder& TMaterialBuilder::SetTopology(D3D11_PRIMITIVE_TOPOLOGY topology)
	{
		mCurPass->mTopoLogy = topology;
		return *this;
	}

	IProgramPtr TMaterialBuilder::SetProgram(IProgramPtr program)
	{
		mCurPass->mProgram = program;
		mMaterial->AddDependency(program->AsRes());
		return program;
	}

	TMaterialBuilder& TMaterialBuilder::AddSampler(ISamplerStatePtr sampler, int count)
	{
		while (count-- > 0)
			mCurPass->AddSampler(sampler);
		return *this;
	}

	TMaterialBuilder& TMaterialBuilder::AddSamplerToTech(ISamplerStatePtr sampler, int count /*= 1*/)
	{
		while (count-- > 0)
			mCurTech->AddSampler(sampler);
		return *this;
	}

	TMaterialBuilder& TMaterialBuilder::ClearSamplersToTech()
	{
		mCurTech->ClearSamplers();
		return *this;
	}

	TMaterialBuilder& TMaterialBuilder::AddConstBuffer(IContantBufferPtr buffer, const std::string& name, bool isUnique)
	{
		mCurPass->AddConstBuffer(TContantBufferInfo(buffer, name, isUnique));
		return *this;
	}

	TMaterialBuilder& TMaterialBuilder::AddConstBufferToTech(IContantBufferPtr buffer, const std::string& name, bool isUnique)
	{
		mCurTech->AddConstBuffer(TContantBufferInfo(buffer, name, isUnique));
		return *this;
	}

	TMaterialBuilder& TMaterialBuilder::SetRenderTarget(IRenderTexturePtr target)
	{
		mCurPass->mRenderTarget = target;
		return *this;
	}

	TMaterialBuilder& TMaterialBuilder::AddIterTarget(IRenderTexturePtr target)
	{
		mCurPass->AddIterTarget(target);
		return *this;
	}

	TMaterialBuilder& TMaterialBuilder::SetTexture(size_t slot, ITexturePtr texture)
	{
		mCurPass->mTextures[slot] = texture;
		mMaterial->AddDependency(texture->AsRes());
		return *this;
	}

	TMaterialPtr TMaterialBuilder::Build()
	{
		mMaterial->CheckAndSetLoaded();
		return mMaterial;
	}

	/********** TMaterialFactory **********/
	TMaterialFactory::TMaterialFactory(TRenderSystem* pRenderSys)
	{
		mRenderSys = pRenderSys;
		mParseXmlCreateMaterial = std::make_shared<ParseXmlAndCreateMaterial>();
	}

	TMaterialPtr TMaterialFactory::GetMaterial(std::string name, std::function<void(TMaterialPtr material)> callback /*= nullptr*/, std::string identify /* = ""*/, bool readonly /*= false*/)
	{
		TMaterialPtr material;

		if (mMaterials.find(name) == mMaterials.end()) {
			mMaterials.insert(std::make_pair(name, CreateStdMaterial(name)));
		}

		if (!identify.empty()) {
			auto key = name + ":" + identify;
			if (mMaterials.find(key) == mMaterials.end()) {
				material = mMaterials[name]->Clone(mRenderSys);
				if (callback) callback(material);
				mMaterials.insert(std::make_pair(key, material));
			}
			else {
				material = readonly ? mMaterials[key] : mMaterials[key]->Clone(mRenderSys);
			}
		}
		else if (callback != nullptr) {
			material = mMaterials[name]->Clone(mRenderSys);
			callback(material);
		}
		else {
			material = readonly ? mMaterials[name] : mMaterials[name]->Clone(mRenderSys);
		}
		return material;
	}

#if defined MATERIAL_FROM_XML
	TMaterialPtr TMaterialFactory::CreateStdMaterial(std::string name) {
		return mParseXmlCreateMaterial->Execute(mRenderSys, name, "");
	}
#else
	void SetCommonField(TMaterialBuilder& builder, TRenderSystem* pRenderSys)
	{
		builder.SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		builder.AddConstBuffer(pRenderSys->CreateConstBuffer(MAKE_CBDESC(cbGlobalParam)));
		builder.AddSampler(pRenderSys->CreateSampler(D3D11_FILTER_MIN_MAG_MIP_LINEAR));
		builder.AddSampler(pRenderSys->CreateSampler(D3D11_FILTER_ANISOTROPIC));
		builder.AddSampler(pRenderSys->CreateSampler(D3D11_FILTER_MIN_MAG_MIP_POINT));
	}

	void SetCommonField2(TMaterialBuilder& builder, TRenderSystem* pRenderSys)
	{
		builder.SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		builder.AddConstBuffer(pRenderSys->CreateConstBuffer(MAKE_CBDESC(cbGlobalParam)));
	}

	void AddD3D9Technique(TMaterialBuilder& builder, TRenderSystem* pRenderSys)
	{
		builder.CloneTechnique(pRenderSys, "d3d9");
		builder.ClearSamplersToTech();
		builder.AddSamplerToTech(pRenderSys->CreateSampler(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_COMPARISON_ALWAYS), 8);
	}

	TMaterialPtr TMaterialFactory::CreateStdMaterial(std::string name)
	{
		TIME_PROFILE2(CreateStdMaterial, name);

		TMaterialPtr material;
		TMaterialBuilder builder;
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
				SET_DEBUG_NAME(TexToneMaps[i]->mDepthStencilView, "TexToneMaps"+i);
				nSampleLen *= 2;
			}
			IRenderTexturePtr TexBrightPass = mRenderSys->CreateRenderTexture(mRenderSys->GetWinSize().x / 8, mRenderSys->GetWinSize().y / 8, DXGI_FORMAT_B8G8R8A8_UNORM);
			SET_DEBUG_NAME(TexBrightPass->mDepthStencilView, "TexBrightPass");
			std::vector<IRenderTexturePtr> TexBlooms(NUM_BLOOM_TEXTURES);
			for (size_t i = 0; i < NUM_BLOOM_TEXTURES; i++) {
				TexBlooms[i] = mRenderSys->CreateRenderTexture(mRenderSys->GetWinSize().x / 8, mRenderSys->GetWinSize().y / 8, DXGI_FORMAT_R16G16B16A16_UNORM);
				SET_DEBUG_NAME(TexBlooms[i]->mDepthStencilView, "TexBlooms"+i);
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
			builder.SetRenderTarget(TexToneMaps[NUM_TONEMAP_TEXTURES-1]);
			builder.AddConstBuffer(mRenderSys->CreateConstBuffer(MAKE_CBDESC(cbBloom)), MAKE_CBNAME(cbBloom));
			builder.mCurPass->OnBind = [](TPass* pass, IRenderSystem* pRenderSys, TTextureBySlot& textures) {
				auto mainTex = textures[0];
				cbBloom bloom = cbBloom::CreateDownScale2x2Offsets(mainTex->GetWidth(), mainTex->GetHeight());
				pass->UpdateConstBufferByName(pRenderSys, MAKE_CBNAME(cbBloom), make_data(bloom));
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
			builder.mCurPass->OnBind = [](TPass* pass, IRenderSystem* pRenderSys, TTextureBySlot& textures) {
				auto mainTex = textures[0];
				cbBloom bloom = cbBloom::CreateDownScale3x3Offsets(mainTex->GetWidth(), mainTex->GetHeight());
				pass->UpdateConstBufferByName(pRenderSys, MAKE_CBNAME(cbBloom), make_data(bloom));
			};

			//pass DownScale3x3_BrightPass
			builder.AddPass(E_PASS_POSTPROCESS, "DownScale3x3_BrightPass");
			SetCommonField(builder, mRenderSys);
			program = builder.SetProgram(mRenderSys->CreateProgram(MAKE_MAT_NAME("Bloom"), "VS", "DownScale3x3_BrightPass"));
			builder.SetInputLayout(mRenderSys->CreateLayout(program, layout, ARRAYSIZE(layout)));
			builder.SetRenderTarget(TexBrightPass);
			builder.SetTexture(1, TexToneMaps[0]->GetColorTexture());
			builder.AddConstBuffer(mRenderSys->CreateConstBuffer(MAKE_CBDESC(cbBloom)), MAKE_CBNAME(cbBloom));
			builder.mCurPass->OnBind = [](TPass* pass, IRenderSystem* pRenderSys, TTextureBySlot& textures) {
				auto mainTex = textures[0];
				cbBloom bloom = cbBloom::CreateDownScale3x3Offsets(mainTex->GetWidth(), mainTex->GetHeight());
				pass->UpdateConstBufferByName(pRenderSys, MAKE_CBNAME(cbBloom), make_data(bloom));
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
			builder.mCurPass->OnBind = [](TPass* pass, IRenderSystem* pRenderSys, TTextureBySlot& textures) {
				auto mainTex = textures[0];
				cbBloom bloom = cbBloom::CreateBloomOffsets(mainTex->GetWidth(), 3.0f, 1.25f);
				pass->UpdateConstBufferByName(pRenderSys, MAKE_CBNAME(cbBloom), make_data(bloom));
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