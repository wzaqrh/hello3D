#include "core/rendersys/resource_manager.h"

namespace mir {

ResourceManager::ResourceManager(RenderSystem& renderSys)
	:mRenderSys(renderSys)
{}

IInputLayoutPtr ResourceManager::CreateLayoutAsync(IProgramPtr pProgram, LayoutInputElement descArray[], size_t descCount)
{
	return nullptr;
}

}