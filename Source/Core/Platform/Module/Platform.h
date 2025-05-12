#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

GLOBAL_MODULE

PARTING_MODULE(Pltform)

PARTING_EXPORT PARTING_SUBMODE_IMPORT(Type);
PARTING_EXPORT PARTING_SUBMODE_IMPORT(Windows)

#else 
#pragma once

#include "Core/ModuleBuild.h"

//Global

#include "Core/Platform/Module/Platform-Type.h"
#include "Core/Platform/Module/Platform-Windows.h"
#endif