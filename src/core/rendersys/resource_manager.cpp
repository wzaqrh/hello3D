#include <boost/assert.hpp>
#include "core/rendersys/resource_manager.h"
#include "core/rendersys/render_system.h"
#include "core/rendersys/interface_type.h"

namespace mir {

ResourceManager::ResourceManager(RenderSystem& renderSys)
	:mRenderSys(renderSys)
{}

void ResourceManager::UpdateForLoading()
{
	const std::vector<IResourcePtr>& topNodes = mResDependencyTree.TopNodes();
	for (auto& res : topNodes) {
		if (res->IsPreparedNeedLoading()) {
			auto task = mLoadTaskByRes[res];
			task(res);
		}
	}
	for (auto& res : topNodes) {
		if (res->IsLoaded()) {
			mResDependencyTree.RemoveNode(res);
			mLoadTaskByRes.erase(res);
		}
	}
}

IInputLayoutPtr ResourceManager::CreateLayoutAsync(IProgramPtr pProgram, const LayoutInputElement descArray[], size_t descCount)
{
	BOOST_ASSERT(pProgram && descCount > 0);

	IResourcePtr res = mRenderSys.CreateResource(kDeviceResourceInputLayout);
	mResDependencyTree.AddNode(res, pProgram);
	
	std::vector<LayoutInputElement> descVec(descArray, descArray + descCount);
	mLoadTaskByRes[res] = [=](IResourcePtr res) {
		mRenderSys.LoadLayout(res, pProgram, &descVec[0], descVec.size());
	};
	return std::static_pointer_cast<IInputLayout>(res);
}

}