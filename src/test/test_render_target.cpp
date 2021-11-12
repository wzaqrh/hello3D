#include "test/test_case.h"
#include "test/app.h"
#include "core/rendersys/material_factory.h"
#include "core/rendersys/scene_manager.h"
#include "core/rendersys/interface_type.h"
#include "core/renderable/assimp_model.h"
#include "core/renderable/sprite.h"
#include "core/base/transform.h"
#include "core/base/utility.h"

using namespace mir;

class TestRT : public App
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
	virtual void OnInitLight() override;
private:
	AssimpModel* mModel = nullptr;
	IRenderTexturePtr mRendTexture = nullptr;
	SpritePtr mSprite, mLayerColor;
};

/********** TestRT **********/
void TestRT::OnInitLight()
{
	auto light1 = mContext->SceneMng()->AddPointLight();
	light1->SetPosition(20, 0, -20);
	light1->SetAttenuation(1.0, 0.01, 0);
	light1->SetSpecularPower(60);
}

void TestRT::OnPostInitDevice()
{
	mModel = new AssimpModel(*mContext->RenderSys(), *mContext->MaterialFac(), mTransform, E_MAT_MODEL);
	gModelPath = "Spaceship\\"; 
	mModel->LoadModel(MakeModelPath("Spaceship.fbx")); 
	mMoveDefScale = 0.01;
	mTransform->SetScale(Eigen::Vector3f(mMoveDefScale, mMoveDefScale, mMoveDefScale));
	mTransform->SetPosition(Eigen::Vector3f(0, 0, 0));
	
	mRendTexture = mContext->RenderSys()->CreateRenderTexture(mContext->RenderSys()->GetWinSize().x, 
		mContext->RenderSys()->GetWinSize().y);

	mSprite = std::make_shared<Sprite>(*mContext->RenderSys(), *mContext->MaterialFac(), E_MAT_SPRITE);
	mSprite->SetTexture(mRendTexture->GetColorTexture());
	mSprite->SetPosition(Eigen::Vector3f(0, 0, 0));
	mSprite->SetSize(Eigen::Vector2f(5, 5));

	/*mLayerColor = std::make_shared<TSprite>(mRenderSys, E_MAT_LAYERCOLOR);
	mLayerColor->SetPosition(-5, -5, 0);
	mLayerColor->SetSize(5, 5);*/
}

//#define USE_RENDER_TARGET
void TestRT::OnRender()
{
	mModel->Update(mTimer->mDeltaTime);
	mContext->RenderPipe()->Draw(*mModel, *mContext->SceneMng());
}

#if defined TEST_RT && TEST_CASE == TEST_RT
auto reg = AppRegister<TestRT>("TestRT: RenderTarget");
#endif