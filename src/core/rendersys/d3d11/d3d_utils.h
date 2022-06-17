#pragma once
#include <windows.h>
#include <d3d11.h>
#include <d3d9.h>
#include "core/predeclare.h"
#include "core/rendersys/base_type.h"
#include "core/rendersys/input_layout.h"

namespace mir {
namespace d3d {

bool CheckHResultFailed(HRESULT result);
bool CheckCompileFailed(HRESULT hr, ID3DBlob* pErrorBlob);
bool CheckCompileFailed(HRESULT hr, IBlobDataPtr data);

#if defined MIR_RESOURCE_DEBUG
void MIR_CORE_API ResourceAddDebugDevice(IResourcePtr res, void* device, const std::string& devName);
void MIR_CORE_API SetDebugPrivData(IResourcePtr res, const std::string& privData);
void MIR_CORE_API SetDebugResourcePath(IResourcePtr res, const std::string& resPath);
void MIR_CORE_API SetDebugCallStack(IResourcePtr res, const std::string& callstack);
#endif
}
}

#if defined _DEBUG
#define CheckHR(HR) mir::d3d::CheckHResultFailed(HR)
#else
#define CheckHR(HR) FAILED(HR)
#endif