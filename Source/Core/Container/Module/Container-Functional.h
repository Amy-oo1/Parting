#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE

PARTING_SUBMODULE(Container, Functional)

PARTING_IMPORT std;

#else
#pragma once

#include "Core/ModuleBuild.h"

//Global
#include<functional>

#endif // PARTING_MODULE_BUILD

PARTING_EXPORT template<typename Type>
using Function = std::function<Type>;

PARTING_EXPORT template<typename Type>
using Is_Function = std::is_function<Type>;