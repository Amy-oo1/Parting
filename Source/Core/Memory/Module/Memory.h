#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE

PARTING_MODULE(Memory)

PARTING_EXPORT PARTING_SUBMODE_IMPORT(Alloctor)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(Tracker)

#else
#pragma once

#include "Core/ModuleBuild.h"

//Global

#include "Core/Memory/Module/Memory-Alloctor.h"
#include "Core/Memory/Module/Memory-Tracker.h"

#endif // PARTING_MODULE_BUILD