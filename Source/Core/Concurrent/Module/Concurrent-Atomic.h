#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE

PARTING_SUBMODULE(Concurrent, Atomic)

PARTING_IMPORT std;

#else 
#pragma once

#include "Core/ModuleBuild.h"

//Global

#include<atomic>

#endif // PARTING_MODULE_BUILD

PARTING_EXPORT template<typename Type>
using Atomic = std::atomic<Type>;

PARTING_EXPORT template<typename Type>
using AtomicRef = std::atomic_ref<Type>;

PARTING_EXPORT using MemoryOrder = std::memory_order;

PARTING_EXPORT constexpr auto MemoryOrderRelaxd = MemoryOrder::relaxed;
PARTING_EXPORT constexpr auto MemoryOrderConsume = MemoryOrder::consume;//NOTE :Not Ues
PARTING_EXPORT constexpr auto MemoryOrderAcquire = MemoryOrder::acquire;
PARTING_EXPORT constexpr auto MemoryOrderRelease = MemoryOrder::release;
PARTING_EXPORT constexpr auto MemoryOrderAcqRel = MemoryOrder::acq_rel;
PARTING_EXPORT constexpr auto MemoryOrderSeqCst = MemoryOrder::seq_cst;//NOTE :Not Ues
