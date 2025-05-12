#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"

PARTING_SUBMODULE(Container, Tuple)

PARTING_IMPORT std;

PARTING_IMPORT Utility;

#else
#pragma once

#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
//Global
#include<tuple>
#include<utility>

#include "Core/Utility/Module/Utility.h"

#endif // PARTING_MODULE_BUILD

PARTING_EXPORT template <class _Ty1, class _Ty2>
using Pair = std::pair<_Ty1, _Ty2>;

PARTING_EXPORT template <class _This, class... _Rest>
using Tuple = std::tuple<_This, _Rest...>;

PARTING_EXPORT template <class _Ty1, class _Ty2>
STDNODISCARD constexpr std::pair<std::_Unrefwrap_t<_Ty1>, std::_Unrefwrap_t<_Ty2>> MakePair(_Ty1&& _Arg1, _Ty2&& _Arg2) { return std::make_pair(std::forward<_Ty1>(_Arg1), std::forward<_Ty2>(_Arg2)); }

PARTING_EXPORT template <class... _Types>
STDNODISCARD constexpr auto MakeTuple(_Types&&... _Args) { return std::make_tuple(std::forward<_Types>(_Args)...); }

PARTING_EXPORT template <class... _Types>
STDNODISCARD constexpr auto Tie(_Types&... _Args) { return std::tie(_Args...); }