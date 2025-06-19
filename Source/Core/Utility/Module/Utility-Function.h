#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"

PARTING_SUBMODULE(Utility, Function)

PARTING_IMPORT std;

#else
#pragma once

#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
//Global

#include<type_traits>
#include<xutility>

#endif // PARTING_MODULE_BUILD


//NOTE  : this Function in cpp23 ,so i write by myself
PARTING_EXPORT template <typename Enum>
STDNODISCARD inline constexpr std::underlying_type_t<Enum> Tounderlying(Enum e) noexcept { return static_cast<std::underlying_type_t<Enum>>(e); }

PARTING_EXPORT template<typename Type>
STDNODISCARD inline constexpr std::remove_reference_t<Type>&& MoveTemp(Type&& value) noexcept { return std::move(value); }

PARTING_EXPORT template<typename _Ty>
STDNODISCARD inline constexpr _Ty&& Forward(_Ty& value) noexcept { return std::forward<_Ty>(value); }

PARTING_EXPORT template<typename _Ty>
STDNODISCARD inline constexpr _Ty&& Forward(_Ty&& value) noexcept { return std::forward<_Ty>(value); }

PARTING_EXPORT template<typename _Ty>
STDNODISCARD inline constexpr _Ty* Addressof(_Ty& value) noexcept { return std::addressof(value); }

PARTING_EXPORT template<typename _Ty>
STDNODISCARD inline constexpr _Ty* Addressof(_Ty&& value) noexcept { return std::addressof(value); }

PARTING_EXPORT template<typename _Ty>
STDNODISCARD inline constexpr void Swap(_Ty& lhs, _Ty& rhs) noexcept { std::swap(lhs, rhs); }//TODO to use in single type