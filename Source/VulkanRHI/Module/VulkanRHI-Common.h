#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "D3D12RHI/Include/DirectXMacros.h"

#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"



PARTING_SUBMODULE(D3D12RHI, Common)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Concurrent;
PARTING_IMPORT Algorithm;
PARTING_IMPORT Container;
PARTING_IMPORT String;
PARTING_IMPORT Logger;

PARTING_IMPORT RHI;

PARTING_SUBMODE_IMPORT(Traits)

#else
#pragma once

#include <fstream>

#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"
//Global
#include "VulkanWrapper.h"

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Concurrent/Module/Concurrent.h"
#include "Core/Algorithm/Module/Algorithm.h"
#include "Core/Container/Module/Container.h"
#include "Core/String/Module/String.h"
#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI.h"

#include "VulkanRHI/Module/VulkanRHI-Traits.h"

#endif // PARTING_MODULE_BUILD


namespace RHI::Vulkan {
	constexpr vk::SampleCountFlagBits ConvertSampleCount(Uint32 sampleCount) {
		switch (sampleCount) {
		case 1:return vk::SampleCountFlagBits::e1;
		case 2:return vk::SampleCountFlagBits::e2;
		case 4:return vk::SampleCountFlagBits::e4;
		case 8:return vk::SampleCountFlagBits::e8;
		case 16:return vk::SampleCountFlagBits::e16;
		case 32:return vk::SampleCountFlagBits::e32;
		case 64:return vk::SampleCountFlagBits::e64;
		default:ASSERT(false); return vk::SampleCountFlagBits::e1;
		}
	}

	struct ResourceStateMapping final {
		RHIResourceState RHIState;
		vk::PipelineStageFlags StageFlags;
		vk::AccessFlags AccessMask;
		vk::ImageLayout ImageLayout;
	};

	struct ResourceStateMapping2 final {// for use with KHR_synchronization2
		RHIResourceState RHIState;
		vk::PipelineStageFlags2 StageFlags;
		vk::AccessFlags2 AccessMask;
		vk::ImageLayout ImageLayout;
	};

	struct ResourceStateMappingInternal final {
		RHIResourceState RHIState;
		vk::PipelineStageFlags2 StageFlags;
		vk::AccessFlags2 AccessMask;
		vk::ImageLayout ImageLayout;

		ResourceStateMapping AsResourceStateMapping(void) const {
			// It's safe to cast vk::AccessFlags2 -> vk::AccessFlags and vk::PipelineStageFlags2 -> vk::PipelineStageFlags (as long as the enum exist in both versions!),
			// synchronization2 spec says: "The new flags are identical to the old values within the 32-bit range, with new stages and bits beyond that."
			// The below stages are exclustive to synchronization2
			ASSERT((this->StageFlags & vk::PipelineStageFlagBits2::eMicromapBuildEXT) != vk::PipelineStageFlagBits2::eMicromapBuildEXT);
			ASSERT((this->AccessMask & vk::AccessFlagBits2::eMicromapWriteEXT) != vk::AccessFlagBits2::eMicromapWriteEXT);
			return ResourceStateMapping{
				.RHIState{ this->RHIState },
				.StageFlags{ reinterpret_cast<const vk::PipelineStageFlags&>(this->StageFlags) },
				.AccessMask{ reinterpret_cast<const vk::AccessFlags&>(this->AccessMask) },
				.ImageLayout{ this->ImageLayout }
			};
		}

		constexpr ResourceStateMapping2 AsResourceStateMapping2(void) const {
			return ResourceStateMapping2{
				.RHIState{ this->RHIState },
				.StageFlags{ this->StageFlags },
				.AccessMask{ this->AccessMask },
				.ImageLayout{ this->ImageLayout }
			};
		}
	};

	constexpr Array<ResourceStateMappingInternal, 16> g_ResourceStateMap{
		ResourceStateMappingInternal{ RHIResourceState::Common,				vk::PipelineStageFlagBits2::eTopOfPipe,																vk::AccessFlagBits2{},																					vk::ImageLayout::eUndefined},
		ResourceStateMappingInternal{ RHIResourceState::ConstantBuffer,		vk::PipelineStageFlagBits2::eAllCommands,															vk::AccessFlagBits2::eUniformRead,																		vk::ImageLayout::eUndefined },
		ResourceStateMappingInternal{ RHIResourceState::VertexBuffer,		vk::PipelineStageFlagBits2::eVertexInput,															vk::AccessFlagBits2::eVertexAttributeRead,																vk::ImageLayout::eUndefined },
		ResourceStateMappingInternal{ RHIResourceState::IndexBuffer,		vk::PipelineStageFlagBits2::eVertexInput,															vk::AccessFlagBits2::eIndexRead,																		vk::ImageLayout::eUndefined },
		ResourceStateMappingInternal{ RHIResourceState::IndirectArgument,	vk::PipelineStageFlagBits2::eDrawIndirect,															vk::AccessFlagBits2::eIndirectCommandRead,																vk::ImageLayout::eUndefined },
		ResourceStateMappingInternal{ RHIResourceState::ShaderResource,		vk::PipelineStageFlagBits2::eAllCommands,															vk::AccessFlagBits2::eShaderRead,																		vk::ImageLayout::eShaderReadOnlyOptimal },
		ResourceStateMappingInternal{ RHIResourceState::UnorderedAccess,	vk::PipelineStageFlagBits2::eAllCommands,															vk::AccessFlagBits2::eShaderRead | vk::AccessFlagBits2::eShaderWrite,									vk::ImageLayout::eGeneral },
		ResourceStateMappingInternal{ RHIResourceState::RenderTarget,		vk::PipelineStageFlagBits2::eColorAttachmentOutput,													vk::AccessFlagBits2::eColorAttachmentRead | vk::AccessFlagBits2::eColorAttachmentWrite,					vk::ImageLayout::eColorAttachmentOptimal },
		ResourceStateMappingInternal{ RHIResourceState::DepthWrite,			vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,	vk::AccessFlagBits2::eDepthStencilAttachmentRead | vk::AccessFlagBits2::eDepthStencilAttachmentWrite,	vk::ImageLayout::eDepthStencilAttachmentOptimal },
		ResourceStateMappingInternal{ RHIResourceState::DepthRead,			vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,	vk::AccessFlagBits2::eDepthStencilAttachmentRead,														vk::ImageLayout::eDepthStencilReadOnlyOptimal },
		ResourceStateMappingInternal{ RHIResourceState::StreamOut,			vk::PipelineStageFlagBits2::eTransformFeedbackEXT,													vk::AccessFlagBits2::eTransformFeedbackWriteEXT,														vk::ImageLayout::eUndefined },
		ResourceStateMappingInternal{ RHIResourceState::CopyDest,			vk::PipelineStageFlagBits2::eTransfer,																vk::AccessFlagBits2::eTransferWrite,																	vk::ImageLayout::eTransferDstOptimal },
		ResourceStateMappingInternal{ RHIResourceState::CopySource,			vk::PipelineStageFlagBits2::eTransfer,																vk::AccessFlagBits2::eTransferRead,																		vk::ImageLayout::eTransferSrcOptimal },
		ResourceStateMappingInternal{ RHIResourceState::ResolveDest,		vk::PipelineStageFlagBits2::eTransfer,																vk::AccessFlagBits2::eTransferWrite,																	vk::ImageLayout::eTransferDstOptimal },
		ResourceStateMappingInternal{ RHIResourceState::ResolveSource,		vk::PipelineStageFlagBits2::eTransfer,																vk::AccessFlagBits2::eTransferRead,																		vk::ImageLayout::eTransferSrcOptimal },
		ResourceStateMappingInternal{ RHIResourceState::Present,			vk::PipelineStageFlagBits2::eAllCommands,															vk::AccessFlagBits2::eMemoryRead,																		vk::ImageLayout::ePresentSrcKHR }
	};

	ResourceStateMappingInternal ConvertResourceStateInternal(RHIResourceState state) {
		ResourceStateMappingInternal result{};

		constexpr Uint32 numStateBits{ static_cast<Uint32>(g_ResourceStateMap.size()) };

		auto stateTmp{ Tounderlying(state) };
		Uint32 bitIndex{ 0 };

		while (stateTmp != 0 && bitIndex < numStateBits) {
			Uint32 bit{ (1u << bitIndex) };

			if (stateTmp & bit) {
				const ResourceStateMappingInternal& mapping{ g_ResourceStateMap[bitIndex] };

				ASSERT(Tounderlying(mapping.RHIState) == bit);
				ASSERT(result.ImageLayout == vk::ImageLayout::eUndefined || mapping.ImageLayout == vk::ImageLayout::eUndefined || result.ImageLayout == mapping.ImageLayout);

				result.RHIState |= mapping.RHIState;
				result.AccessMask |= mapping.AccessMask;
				result.StageFlags |= mapping.StageFlags;
				if (mapping.ImageLayout != vk::ImageLayout::eUndefined)
					result.ImageLayout = mapping.ImageLayout;

				stateTmp &= ~bit;
			}

			++bitIndex;
		}

		ASSERT(result.RHIState == state);

		return result;
	}

	ResourceStateMapping ConvertResourceState(RHIResourceState state) { return ConvertResourceStateInternal(state).AsResourceStateMapping(); }

	ResourceStateMapping2 ConvertResourceState2(RHIResourceState state) { return ConvertResourceStateInternal(state).AsResourceStateMapping2(); }

	//Vulkan Begin

	struct DeviceDesc final {
		VkInstance Instance;
		VkPhysicalDevice PhysicalDevice;
		VkDevice Device;

		// any of the queues can be null if this context doesn't intend to use them
		VkQueue GraphicsQueue;
		Uint32 GraphicsQueueIndex{ Max_Uint32 };
		VkQueue TransferQueue;
		Uint32 TransferQueueIndex{ Max_Uint32 };
		VkQueue ComputeQueue;
		Uint32 ComputeQueueIndex{ Max_Uint32 };

		VkAllocationCallbacks* AllocationCallbacks{ nullptr };

		const char** InstanceExtensions{ nullptr };
		Uint64 NumInstanceExtensions{ 0 };

		const char** DeviceExtensions{ nullptr };
		Uint64 NumDeviceExtensions{ 0 };

		Uint32 MaxTimerQueries{ 256 };//TODO :
	};

	struct Context final {
		vk::Instance Instance;
		vk::PhysicalDevice PhysicalDevice;
		vk::Device Device;

		vk::AllocationCallbacks* AllocationCallbacks;

		vk::PipelineCache PipelineCache;

		struct {
			bool KHR_maintenance1{ false };
			bool EXT_debug_report{ false };
			bool EXT_debug_marker{ false };
			bool EXT_debug_utils{ false };
			bool KHR_synchronization2{ false };
			bool EXT_conservative_rasterization{ false };
			bool NV_mesh_shader{ false };
		} Extensions;

		vk::PhysicalDeviceProperties PhysicalDeviceProperties;
		vk::PhysicalDeviceConservativeRasterizationPropertiesEXT ConservativeRasterizationProperties;
		vk::DescriptorSetLayout EmptyDescriptorSetLayout;

		void NameVKObject(const void* handle, const vk::ObjectType objtype, const vk::DebugReportObjectTypeEXT objtypeEXT, const char* name) const {
			if (!(nullptr != name && *name && nullptr != handle))
				return;

			if (this->Extensions.EXT_debug_utils) {
				auto info{ vk::DebugUtilsObjectNameInfoEXT{}
					.setObjectType(objtype)
					.setObjectHandle(reinterpret_cast<Uint64>(handle))
					.setPObjectName(name)
				};
				this->Device.setDebugUtilsObjectNameEXT(info);
			}
			else if (this->Extensions.EXT_debug_marker) {
				auto info{ vk::DebugMarkerObjectNameInfoEXT{}
					.setObjectType(objtypeEXT)
					.setObject(reinterpret_cast<Uint64>(handle))
					.setPObjectName(name)
				};

				VULKAN_CHECK(this->Device.debugMarkerSetObjectNameEXT(&info));
			}
		}
	};

	class VulkanQueue final {
		friend class CommandList;
		friend class Device;

	public:
		VulkanQueue(const Context& context, RHICommandQueue queueID, vk::Queue queue, Uint32 queueFamilyIndex);

		~VulkanQueue(void);

	public:
		void AddWaitSemaphore(vk::Semaphore semaphore, Uint64 value);

		void AddSignalSemaphore(vk::Semaphore semaphore, Uint64 value);

		Uint64 UpdateLastFinishedID(void);

		Uint64 Get_LastSubmittedID(void) const { return this->m_LastSubmittedID; }

		Uint64 Get_LastFinishedID(void) const { return this->m_LastFinishedID; }

		RHICommandQueue Get_QueueID(void) const { return this->m_QueueID; }

		vk::Queue Get_VkQueue(void) const { return this->m_Queue; }

		VkSemaphore Get_Semaphore(void) const { return this->m_TrackingSemaphore; }

		bool PollCommandList(Uint64 commandListID);

		bool WaitCommandList(Uint64 commandListID, Uint64 timeout);

	private:
		const Context& m_Context;

		vk::Queue m_Queue;
		RHICommandQueue m_QueueID;
		Uint32 m_QueueFamilyIndex{ Max_Uint32 };

		Mutex m_Mutex;
		vk::Semaphore m_TrackingSemaphore;
		Vector<vk::Semaphore> m_WaitSemaphores;
		Vector<vk::Semaphore> m_SignalSemaphores;

		Vector<Uint64> m_WaitSemaphoreValues;
		Vector<Uint64> m_SignalSemaphoreValues;

		Uint64 m_LastRecordingID{ 0 };
		Uint64 m_LastSubmittedID{ 0 };
		Uint64 m_LastFinishedID{ 0 };
	};

	inline Uint64 VulkanQueue::UpdateLastFinishedID(void) {
		return this->m_LastFinishedID = this->m_Context.Device.getSemaphoreCounterValue(this->m_TrackingSemaphore);
	}

	inline bool VulkanQueue::PollCommandList(Uint64 commandListID) {
		if (commandListID > this->m_LastSubmittedID || commandListID == 0)
			return false;

		if (this->m_LastFinishedID >= commandListID)
			return true;

		return this->UpdateLastFinishedID() >= commandListID;
	}

	bool VulkanQueue::WaitCommandList(Uint64 commandListID, Uint64 timeout) {
		if (commandListID > this->m_LastSubmittedID || commandListID == 0)
			return false;

		if (this->PollCommandList(commandListID))
			return true;

		Array<const vk::Semaphore, 1> semaphores{ this->m_TrackingSemaphore };
		Array<Uint64, 1> waitValues{ commandListID };

		auto waitInfo{ vk::SemaphoreWaitInfo{}
			.setSemaphores(semaphores)
			.setValues(waitValues)
		};

		return 	vk::Result::eSuccess == this->m_Context.Device.waitSemaphores(waitInfo, timeout);
	}

	inline VulkanQueue::VulkanQueue(const Context& context, RHICommandQueue queueID, vk::Queue queue, Uint32 queueFamilyIndex) :
		m_Context{ context },
		m_Queue{ queue },
		m_QueueID{ queueID },
		m_QueueFamilyIndex{ queueFamilyIndex } {
		auto semaphoreTypeInfo{ vk::SemaphoreTypeCreateInfo{}
			.setSemaphoreType(vk::SemaphoreType::eTimeline)
		};

		auto semaphoreInfo{ vk::SemaphoreCreateInfo{}
			.setPNext(&semaphoreTypeInfo)
		};

		this->m_TrackingSemaphore = context.Device.createSemaphore(semaphoreInfo, context.AllocationCallbacks);
	}

	inline VulkanQueue::~VulkanQueue(void) {
		this->m_Context.Device.destroySemaphore(this->m_TrackingSemaphore, m_Context.AllocationCallbacks);
		this->m_TrackingSemaphore = nullptr;
	}

	inline void VulkanQueue::AddWaitSemaphore(vk::Semaphore semaphore, Uint64 value) {
		ASSERT(nullptr != semaphore);

		this->m_WaitSemaphores.push_back(semaphore);
		this->m_WaitSemaphoreValues.push_back(value);
	}

	inline void VulkanQueue::AddSignalSemaphore(vk::Semaphore semaphore, Uint64 value) {
		ASSERT(nullptr != semaphore);

		this->m_SignalSemaphores.push_back(semaphore);
		this->m_SignalSemaphoreValues.push_back(value);
	}

	struct MemoryResource {
		bool Managed{ true };
		vk::DeviceMemory Memory;
	};

	class VulkanAllocator final {
	public:
		explicit VulkanAllocator(const Context& context) : m_Context{ context } {}

		~VulkanAllocator(void) = default;

	public:
		void AllocateMemory(
			MemoryResource* res,
			vk::MemoryRequirements memRequirements,
			vk::MemoryPropertyFlags memPropertyFlags
		) const;

		void FreeMemory(MemoryResource* res) const;

	private:
		const Context& m_Context;
	};

	inline void VulkanAllocator::AllocateMemory(MemoryResource* res, vk::MemoryRequirements memRequirements, vk::MemoryPropertyFlags memPropertyFlags) const {
		res->Managed = true;

		// find a memory space that satisfies the requirements
		vk::PhysicalDeviceMemoryProperties memProperties;
		this->m_Context.PhysicalDevice.getMemoryProperties(&memProperties);

		Uint32 memTypeIndex;
		for (memTypeIndex = 0; memTypeIndex < memProperties.memoryTypeCount; ++memTypeIndex)
			if ((memRequirements.memoryTypeBits & (1 << memTypeIndex)) &&
				((memProperties.memoryTypes[memTypeIndex].propertyFlags & memPropertyFlags) == memPropertyFlags))
				break;

		if (memTypeIndex == memProperties.memoryTypeCount)
			LOG_ERROR("Failed to find a memory type that satisfies the requirements for allocation of size {0}", memRequirements.size);

		auto allocInfo{ vk::MemoryAllocateInfo{}
			.setAllocationSize(memRequirements.size)
			.setMemoryTypeIndex(memTypeIndex)
		};

		VULKAN_CHECK(this->m_Context.Device.allocateMemory(&allocInfo, this->m_Context.AllocationCallbacks, &res->Memory));
	}

	void VulkanAllocator::FreeMemory(MemoryResource* res) const {
		ASSERT(res->Managed);

		this->m_Context.Device.freeMemory(res->Memory, this->m_Context.AllocationCallbacks);
		res->Memory = nullptr;
	}

	//Vulan End

	class EventQuery final : public RHIEventQuery<EventQuery> {
		friend class RHIResource<EventQuery>;
		friend class RHIEventQuery<EventQuery>;

		friend class CommandList;
		friend class Device;
	public:
		EventQuery(void) = default;
		~EventQuery(void) = default;

	private:
		RHICommandQueue m_Queue{ RHICommandQueue::Graphics };
		Uint64 m_CommandListID{ 0 };

	private:
		RHIObject Imp_GetNativeObject(RHIObjectType)const noexcept { LOG_ERROR("Imp But Empty");  return RHIObject{}; };
	};

	//TODO :
	class TimerQuery final : public RHITimerQuery<TimerQuery> {
		friend class RHIResource<TimerQuery>;
		friend class RHITimerQuery<TimerQuery>;

		friend class CommandList;
	public:
		explicit TimerQuery(BitSetAllocator& allocator) :
			RHITimerQuery<TimerQuery>{},
			m_QueryAllocator{ allocator }{
		}

		~TimerQuery(void) {
			this->m_QueryAllocator.Release(this->m_BeginQueryIndex / 2);
			this->m_BeginQueryIndex = -1;
			this->m_EndQueryIndex = -1;
		}

	private:
		BitSetAllocator& m_QueryAllocator;

		Int32 m_BeginQueryIndex = -1;
		Int32 m_EndQueryIndex = -1;

		bool m_Started{ false };
		bool m_Resolved{ false };
		float m_Time{ 0.f };

		vk::QueryPool m_TimerQueryPool{ nullptr };//TODO : 

	private:
		RHIObject Imp_GetNativeObject(RHIObjectType)const noexcept { LOG_ERROR("Imp But Empty");  return RHIObject{}; };
	};
}