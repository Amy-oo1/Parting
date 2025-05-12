#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
PARTING_GLOBAL_MODULE

PARTING_SUBMODULE(Memory, Alloctor)

PARTING_IMPORT std;

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;

#else
#pragma once

#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
//Global

#include<memory>
#include<memory_resource>

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"

#endif // PARTING_MODULE_BUILD

PARTING_EXPORT using MemopyResource = std::pmr::memory_resource;

PARTING_EXPORT STDNODISCARD inline MemopyResource* NewDeleteResource(void) { return std::pmr::new_delete_resource(); }

PARTING_EXPORT STDNODISCARD inline MemopyResource* NullMemoryResource(void) { return std::pmr::null_memory_resource(); }

PARTING_EXPORT STDNODISCARD inline MemopyResource* Get_DefaultResource(void) { return std::pmr::get_default_resource(); }

//NOTE : DO NOT Set Becase has memory class use new/delete to do sync in ThreeadSfte 
PARTING_EXPORT inline void Set_DefaultResource(MemopyResource* resource) { std::pmr::set_default_resource(resource); }

PARTING_EXPORT using PoolOptions = std::pmr::pool_options;

PARTING_EXPORT using SynchronizedPoolResource = std::pmr::synchronized_pool_resource;

PARTING_EXPORT using UnsynchronizedPoolResource = std::pmr::unsynchronized_pool_resource;

PARTING_EXPORT using MonotonicBufferResource = std::pmr::monotonic_buffer_resource;

PARTING_EXPORT template<typename Type = std::byte>
using PolymorphicAllocator = std::pmr::polymorphic_allocator<Type>;