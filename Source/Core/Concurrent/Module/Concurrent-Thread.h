#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE

PARTING_SUBMODULE(Concurrent, Atomic)

PARTING_IMPORT std;

#else 
#pragma once

#include "Core/ModuleBuild.h"

//Global
#include<thread>

#endif // PARTING_MODULE_BUILD

PARTING_EXPORT using Thread = std::jthread;


namespace ThisThrad {
	PARTING_EXPORT using  std::this_thread::yield;
	PARTING_EXPORT using  std::this_thread::sleep_for;
	PARTING_EXPORT using  std::this_thread::sleep_until;
	PARTING_EXPORT using  std::this_thread::get_id;
}