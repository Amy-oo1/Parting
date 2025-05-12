#pragma once

#ifdef PARTING_MODULE_BUILD
#define PARTING_GLOBAL_MODULE module;
#define PARTING_MODULE (ModuleName)export module ModuleName;
#define PARTING_SUBMODULE (ModuleName,SubModuleName) export module ModuleName :SubModuleName;
#define PARTING_EXPORT export
#define PARTING_IMPORT import
#define PARTING_SUBMODE_IMPORT (ModuleName) import :ModuleName;

#define HEADER_INLINE
#else
#define PARTING_EXPORT 
#define PARTING_IMPORT

#define HEADER_INLINE inline
#endif // PARTING_MODULE


//NOTE : this is a template 

#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE

PARTING_MODULE(Container)

#else
#pragma once

//#include "Core/ModuleBuild.h"

//Global


#endif // PARTING_MODULE_BUILD

