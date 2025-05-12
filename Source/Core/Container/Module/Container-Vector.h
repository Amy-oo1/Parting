#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE

PARTING_SUBMODULE(Container, Vector)

PARTING_IMPORT std;

#else
#pragma once

#include "Core/ModuleBuild.h"

//Global

#include<vector>

#endif // PARTING_MODULE_BUILD#pragma once

PARTING_EXPORT template<typename Type>
using MVector = std::vector<Type, std::pmr::polymorphic_allocator<Type>>;

PARTING_EXPORT template<typename Type, typename Alloc = std::allocator<Type>>
using Vector = std::vector<Type, Alloc>;