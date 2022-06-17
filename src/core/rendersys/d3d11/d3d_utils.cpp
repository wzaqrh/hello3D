#include <boost/assert.hpp>
#include <d3dcompiler.h>
#include <dxerr.h>
#include "core/base/debug.h"
#include "core/rendersys/blob.h"
#include "core/rendersys/d3d11/d3d_utils.h"

using namespace mir::debug;

namespace mir {
namespace d3d {

/********** CheckHRXXX **********/
bool CheckHResultFailed(HRESULT hr)
{
	if (FAILED(hr)) {
		Log(DXGetErrorDescriptionA(hr));
		Log(DXGetErrorStringA(hr));

		//DXTRACE_ERR_MSGBOX(DXGetErrorDescription(hr), hr);
		//DXTRACE_ERR_MSGBOX(DXGetErrorString(hr), hr);
		//DXTRACE_ERR_MSGBOX(L"Clear failed!", hr); // Use customized error string
		//DXTRACE_MSG(DXGetErrorDescription(hr));
		//DXTRACE_ERR(DXGetErrorDescription(hr), hr);

		BOOST_ASSERT(false);
		return true;
	}
	return false;
}

bool CheckCompileFailed(HRESULT hr, ID3DBlob* pErrorBlob)
{
	if (FAILED(hr))
	{
		if (pErrorBlob != NULL)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
			pErrorBlob = nullptr;
		}
		CheckHR(hr);
		return true;
	}
	return false;
}

bool CheckCompileFailed(HRESULT hr, IBlobDataPtr data)
{
	if (FAILED(hr))
	{
		ID3DBlob* pErrorBlob = nullptr;
		D3DGetDebugInfo(data->GetBytes(), data->GetSize(), &pErrorBlob);
		if (pErrorBlob != NULL)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
			pErrorBlob = nullptr;
		}
		CheckHR(hr);
		return true;
	}
	return false;
}

/********** SetDebugXXX **********/
#if defined MIR_RESOURCE_DEBUG
static void SetChildrenPrivateData(const std::vector<std::pair<void*, std::string>>& children, const std::string& name)
{
	for (auto& child : children) {
		if (child.first) {
			std::string str_child_res = "__child_res";
			if (child.second.size() >= str_child_res.size() && child.second.substr(0, str_child_res.size()) == str_child_res) {
				IResource* child_res = (IResource*)(child.first);

				child_res->_Debug._PrivData = name;
				if (child.second.size() > str_child_res.size())
					child_res->_Debug._PrivData += "." + child.second.substr(str_child_res.size());

				SetChildrenPrivateData(child_res->_Debug._DeviceChilds, child_res->_Debug.GetDebugInfo());
			}
			else {
				auto fullname = name;
				if (!child.second.empty())
					fullname += "." + child.second;

				static_cast<ID3D11DeviceChild*>(child.first)->SetPrivateData(WKPDID_D3DDebugObjectName, fullname.size(), fullname.c_str());
			}
		}
	}
}

void ResourceAddDebugDevice(IResourcePtr res, void* device, const std::string& devName)
{
#if defined MIR_RESOURCE_DEBUG
	if (res) {
		res->_Debug._DeviceChilds.push_back(std::make_pair(device, devName));
		SetChildrenPrivateData(res->_Debug._DeviceChilds, res->_Debug.GetDebugInfo());
	}
#endif
}

void SetDebugPrivData(IResourcePtr res, const std::string& privData)
{
#if defined MIR_RESOURCE_DEBUG
	if (res) {
		res->_Debug._PrivData = (privData);
		SetChildrenPrivateData(res->_Debug._DeviceChilds, res->_Debug.GetDebugInfo());
	}
#endif
}

void SetDebugResourcePath(IResourcePtr res, const std::string& resPath)
{
#if defined MIR_RESOURCE_DEBUG
	if (res) {
		res->_Debug._ResourcePath = (resPath);
		SetChildrenPrivateData(res->_Debug._DeviceChilds, res->_Debug.GetDebugInfo());
	}
#endif
}

void SetDebugCallStack(IResourcePtr res, const std::string& callstack)
{
#if defined MIR_RESOURCE_DEBUG
	if (res) {
		res->_Debug._CallStack = (callstack);
		SetChildrenPrivateData(res->_Debug._DeviceChilds, res->_Debug.GetDebugInfo());
	}
#endif
}
#endif

}
}