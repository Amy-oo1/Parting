#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE

PARTING_MODULE(Concurrent)

PARTING_EXPORT PARTING_SUBMODE_IMPORT(Concurrent, Atomic)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(Concurrent, Mutex)

#else
#pragma once

#include "Core/ModuleBuild.h"

//Global

#include "Core/Concurrent/Module/Concurrent-Atomic.h"
#include "Core/Concurrent/Module/Concurrent-Mutex.h"

#endif // PARTING_MODULE_BUILD
