#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE

PARTING_MODULE(String)

PARTING_IMPORT std;

#else
#pragma once

#include "Core/ModuleBuild.h"

//Global
#include<string>

#endif // PARTING_MODULE_BUILD#pragma once

PARTING_EXPORT using String = std::string;
PARTING_EXPORT using MString = std::pmr::string;
PARTING_EXPORT using StringView = std::string_view;

//NOTE :In Platformt used in wchar_t ,and it selif define 16(windows) or 32(linux) bit
PARTING_EXPORT using WString = std::wstring;
PARTING_EXPORT using MWString = std::pmr::wstring;
PARTING_EXPORT using WStringView = std::wstring_view;

PARTING_EXPORT using u8String = std::u8string;
PARTING_EXPORT using Mu8String = std::pmr::u8string;
PARTING_EXPORT using u8StringView = std::u8string_view;

String WstringtoString(const WString& wstr) {
	Uint64 Length{ wcsnlen(wstr.c_str(), wstr.size()) };

	String Re;
	Re.resize(Length);

	WideCharToMultiByte(CP_ACP, 0,wstr.c_str(), static_cast<Int32>(Length), Re.data(), static_cast<Int32>(Re.size()), nullptr, nullptr);

	return Re;
}
