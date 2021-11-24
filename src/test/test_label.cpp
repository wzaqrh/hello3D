#include "test/test_case.h"
#include "test/app.h"
#include "core/resource/material_factory.h"
#include "core/scene/scene_manager.h"
#include "core/base/transform.h"
#include "core/renderable/label.h"

using namespace mir;

#define CASE_COUNT 4

class TestLabel : public App
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
private:
	LabelPtr mLabel[CASE_COUNT];
	SpritePtr mLabelBg[CASE_COUNT];
	SpritePtr mLabelDebug;
};

void TestLabel::OnPostInitDevice()
{
	mContext->SceneMng()->RemoveAllCameras();
	mContext->SceneMng()->AddOthogonalCamera(Eigen::Vector3f(0,0,-10), 100);

	for (int i = 0; i < CASE_COUNT; ++i) {
		auto label = mContext->RenderableFac()->CreateLabel("D:\\ProjectWork1\\hello3D\\Debug\\msyh.ttc", 24);
		mLabel[i] = label;
		if (i == 0) {
			label->SetString("HelloWorld");
			label->GetTransform()->SetPosition(Eigen::Vector3f(0, 0, 0));
		}
		else if (i == 1) {
			label->SetString("Hello\nWorld");
			label->GetTransform()->SetPosition(Eigen::Vector3f(200, 0, 0));
		}
		else if (i == 2) {
			label->SetSize(false, Eigen::Vector2f(120, 64));
			label->SetString("HelloWorld");
			label->GetTransform()->SetPosition(Eigen::Vector3f(400, 0, 0));
		}
		else if (i == 3) {
			label->SetString("Hello\nWorld");
			label->GetTransform()->SetPosition(Eigen::Vector3f(600, 0, 0));
			label->SetSize(false, Eigen::Vector2f(120, 64));
		}

		auto labelBg = mContext->RenderableFac()->CreateColorLayer();
		mLabelBg[i] = labelBg;
		labelBg->SetColor(Eigen::Vector4f(1, 0, 0, 1));
		labelBg->SetSize(label->GetSize());
		labelBg->GetTransform()->SetPosition(label->GetTransform()->GetPosition());
	}
#ifdef TEST_LABEL_DEBUG_SP
	//mSprite = std::make_shared<TSprite>(mContext->GetRenderSys(), E_MAT_LABEL);
	mLabelDebug = std::make_shared<Sprite>(mContext->RenderSys(), E_MAT_SPRITE);
	mLabelDebug->SetTexture(mLabel->mCharSeq[0].texture);
	mLabelDebug->SetPosition(0, 0, 0);
	mLabelDebug->SetSize(512, 512);
#endif
}

void TestLabel::OnRender()
{
	if (mContext->RenderPipe()->BeginFrame()) {
		RenderOperationQueue opQue;
		if (mLabelDebug) mLabelDebug->GenRenderOperation(opQue);
		for (int i = 0; i < CASE_COUNT; ++i) {
			if (mLabelBg[i]) mLabelBg[i]->GenRenderOperation(opQue);
			if (mLabel[i]) mLabel[i]->GenRenderOperation(opQue);
		}
		mContext->RenderPipe()->Render(opQue, *mContext->SceneMng());
		mContext->RenderPipe()->EndFrame();
	}
}

#if defined TEST_LABEL && TEST_CASE == TEST_LABEL
auto reg = AppRegister<TestLabel>("Label");
#endif