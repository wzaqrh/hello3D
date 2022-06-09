#pragma once
#include "test/framework/app.h"

void RegisterApp(const char* name, std::function<IApp*()> appClassCreator);
template<class T> struct AppRegister
{
	AppRegister(const char* name)
	{
		RegisterApp(name, [name]()->IApp* {
			T* app = new T;
			app->mName = name;
			return app;
		});
	}
};

std::string& GetCurrentAppName();
IApp* CreateApp(std::string name = "");
IApp* CreateAppByCommandLineOrCommandFile();