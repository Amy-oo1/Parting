#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"

PARTING_SUBMODULE(Container, Variant)

PARTING_IMPORT std;

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;

#else
#pragma once

#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
//Global
#include<variant>
#include<utility>

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"

#endif // PARTING_MODULE_BUILD

PARTING_EXPORT	template<typename ...Types>
using Variant = std::variant<Types...>;

PARTING_EXPORT template <class _Ty, class... _Types>
STDNODISCARD constexpr bool HoldsAlternative(const std::variant<_Types...>& _Val) { return std::holds_alternative<_Ty>(_Val); }

PARTING_EXPORT template <size_t _Idx, class... _Types>
STDNODISCARD constexpr decltype(auto) Get(std::variant<_Types...>& _Val) { return std::get<_Idx>(_Val); }

PARTING_EXPORT template <size_t _Idx, class... _Types>
STDNODISCARD constexpr decltype(auto) Get(std::variant<_Types...>&& _Var) { return std::get<_Idx>(_Var); }

PARTING_EXPORT template <size_t _Idx, class... _Types>
STDNODISCARD constexpr decltype(auto) Get(const std::variant<_Types...>& _Val) { return std::get<_Idx>(_Val); }

PARTING_EXPORT template <size_t _Idx, class... _Types>
STDNODISCARD constexpr decltype(auto) Get(const std::variant<_Types...>&& _Val) { return std::get<_Idx>(_Val); }

PARTING_EXPORT template <class _Ty, class... _Types>
STDNODISCARD constexpr decltype(auto) Get(std::variant<_Types...>& _Val) { return std::get<_Ty>(_Val); }

PARTING_EXPORT template <class _Ty, class... _Types>
STDNODISCARD constexpr decltype(auto) Get(std::variant<_Types...>&& _Val) { return std::get<_Ty>(_Val); }

PARTING_EXPORT template <class _Ty, class... _Types>
STDNODISCARD constexpr decltype(auto) Get(const std::variant<_Types...>& _Val) { return std::get<_Ty>(_Val); }

PARTING_EXPORT template <class _Ty, class... _Types>
STDNODISCARD constexpr decltype(auto) Get(const std::variant<_Types...>&& _Val) { return std::get<_Ty>(_Val); }


//PARTING_EXPORT template <size_t _Idx, class... _Types>
//STDNODISCARD constexpr auto GetIf(std::variant<_Types...>* _Ptr) noexcept { return std::get_if<_Idx>(_Ptr); }
//
//PARTING_EXPORT template <size_t _Idx, class... _Types>
//STDNODISCARD constexpr auto GetIf(const std::variant<_Types...>* _Ptr) noexcept { return std::get_if<_Idx>(_Ptr); }

PARTING_EXPORT template <class _Ty, class... _Types>
STDNODISCARD constexpr auto GetIf(std::variant<_Types...>* _Ptr) noexcept { return std::get_if<_Ty>(_Ptr); }

PARTING_EXPORT template <class _Ty, class... _Types>
STDNODISCARD constexpr auto GetIf(const std::variant<_Types...>* _Ptr) noexcept { return std::get_if<_Ty>(_Ptr); }