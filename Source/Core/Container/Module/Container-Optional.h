#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"

PARTING_SUBMODULE(Container, Optional)

PARTING_IMPORT std;


#else
#pragma once

#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
//Global

#include<optional>

#include "Core/Utility/Module/Utility.h"

#endif // PARTING_MODULE_BUILD

PARTING_EXPORT template<typename Type>
using Optional = std::optional<Type>;

PARTING_EXPORT std::nullopt_t NullOpt{ std::nullopt };

PARTING_EXPORT template<class _Ty>
STDNODISCARD inline constexpr Optional<std::decay_t<_Ty>> MakeOptional(_Ty&& value) {
	return std::make_optional(std::forward<_Ty>(value));
}

PARTING_EXPORT template<class Type, typename... Arges>
STDNODISCARD inline constexpr Optional<Type> MakeOptional(Arges&&... args) {
	return std::make_optional(std::forward<Arges>(args)...);
}

PARTING_EXPORT template<typename _Ty, typename _Elem, typename... _Arges>
STDNODISCARD inline constexpr Optional<_Ty> MakeOptional(std::initializer_list<_Elem> _Ilist, _Arges&&... _Args) {
	return std::make_optional(_Ilist, std::forward<_Arges>(_Args)...);
}