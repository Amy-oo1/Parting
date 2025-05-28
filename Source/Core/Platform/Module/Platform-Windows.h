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

#include "shellapi.h"
#include "ShellScalingApi.h"
#include "WinUser.h"
#include "processenv.h"
//TODO useed to wapper in module

PARTING_EXPORT using FileOffset_T = _off_t;

constexpr UINT PlatformWindowsAccessGenericRead{ GENERIC_READ };
constexpr UINT PlatformWindowsAccessGenericWrite{ GENERIC_WRITE };
constexpr UINT PlatformWindowsAccessGenericExecute{ GENERIC_EXECUTE };
constexpr UINT PlatformWindowsAccessGenericAll{ GENERIC_ALL };//TODO :Trans

std::filesystem::path Get_CatallogDirectory(void) {
	char path[MAX_PATH] = {};
	if (0 == GetModuleFileNameA(nullptr, path, MAX_PATH))
		assert(false);

	return std::filesystem::path(path).parent_path().parent_path();//remove de/re
}




