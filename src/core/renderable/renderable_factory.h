#pragma once
#include "core/mir_export.h"
#include "core/base/cppcoro.h"
#include "core/base/launch.h"
#include "core/base/material_load_param.h"
#include "core/renderable/renderable.h"

namespace mir {

class MIR_CORE_API RenderableFactory 
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	typedef const std::string& string_cref;
	RenderableFactory(ResourceManager& resMng, Launch launchMode);
	CoTask<rend::SkyBoxPtr> CreateSkybox(std::string imagePath, MaterialLoadParam loadParam = "");
	CoTask<rend::SpritePtr> CreateColorLayer(MaterialLoadParam loadParam = "");
	CoTask<rend::SpritePtr> CreateSprite(std::string imagePath = "", MaterialLoadParam loadParam = "");
	CoTask<rend::MeshPtr> CreateMesh(int vertCount = 1024, int indexCount = 1024, MaterialLoadParam loadParam = "");
	CoTask<rend::CubePtr> CreateCube(Eigen::Vector3f center, Eigen::Vector3f halfsize, unsigned bgra = -1, MaterialLoadParam loadParam = "");
	CoTask<rend::AssimpModelPtr> CreateAssimpModel(MaterialLoadParam loadParam = "");
	CoTask<rend::LabelPtr> CreateLabel(std::string fontPath, int fontSize);
	CoTask<rend::PostProcessPtr> CreatePostProcessEffect(std::string effectName, scene::Camera& camera);

	template<typename RendClass, typename... T> std::shared_ptr<RendClass> CreateRend(T &&...args) {
		return CreateRendFunctor<RendClass>()(*this, std::forward<T>(args)...);
	}
private:
	template<typename RendClass> struct CreateRendFunctor {};
	template<> struct CreateRendFunctor<rend::SkyBox> {
		TemplateArgs rend::SkyBoxPtr operator()(RenderableFactory& __this, T &&...args) const { return __this.CreateSkybox(std::forward<T>(args)...); }
	};
	template<> struct CreateRendFunctor<rend::Sprite> {
		TemplateArgs rend::SpritePtr operator()(RenderableFactory& __this, T &&...args) const { return __this.CreateSprite(std::forward<T>(args)...); }
	};
	template<> struct CreateRendFunctor<rend::Mesh> {
		TemplateArgs rend::MeshPtr operator()(RenderableFactory& __this, T &&...args) const { return __this.CreateMesh(std::forward<T>(args)...); }
	};
	template<> struct CreateRendFunctor<rend::Cube> {
		TemplateArgs rend::CubePtr operator()(RenderableFactory& __this, T &&...args) const { return __this.CreateCube(std::forward<T>(args)...); }
	};
	template<> struct CreateRendFunctor<rend::AssimpModel> {
		TemplateArgs rend::AssimpModelPtr operator()(RenderableFactory& __this, T &&...args) const { return __this.CreateAssimpModel(std::forward<T>(args)...); }
	};
	template<> struct CreateRendFunctor<rend::Label> {
		TemplateArgs rend::LabelPtr operator()(RenderableFactory& __this, T &&...args) const { return __this.CreateLabel(std::forward<T>(args)...); }
	};
	template<> struct CreateRendFunctor<rend::PostProcess> {
		TemplateArgs rend::PostProcessPtr operator()(RenderableFactory& __this, T &&...args) const { return __this.CreatePostProcessEffect(std::forward<T>(args)...); }
	};
private:
	Launch mLaunchMode;
	ResourceManager& mResourceMng;
	FontCachePtr mFontCache;
};

}