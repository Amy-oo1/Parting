#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"

PARTING_SUBMODULE(Container, Hash)

PARTING_IMPORT std;

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Concurrent;
PARTING_IMPORT Memory;

PARTING_SUBMODE_IMPORT(Span)

PARTING_IMPORT std;

#else
#pragma once

#include "Core/ModuleBuild.h"

//Global
#include<span>

#endif // PARTING_MODULE_BUILD

PARTING_EXPORT template <class _Ty, size_t _Extent = std::dynamic_extent>
using Span = std::span<_Ty>;