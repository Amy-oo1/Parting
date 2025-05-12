#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
PARTING_GLOBAL_MODULE

PARTING_SUBMODULE(Memory, Alloctor)

PARTING_IMPORT std;

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;

PARTING_SUBMODE_IMPORT(Allocator)

#else
#pragma once

#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
//Global
#include<memory>
#include<memory_resource>
#include<string>
#include<iostream>


#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"

#include "Core/Memory/Module/Memory-Alloctor.h"

#endif // PARTING_MODULE_BUILD

PARTING_EXPORT class Tracker final :public MoveAbleOnly, public MemopyResource {
public:
	explicit Tracker(MemopyResource* us = Get_DefaultResource()) :
		MoveAbleOnly{},
		MemopyResource{},
		m_UpStream(us) {
	}

	explicit Tracker(const std::string& prefix, MemopyResource* us = Get_DefaultResource()) :
		MoveAbleOnly{},
		MemopyResource{},
		m_UpStream(us),
		m_Prefix(prefix) {
	}

private:
	MemopyResource* m_UpStream;
	std::string m_Prefix{};

private:
	void* do_allocate(Uint64 bytes, Uint64 alignment) override {
		std::cout << reinterpret_cast<const char*>(this->m_Prefix.c_str()) << ("Allocating ") << bytes << " bytes of memory with alignment " << alignment << std::endl;

		return m_UpStream->allocate(bytes, alignment);
	}

	void do_deallocate(void* ptr, size_t bytes, size_t alignment) override {
		std::cout << reinterpret_cast<const char*>(this->m_Prefix.c_str()) << ("Deallocating ") << ptr << std::endl;

		m_UpStream->deallocate(ptr, bytes, alignment);
	}

	bool do_is_equal(const MemopyResource& other) const noexcept override {
		if (this == &other)
			return true;

		auto op{ dynamic_cast<const Tracker*>(&other) };
		return op != nullptr && op->m_Prefix == this->m_Prefix && this->m_UpStream->is_equal(other);
	}
};