#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"


PARTING_SUBMODULE(D3D12RHI, Buffer)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Algorithm;
PARTING_IMPORT Container;
PARTING_IMPORT String;
PARTING_IMPORT Color;
PARTING_IMPORT Logger;

PARTING_IMPORT RHI;

PARTING_SUBMODE_IMPORT(Traits)
PARTING_SUBMODE_IMPORT(Common)
PARTING_SUBMODE_IMPORT(Format)
PARTING_SUBMODE_IMPORT(Heap)

#else
#pragma once

#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"
//Global
#include "VulkanWrapper.h"

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Algorithm/Module/Algorithm.h"
#include "Core/Container/Module/Container.h"
#include "Core/String/Module/String.h"
#include "Core/Color/Module/Color.h"
#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI.h"
#include "RHI/Module/StateTracking.h"

#include "VulkanRHI/Module/VulkanRHI-Traits.h"
#include "VulkanRHI/Module/VulkanRHI-Common.h"
#include "VulkanRHI/Module/VulkanRHI-Format.h"
#include "VulkanRHI/Module/VulkanRHI-Heap.h"

#endif // PARTING_MODULE_BUILD

namespace RHI::Vulkan {
	// A copyable version of std::atomic to be used in an std::vector
	class BufferVersionItem final : public Atomic<Uint64> {  // NOLINT(cppcoreguidelines-special-member-functions)
	public:
		BufferVersionItem(void) : Atomic<Uint64>{ 0 } {}
		BufferVersionItem(Uint64 value) :Atomic<Uint64>{ value } {}
		BufferVersionItem(const BufferVersionItem& other) { store(other); }

		~BufferVersionItem(void) = default;

		BufferVersionItem& operator=(const Uint64 a) { store(a); return *this; }
	};

	class Buffer final : public RHIBuffer<Buffer>, public MemoryResource {
		friend class RHIResource<Buffer>;
		friend class RHIBuffer<Buffer>;

		friend class CommandList;
		friend class Device;
	public:
		Buffer(const Context& context, VulkanAllocator& allocator, RHIBufferDesc desc) :
			RHIBuffer<Buffer>{},
			MemoryResource{},
			m_Context{ context },
			m_Allocator{ allocator },
			m_Desc{ desc }
		{
		}

		~Buffer(void);

	private:
		const Context& m_Context;
		VulkanAllocator& m_Allocator;

		RHIBufferDesc m_Desc;
		RHIBufferStateExtension<VulkanTag> m_StateExtension{ .DescRef{ this->m_Desc }, .ParentBuffer{ this } };

		RefCountPtr<Heap> m_Heap;

		vk::Buffer m_Buffer;

		UnorderedMap<Uint64, vk::BufferView> m_ViewCache;

		Vector<BufferVersionItem> m_VersionTracking;
		void* m_MappedMemory{ nullptr };
		Uint32 m_VersionSearchStart{ 0 };

		// For staging buffers only
		RHICommandQueue m_LastUseQueue{ RHICommandQueue::Graphics };
		Uint64 m_LastUseCommandListID{ 0 };

	private:
		RHIObject Imp_GetNativeObject(RHIObjectType type)const noexcept;

		const RHIBufferDesc& Imp_Get_Desc(void) const { return this->m_Desc; }
	};

	vk::MemoryPropertyFlags PickBufferMemoryProperties(const RHIBufferDesc& d) {
		vk::MemoryPropertyFlags flags{};

		switch (d.CPUAccess) {
		case RHICPUAccessMode::None:
			flags = vk::MemoryPropertyFlagBits::eDeviceLocal;
			break;

		case RHICPUAccessMode::Read:
			flags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCached;
			break;

		case RHICPUAccessMode::Write:
			flags = vk::MemoryPropertyFlagBits::eHostVisible;
			break;
		}

		return flags;
	}

	Buffer::~Buffer(void) {
		if (nullptr != this->m_MappedMemory) {
			this->m_Context.Device.unmapMemory(this->MemoryResource::Memory);
			this->m_MappedMemory = nullptr;
		}

		for (auto&& iter : this->m_ViewCache)
			this->m_Context.Device.destroyBufferView(iter.second, this->m_Context.AllocationCallbacks);
		this->m_ViewCache.clear();

		if (this->MemoryResource::Managed) {
			ASSERT(nullptr != this->m_Buffer);

			this->m_Context.Device.destroyBuffer(this->m_Buffer, this->m_Context.AllocationCallbacks);
			this->m_Buffer = nullptr;

			if (nullptr != this->MemoryResource::Memory) {
				this->m_Allocator.FreeMemory(static_cast<MemoryResource*>(this));
				this->MemoryResource::Memory = nullptr;
			}
		}
	}

	RHIObject Buffer::Imp_GetNativeObject(RHIObjectType objectType)const noexcept {
		switch (objectType) {
			using enum RHIObjectType;
		case VK_Buffer:return RHIObject{ .Pointer{this->m_Buffer} };
		case VK_DeviceMemory:return RHIObject{ .Pointer{ this->MemoryResource::Memory } };
		default:return RHIObject{};
		}
	}
}