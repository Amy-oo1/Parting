#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE

PARTING_SUBMODULE(Container, List)

PARTING_IMPORT std;

#else
#pragma once

#include "Core/ModuleBuild.h"

//Global

#include<list>

#endif // PARTING_MODULE_BUILD#pragma once

PARTING_EXPORT template<typename _Ty>
using List = std::list<_Ty>;