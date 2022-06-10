#include "core/base/debug.h"
#include "core/gui/imgui_impl_win32.h"
#include "core/gui/gui_manager.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace mir {

LRESULT GuiManager_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
}

GuiManager::GuiManager(void* hwnd)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	ImGui_ImplWin32_Init(hwnd);

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Read 'docs/FONTS.md' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != NULL);
}
CoTask<bool> GuiManager::Initialize(Launch lchMode, ResourceManager& resMng)
{
	mCanvas = CreateInstance<gui::GuiCanvas>(lchMode, resMng);
	CoAwait mCanvas->Init();
	CoReturn true;
}

GuiManager::~GuiManager()
{
	DEBUG_LOG_MEMLEAK("guiMng.destrcutor");
	Dispose();
}
void GuiManager::Dispose()
{
	if (mCanvas) {
		DEBUG_LOG_MEMLEAK("guiMng.dispose");
		mCanvas = nullptr;
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}
}

void GuiManager::ClearCommands()
{
	mCmds.clear();
}

void GuiManager::AddCommand(std::function<CoTask<void>()> cmd)
{
	mCmds.push_back(cmd);
}

CoTask<void> GuiManager::UpdateFrame(float dt)
{
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	for (auto& cmd : mCmds)
		CoAwait cmd();

	ImGui::Render();
	CoReturn;
}

}