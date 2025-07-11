#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE

PARTING_SUBMODULE(Utility, Traits)

PARTING_IMPORT std;

#else
#pragma once

#include "Core/ModuleBuild.h"

//Global

#include<type_traits>

#endif // PARTING_MODULE_BUILD

PARTING_EXPORT template <class _Ty>
using UnderlyingType = typename  std::underlying_type_t<_Ty>;

PARTING_EXPORT template<typename _Ty>
using RemoveCV = std::remove_cv<_Ty>;

PARTING_EXPORT template<typename _Ty>
using RemoveRef = std::remove_reference<_Ty>;

PARTING_EXPORT template<typename _Ty>
using RemovePointer = std::remove_pointer<_Ty>;

PARTING_EXPORT template<typename _Ty>
using RemoveExtent = std::remove_extent<_Ty>;

PARTING_EXPORT template<typename _Ty>
using RemoveAllExtents = std::remove_all_extents<_Ty>;

PARTING_EXPORT template<typename _Ty>
using Decay = std::decay <_Ty>;

PARTING_EXPORT template<typename _Ty>
using AddPointer = std::add_pointer<_Ty>;

PARTING_EXPORT template<typename... _Ty>
using CommonType = std::common_type_t<_Ty...>;

template <class _Ty>std::add_rvalue_reference_t<_Ty> Declval() noexcept { static_assert(false, "Calling declval is ill-formed, see N4950 [declval]/2."); }