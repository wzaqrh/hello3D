#pragma once

#define MIR_VER_MAJOR  1
#define MIR_VER_MINOR  1
#define MIR_VER_PATCH  0

#define MIR_HAS_DECLSPEC
#define MIR_SYMBOL_EXPORT __declspec(dllexport)
#define MIR_SYMBOL_IMPORT __declspec(dllimport)

#ifdef MIR_CORE_SOURCE		// Build dll
#define MIR_CORE_API MIR_SYMBOL_EXPORT
#else							// Use dll
#define MIR_CORE_API MIR_SYMBOL_IMPORT
#endif