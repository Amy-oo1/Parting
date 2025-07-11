#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"

PARTING_SUBMODULE(D3D12RHI, Heap)

PARTING_IMPORT DirectX12Wrapper;

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Container;

PARTING_IMPORT RHI;

PARTING_SUBMODE_IMPORT(Traits)
PARTING_SUBMODE_IMPORT(Common)
PARTING_SUBMODE_IMPORT(Format)

#else
#pragma once

#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
//Global
#include "VulkanWrapper.h"

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Container/Module/Container.h"

#include "RHI/Module/RHI.h"

#include "VulkanRHI/Module/VulkanRHI-Traits.h"
#include "VulkanRHI/Module/VulkanRHI-Common.h"

#endif // PARTING_MODULE_BUILD


namespace RHI::Vulkan {
	class Heap final : public RHIHeap<Heap>, public MemoryResource {
		friend class RHIResource<Heap>;
		friend class RHIHeap<Heap>;

		friend class CommandList;
		friend class Device;

	public:
		explicit Heap(VulkanAllocator& allocator, RHIHeapDesc desc) :
			RHIHeap<Heap>{},
			MemoryResource{},
			m_Allocator{ allocator },
			m_Desc{ desc } {
		}

		~Heap(void);

	private:
		VulkanAllocator& m_Allocator;

		RHIHeapDesc m_Desc;

	private:
		RHIObject Imp_GetNativeObject(RHIObjectType type)const noexcept { LOG_ERROR("Imp But Empty"); return RHIObject{}; }
		
		const RHIHeapDesc& Imp_Get_Desc(void) { return this->m_Desc; }
	};

	//Src

	Heap::~Heap(void) {
		if (nullptr != this->MemoryResource::Memory && this->MemoryResource::Managed) {
			this->m_Allocator.FreeMemory(this);
			this->MemoryResource::Memory = nullptr;
		}
	}

}