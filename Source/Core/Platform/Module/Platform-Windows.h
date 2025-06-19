#pragma once


#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE

PARTING_MODULE(Container)

#else
#pragma once

//#include "Core/ModuleBuild.h"

//Global


#endif // PARTING_MODULE_BUILD#pragma once

#include<cassert>
#include<string>
#include<filesystem>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

#ifndef NOMINMAX
#define NOMINMAX
#endif // !NOMINMAX
#ifndef NOGDI
#define NOGDI
#endif // !NOGDI
#include "windows.h"

//#include "shellapi.h"
#include "ShellScalingApi.h"
//#include "WinUser.h"
//#include "processenv.h"
//TODO useed to wapper in module

#include "Core/Logger/Module/Logger.h"
#include "Core/Logger/Include/LogMacros.h"

std::filesystem::path Get_CatallogDirectory(void) {
	char path[MAX_PATH] = {};
	if (0 == GetModuleFileNameA(nullptr, path, MAX_PATH)) {

		LOG_ERROR("GetModuleFileNameA failed with error code: {}", GetLastError());
	}

	return std::filesystem::path(path).parent_path().parent_path();//remove de/re
}




