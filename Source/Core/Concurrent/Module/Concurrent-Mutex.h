#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"

PARTING_SUBMODULE(Concurrent, Mutex)

PARTING_IMPORT std;

#else 
#pragma once
#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
//Global

#include<mutex>
#include<shared_mutex>

#include "Core/Utility/Module/Utility.h"

#endif // PARTING_MODULE_BUILD

PARTING_EXPORT using Mutex = std::mutex;

PARTING_EXPORT using SharedMutex = std::shared_mutex;

PARTING_EXPORT template<typename  _Mutex>
using LockGuard = std::lock_guard<_Mutex>;

PARTING_EXPORT template<typename _Mutex>
using UniqueLock = std::unique_lock<_Mutex>;

PARTING_EXPORT template<typename _Mutex>
using SharedLock = std::shared_lock<_Mutex>;