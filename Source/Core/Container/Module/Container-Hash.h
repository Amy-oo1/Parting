#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE

PARTING_SUBMODULE(Container, Hash)

PARTING_IMPORT std;

PARTING_IMPORT Platform;

#else
#pragma once

#include "Core/ModuleBuild.h"

//Global
#include<xhash>
#include<string>

#include "Core/Platform/Module/Platform.h"

#endif // PARTING_MODULE_BUILD

PARTING_EXPORT template<typename _Ty>
using Hash = std::hash<_Ty>;

PARTING_EXPORT using HashBool = std::hash<bool>;
PARTING_EXPORT using HashUint8 = std::hash<Uint8>;
PARTING_EXPORT using HashUint16 = std::hash<Uint16>;
PARTING_EXPORT using HashUint32 = std::hash<Uint32>;
PARTING_EXPORT using HashUint64 = std::hash<Uint64>;

PARTING_EXPORT using HashInt8 = std::hash<Int8>;
PARTING_EXPORT using HashInt16 = std::hash<Int16>;
PARTING_EXPORT using HashInt32 = std::hash<Int32>;
PARTING_EXPORT using HashInt64 = std::hash<Int64>;

PARTING_EXPORT using HashChar = std::hash<char>;
PARTING_EXPORT using HashChar8 = std::hash<char8_t>;
PARTING_EXPORT using HashWChar = std::hash<wchar_t>;

PARTING_EXPORT using HashFloat = std::hash<float>;
PARTING_EXPORT using HashDouble = std::hash<double>;

PARTING_EXPORT using HashVoidPtr = std::hash<void*>;
PARTING_EXPORT using HashNullPtr = std::hash<std::nullptr_t>;

PARTING_EXPORT using HashAnsiString = std::hash<std::string>;
PARTING_EXPORT using HashU8String = std::hash<std::u8string>;
PARTING_EXPORT using HashWString = std::hash<std::wstring>;

PARTING_EXPORT using HashAnsiStringView = std::hash<std::string_view>;
PARTING_EXPORT using HashU8StringView = std::hash<std::u8string_view>;
PARTING_EXPORT using HashWStringView = std::hash<std::wstring_view>;

PARTING_EXPORT constexpr Uint64 HashCombine(Uint64 seed, Uint64 value) {
	return seed ^ (value + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}

namespace std {
	template<typename TypeFirst, typename TypeSecond>
	struct hash<std::pair<TypeFirst, TypeSecond>> {
		size_t operator()(const std::pair<TypeFirst, TypeSecond>& pair) const {
			return ::HashCombine(Hash<TypeFirst>{}(pair.first), ::Hash<TypeSecond>{}(pair.second));
		}
	};

	template<typename TypeFirst, typename TypeSecond>
	struct hash<std::tuple<TypeFirst, TypeSecond>> {
		size_t operator()(const std::tuple<TypeFirst, TypeSecond>& tuple) const {
			return ::HashCombine(Hash<TypeFirst>{}(std::get<0>(tuple)), ::Hash<TypeSecond>{}(std::get<1>(tuple)));
		}
	};
}
