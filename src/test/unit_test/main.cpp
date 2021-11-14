//--------------------------------------------------------------------------------------
// File: Tutorial06.cpp
//
// This application demonstrates simple lighting in the vertex shader
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include <windows.h>
#define CATCH_CONFIG_RUNNER
#include <catch.hpp>

int main(int argc, const char* argv[]) 
{
	int result = Catch::Session().run<char>(argc, argv);
	if (result)
		system("pause");
	return result;
}