#pragma once
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#include "core/gui/gui_canvas.h"

namespace mir {

class MIR_CORE_API GuiManager : boost::noncopyable
{
public:
	GuiManager(void* hwnd);
	~GuiManager();
	CoTask<bool> Initialize(Launch lchMode, ResourceManager& resMng);

	void ClearCommands();
	void AddCommand(std::function<void()> cmd);
	void UpdateFrame(float dt);

	gui::GuiCanvasPtr GetCanvas() const { return mCanvas; }
private:
	gui::GuiCanvasPtr mCanvas;
	std::vector<std::function<void()>> mCmds;
};

MIR_CORE_API LRESULT GuiManager_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
}