#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"


PARTING_SUBMODULE(D3D12RHI, Pipeline)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Algorithm;
PARTING_IMPORT Container;
PARTING_IMPORT String;
PARTING_IMPORT Logger;

PARTING_IMPORT RHI;

PARTING_SUBMODE_IMPORT(Traits)
PARTING_SUBMODE_IMPORT(Common)
PARTING_SUBMODE_IMPORT(Format)
PARTING_SUBMODE_IMPORT(Heap)
PARTING_SUBMODE_IMPORT(Buffer)
PARTING_SUBMODE_IMPORT(Texture)
PARTING_SUBMODE_IMPORT(Sampler)
PARTING_SUBMODE_IMPORT(InputLayout)
PARTING_SUBMODE_IMPORT(Shader)
PARTING_SUBMODE_IMPORT(BlendState)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(RasterState)
PARTING_SUBMODE_IMPORT(DepthStencilState)
PARTING_SUBMODE_IMPORT(ViewportState)
PARTING_SUBMODE_IMPORT(FrameBuffer)
PARTING_SUBMODE_IMPORT(ShaderBinding)
PARTING_SUBMODE_IMPORT(Pipeline)

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
#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI.h"
#include "RHI/Module/StateTracking.h"

#include "VulkanRHI/Module/VulkanRHI-Traits.h"
#include "VulkanRHI/Module/VulkanRHI-Common.h"
#include "VulkanRHI/Module/VulkanRHI-Format.h"
#include "VulkanRHI/Module/VulkanRHI-Heap.h"
#include "VulkanRHI/Module/VulkanRHI-Buffer.h"
#include "VulkanRHI/Module/VulkanRHI-Texture.h"
#include "VulkanRHI/Module/VulkanRHI-Sampler.h"
#include "VulkanRHI/Module/VulkanRHI-InputLayout.h"
#include "VulkanRHI/Module/VulkanRHI-Shader.h"
#include "VulkanRHI/Module/VulkanRHI-BlendState.h"
#include "VulkanRHI/Module/VulkanRHI-RasterState.h"
#include "VulkanRHI/Module/VulkanRHI-DepthStencilState.h"
#include "VulkanRHI/Module/VulkanRHI-ViewportState.h"
#include "VulkanRHI/Module/VulkanRHI-FrameBuffer.h"
#include "VulkanRHI/Module/VulkanRHI-ShaderBinding.h"
#include "VulkanRHI/Module/VulkanRHI-Pipeline.h"

#endif // PARTING_MODULE_BUILD

namespace RHI::Vulkan {
	class BufferChunk final {
		friend class UploadManager;
	public:
		explicit BufferChunk(const Context& context) :
			m_Context{ context } {
		}

		~BufferChunk(void) {
			if (nullptr != this->m_MappedMemory) {
				this->m_Context.Device.unmapMemory(this->m_Memory);
				this->m_MappedMemory = nullptr;
			}

			if (nullptr != this->m_Buffer) {
				this->m_Context.Device.destroyBuffer(this->m_Buffer, this->m_Context.AllocationCallbacks);
				this->m_Buffer = nullptr;
			}

			if (nullptr != this->m_Memory) {
				this->m_Context.Device.freeMemory(this->m_Memory, this->m_Context.AllocationCallbacks);
				this->m_Memory = nullptr;
			}
		}

	public:
		static constexpr Uint64 c_SizeAlignment{ 4096 }; // GPU page size

	private:
		const Context& m_Context;
		vk::Buffer m_Buffer;
		vk::DeviceMemory m_Memory;
		void* m_MappedMemory{ nullptr };

		Uint64 m_Version{ 0 };
		Uint64 m_BufferSize{ 0 };
		Uint64 m_WritePointer{ 0 };
	};

	class UploadManager final {
	public:
		UploadManager(const Context& context, VulkanQueue* queue, Uint64 defaultChunkSize) :
			m_Context{ context },
			m_Queue{ queue },
			m_DefaultChunkSize{ defaultChunkSize } {
		}

		~UploadManager(void) = default;

	public:
		SharedPtr<BufferChunk> CreateChunk(Uint64 size);

		bool SuballocateBuffer(Uint64 size, vk::Buffer** pBuffer, Uint64* pOffset, void** pCpuVA, Uint64 currentVersion, Uint32 alignment = 256);

		void SubmitChunks(Uint64 currentVersion, Uint64 submittedVersion);

	private:
		const Context& m_Context;
		VulkanQueue* m_Queue{ nullptr };

		Uint64 m_DefaultChunkSize{ 0 };
		Uint64 m_AllocatedMemory{ 0 };

		List<SharedPtr<BufferChunk>> m_ChunkPool;
		SharedPtr<BufferChunk> m_CurrentChunk;
	};

	SharedPtr<BufferChunk> UploadManager::CreateChunk(Uint64 size) {
		SharedPtr<BufferChunk> chunk{ MakeShared<BufferChunk>(this->m_Context) };
		chunk->m_BufferSize = size;

		vk::BufferUsageFlags usageFlags{ vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst };

		auto bufferInfo{ vk::BufferCreateInfo{}
			.setSize(size)
			.setUsage(usageFlags)
			.setSharingMode(vk::SharingMode::eExclusive)
		};

		VULKAN_CHECK(this->m_Context.Device.createBuffer(&bufferInfo, this->m_Context.AllocationCallbacks, &chunk->m_Buffer));
		//TODO this->m_Context.NameVKObject(VkBuffer{ chunk->Buffer }, vk::ObjectType::eBuffer, vk::DebugReportObjectTypeEXT::eBuffer, _W("Load Buffer"));

		vk::MemoryRequirements memRequirements;
		this->m_Context.Device.getBufferMemoryRequirements(chunk->m_Buffer, &memRequirements);

		// find a memory space that satisfies the requirements
		vk::PhysicalDeviceMemoryProperties memProperties;
		this->m_Context.PhysicalDevice.getMemoryProperties(&memProperties);

		Uint32 memTypeIndex;
		for (memTypeIndex = 0; memTypeIndex < memProperties.memoryTypeCount; ++memTypeIndex)
			if ((memRequirements.memoryTypeBits & (1 << memTypeIndex)) &&
				((memProperties.memoryTypes[memTypeIndex].propertyFlags & vk::MemoryPropertyFlagBits::eHostVisible) == vk::MemoryPropertyFlagBits::eHostVisible))
				break;

		if (memTypeIndex == memProperties.memoryTypeCount)
			LOG_ERROR("Failed to find a memory type that satisfies the requirements for allocation of size {0}", memRequirements.size);

		auto allocInfo{ vk::MemoryAllocateInfo{}
			.setAllocationSize(memRequirements.size)
			.setMemoryTypeIndex(memTypeIndex)
		};

		VULKAN_CHECK(this->m_Context.Device.allocateMemory(&allocInfo, m_Context.AllocationCallbacks, &chunk->m_Memory));

		this->m_Context.Device.bindBufferMemory(chunk->m_Buffer, chunk->m_Memory, 0);

		VULKAN_CHECK(this->m_Context.Device.mapMemory(chunk->m_Memory, 0, size, vk::MemoryMapFlags{}, &chunk->m_MappedMemory));

		return chunk;
	}

	class TrackedCommandBuffer final {
	public:
		using CommandResource = Variant<
			RefCountPtr<Buffer>,
			RefCountPtr<Texture>,
			RefCountPtr<Sampler>,
			RefCountPtr<FrameBuffer>,
			RefCountPtr<BindingLayout>,
			RefCountPtr<BindingSet>,
			RefCountPtr<GraphicsPipeline>,
			RefCountPtr<ComputePipeline>,
			RefCountPtr<MeshletPipeline>,
			RefCountPtr<CommandList>,

			Nullptr_T
		>;

		explicit TrackedCommandBuffer(const Context& context) : m_Context{ context } {}

		~TrackedCommandBuffer(void) {
			if (nullptr != this->CmdPool) {
				this->m_Context.Device.destroyCommandPool(this->CmdPool, this->m_Context.AllocationCallbacks);
				this->CmdPool = nullptr;
			}
		}

		const Context& m_Context;

		// the command buffer itself
		vk::CommandBuffer CmdBuf;
		vk::CommandPool CmdPool;

		Vector<CommandResource> ReferencedResources; // to keep them alive
		Vector<RefCountPtr<Buffer>> ReferencedStagingBuffers; // to allow synchronous mapBuffer

		Uint64 RecordingID{ 0 };
		Uint64 SubmissionID{ 0 };
	};

	// tracks the list of command buffers in flight on this queue
	//TODO :itmaybe in Vuqueue module template right 
	Array<List<SharedPtr<TrackedCommandBuffer>>, Tounderlying(RHICommandQueue::Count)> g_CommandBuffersInFlight;
	Array<List<SharedPtr<TrackedCommandBuffer>>, Tounderlying(RHICommandQueue::Count)> g_CommandBuffersPool;

	bool UploadManager::SuballocateBuffer(Uint64 size, vk::Buffer** pBuffer, Uint64* pOffset, void** pCpuVA, Uint64 currentVersion, Uint32 alignment) {
		SharedPtr<BufferChunk> chunkToRetire;

		if (nullptr != this->m_CurrentChunk) {
			Uint64 alignedOffset{ Math::Align(this->m_CurrentChunk->m_WritePointer, static_cast<Uint64>(alignment)) };
			Uint64 endOfDataInChunk{ alignedOffset + size };

			if (endOfDataInChunk <= this->m_CurrentChunk->m_BufferSize) {
				this->m_CurrentChunk->m_WritePointer = endOfDataInChunk;

				*pBuffer = &this->m_CurrentChunk->m_Buffer;
				*pOffset = alignedOffset;
				if (nullptr != pCpuVA && nullptr != this->m_CurrentChunk->m_MappedMemory)
					*pCpuVA = static_cast<char*>(this->m_CurrentChunk->m_MappedMemory) + alignedOffset;

				return true;
			}

			chunkToRetire = this->m_CurrentChunk;
			this->m_CurrentChunk.reset();
		}

		Uint64 completedInstance{ this->m_Context.Device.getSemaphoreCounterValue(this->m_Queue->Get_Semaphore()) };

		for (auto it{ this->m_ChunkPool.begin() }; it != this->m_ChunkPool.end(); ++it) {
			SharedPtr<BufferChunk> chunk = *it;//TODO :

			if (VersionGetSubmitted(chunk->m_Version)
				&& VersionGetInstance(chunk->m_Version) <= completedInstance)
				chunk->m_Version = 0;

			if (chunk->m_Version == 0 && chunk->m_BufferSize >= size) {
				this->m_ChunkPool.erase(it);
				this->m_CurrentChunk = chunk;
				break;
			}
		}

		if (nullptr != chunkToRetire)
			this->m_ChunkPool.push_back(chunkToRetire);

		if (nullptr == this->m_CurrentChunk)
			this->m_CurrentChunk = CreateChunk(Math::Align(Math::Max(size, this->m_DefaultChunkSize), BufferChunk::c_SizeAlignment));

		this->m_CurrentChunk->m_Version = currentVersion;
		this->m_CurrentChunk->m_WritePointer = size;

		*pBuffer = &this->m_CurrentChunk->m_Buffer;
		*pOffset = 0;
		if (nullptr != pCpuVA)
			*pCpuVA = this->m_CurrentChunk->m_MappedMemory;

		return true;
	}

	void UploadManager::SubmitChunks(Uint64 currentVersion, Uint64 submittedVersion) {
		if (nullptr != this->m_CurrentChunk) {
			this->m_ChunkPool.push_back(this->m_CurrentChunk);
			this->m_CurrentChunk.reset();
		}

		for (const auto& chunk : this->m_ChunkPool)
			if (chunk->m_Version == currentVersion)
				chunk->m_Version = submittedVersion;
	}

	class CommandList final : public RHICommandList<CommandList, VulkanTag> {
		friend class RHIResource<CommandList>;
		friend class RHICommandList<CommandList, VulkanTag>;

		friend class Device;
	public:
		struct VolatileBufferState final {
			Uint32 LatestVersion{ 0 };
			Uint32 MinVersion{ 0 };
			Uint32 MaxVersion{ 0 };
			bool Initialized{ false };
		};

	public:
		CommandList(const Context& context, Device* device, VulkanQueue* queue, const RHICommandListParameters& parameters) :
			RHICommandList<CommandList, VulkanTag>{},
			m_Context{ context },
			m_Device{ device },
			m_Queue{ queue },
			m_CommandListParameters{ parameters },
			m_UploadManager{ MakeUnique<decltype(this->m_UploadManager)::element_type>(context, queue, parameters.UploadChunkSize) } {
		}

		~CommandList(void) = default;

	private:
		SharedPtr<TrackedCommandBuffer> Get_CurrentCmdBuf(void) const { return this->m_CurrentCmdBuf; }

		//// creates a command buffer and its synchronization resources
		SharedPtr<TrackedCommandBuffer> CreateCommandBuffer(void);

		SharedPtr<TrackedCommandBuffer> GetOrCreateCommandBuffer(void);

		bool AnyBarriers(void) const { return !this->m_StateTracker.Get_BufferBarriers().empty() || !this->m_StateTracker.Get_TextureBarriers().empty(); }

		void EndRenderPass(void);

		void ClearTexture(Texture* texture, RHITextureSubresourceSet subresources, const vk::ClearColorValue& clearValue);

		void WriteVolatileBuffer(Buffer* buffer, const void* data, Uint64 dataSize);

		void FlushVolatileBufferWrites(void);

		void TrackResourcesAndBarriers(const RHIGraphicsState<VulkanTag>& state);

		void BindBindingSets(vk::PipelineBindPoint bindPoint, vk::PipelineLayout pipelineLayout, Span<const BindingSet* const> bindings, Span<const Uint32> descriptorSetIdxToBindingIdx);

		void UpdateGraphicsVolatileBuffers(void);

		void CommitBarriersInternal(void);

		void CommitBarriersInternal_synchronization2(void);

		void SubmitVolatileBuffers(Uint64 recordingID, Uint64 submittedID);

		void Executed(Uint64 submissionID);
	private:
		const Context& m_Context;
		Device* m_Device;
		VulkanQueue* m_Queue;

		RHICommandListParameters m_CommandListParameters;

		UniquePtr<UploadManager> m_UploadManager;

		RHICommandListResourceStateTracker<VulkanTag> m_StateTracker;

		RHIGraphicsState<VulkanTag> m_CurrentGraphicsState;
		RHIComputeState<VulkanTag> m_CurrentComputeState;
		RHIMeshletState<VulkanTag> m_CurrentMeshletState;
		UnorderedMap<Buffer*, VolatileBufferState> m_VolatileBufferStates;

		// current internal command buffer
		SharedPtr<TrackedCommandBuffer> m_CurrentCmdBuf;

		vk::PipelineLayout m_CurrentPipelineLayout;
		vk::ShaderStageFlags m_CurrentPushConstantsVisibility;

		bool m_EnableAutomaticBarriers{ true };
		bool m_AnyVolatileBufferWrites{ false };

	private:
		RHIObject Imp_GetNativeObject(RHIObjectType objectType)const noexcept {
			switch (objectType) {
				using enum RHIObjectType;
			case VK_CommandBuffer:return RHIObject{ .Pointer { this->m_CurrentCmdBuf->CmdBuf } };
			default:return RHIObject{};
			}
		}

		void Imp_Open(void);
		void Imp_Close(void);
		void Imp_ClearState(void);
		void Imp_ClearTextureFloat(Texture* texture, RHITextureSubresourceSet subresources, const Color& color);
		void Imp_ClearDepthStencilTexture(Texture* texture, RHITextureSubresourceSet subresources, Optional<float> depth, Optional<Uint8> stencil);
		void Imp_ClearTextureUInt(Texture* texture, RHITextureSubresourceSet subresources, Uint32 clearColor);
		void Imp_CopyTexture(Texture* des, RHITextureSlice desSlice, Texture* src, RHITextureSlice srcSlice);
		void Imp_CopyTexture(Texture* des, RHITextureSlice desSlice, StagingTexture* src, RHITextureSlice srcSlice);
		void Imp_CopyTexture(StagingTexture* des, RHITextureSlice desSlice, Texture* src, RHITextureSlice srcSlice);
		void Imp_WriteTexture(Texture* texture, Uint32 ArraySlice, Uint32 MipLevel, const void* data, Uint64 RowPitch, Uint64 DepthPitch);
		void Imp_ResolveTexture(Texture* dest, const RHITextureSubresourceSet& dstSubresources, Texture* src, const RHITextureSubresourceSet& srcSubresources);
		void Imp_WriteBuffer(Buffer* buffer, const void* data, Uint64 dataSize, Uint64 destOffsetBytes);
		void Imp_ClearBufferUInt(Buffer* buffer, Uint32 clearvalue);
		void Imp_CopyBuffer(Buffer* des, Uint64 desOffset, Buffer* src, Uint64 srcOffset, Uint64 dataSizeBytes);
		void Imp_SetPushConstants(const void* data, Uint32 ByteSize);
		void Imp_SetGraphicsState(const RHIGraphicsState<VulkanTag>& pipeline);
		void Imp_Draw(const RHIDrawArguments& args);
		void Imp_DrawIndexed(const RHIDrawArguments& args);
		void Imp_DrawIndirect(Uint32 OffsetBytes, Uint32 Count);
		void Imp_DrawIndexedIndirect(Uint32 OffsetBytes, Uint32 Count);
		void Imp_SetComputeState(const RHIComputeState<VulkanTag>& pipeline);
		void Imp_Dispatch(Uint32 GroupsX, Uint32 GroupsY, Uint32 GroupsZ);
		void Imp_DispatchIndirect(Uint32 OffsetBytes);
		void Imp_SetMeshletState(const RHIMeshletState<VulkanTag>& pipeline);
		void Imp_DispatchMesh(Uint32 GroupsX, Uint32 GroupsY, Uint32 GroupsZ);
		void Imp_BeginTimerQuery(TimerQuery* query);
		void Imp_EndTimerQuery(TimerQuery* query);
		void Imp_BeginMarker(const char* Name);
		void Imp_EndMarker(void);
		void Imp_SetEnableAutomaticBarriers(bool enable);
		void Imp_SetResourceStatesForBindingSet(BindingSet* bindingSet);
		void Imp_SetEnableUAVBarriersForTexture(Texture* texture, bool enable);
		void Imp_SetEnableUAVBarriersForBuffer(Buffer* buffer, bool enable);
		void Imp_BeginTrackingTextureState(Texture* texture, RHITextureSubresourceSet subresources, RHIResourceState state);
		void Imp_BeginTrackingBufferState(Buffer* buffer, RHIResourceState state);
		void Imp_SetTextureState(Texture* texture, RHITextureSubresourceSet subresources, RHIResourceState state);
		void Imp_SetBufferState(Buffer* buffer, RHIResourceState state);
		void Imp_SetPermanentTextureState(Texture* texture, RHIResourceState state);
		void Imp_SetPermanentBufferState(Buffer* buffer, RHIResourceState state);
		void Imp_CommitBarriers(void);
		RHIResourceState Imp_Get_TextureSubresourceState(Texture* texture, Uint32 arraySlice, Uint32 mipLevel);
		RHIResourceState Imp_Get_BufferState(Buffer* buffer);
		Device* Imp_Get_Device(void);
		const RHICommandListParameters& Imp_Get_Desc(void);
	};

	//Misc

	Tuple<Uint32, Uint32, Uint32> ComputeMipLevelInformation(const RHITextureDesc& desc, Uint32 mipLevel) {
		return MakeTuple(
			Math::Max(desc.Extent.Width >> mipLevel, 1u),
			Math::Max(desc.Extent.Height >> mipLevel, 1u),
			Math::Max(desc.Extent.Depth >> mipLevel, 1u)
		);
	}

	//Src
	SharedPtr<TrackedCommandBuffer> CommandList::CreateCommandBuffer(void) {
		SharedPtr<TrackedCommandBuffer> ret{ MakeShared<TrackedCommandBuffer>(this->m_Context) };

		auto cmdPoolInfo{ vk::CommandPoolCreateInfo{}
			.setQueueFamilyIndex(this->m_Queue->m_QueueFamilyIndex)
			.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer | vk::CommandPoolCreateFlagBits::eTransient)
		};
		VULKAN_CHECK(this->m_Context.Device.createCommandPool(&cmdPoolInfo, this->m_Context.AllocationCallbacks, &ret->CmdPool));

		// allocate command buffer
		auto allocInfo{ vk::CommandBufferAllocateInfo{}
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandPool(ret->CmdPool)
			.setCommandBufferCount(1)
		};
		VULKAN_CHECK(this->m_Context.Device.allocateCommandBuffers(&allocInfo, &ret->CmdBuf));

		return ret;
	}

	inline SharedPtr<TrackedCommandBuffer> CommandList::GetOrCreateCommandBuffer(void) {
		LockGuard lockGuard{ this->m_Queue->m_Mutex }; // this is called from CommandList::open, so free-threaded

		SharedPtr<TrackedCommandBuffer> cmdBuf;
		if (g_CommandBuffersPool[Tounderlying(this->m_Queue->m_QueueID)].empty())
			cmdBuf = this->CreateCommandBuffer();
		else {
			cmdBuf = g_CommandBuffersPool[Tounderlying(this->m_Queue->m_QueueID)].front();
			g_CommandBuffersPool[Tounderlying(this->m_Queue->m_QueueID)].pop_front();
		}

		cmdBuf->RecordingID = ++this->m_Queue->m_LastRecordingID;
		return cmdBuf;
	}

	void CommandList::EndRenderPass(void) {
		if (nullptr != this->m_CurrentGraphicsState.FrameBuffer || nullptr != this->m_CurrentMeshletState.FrameBuffer) {
			this->m_CurrentCmdBuf->CmdBuf.endRenderPass();
			this->m_CurrentGraphicsState.FrameBuffer = nullptr;
			this->m_CurrentMeshletState.FrameBuffer = nullptr;
		}
	}

	inline void CommandList::ClearTexture(Texture* texture, RHITextureSubresourceSet subresources, const vk::ClearColorValue& clearValue) {
		this->EndRenderPass();

		ASSERT(nullptr != texture);
		ASSERT(nullptr != this->m_CurrentCmdBuf);

		subresources = subresources.Resolve(texture->m_Desc, false);

		if (this->m_EnableAutomaticBarriers)
			this->m_StateTracker.RequireTextureState(&texture->m_StateExtension, subresources, RHIResourceState::CopyDest);
		this->CommitBarriers();

		vk::ImageSubresourceRange subresourceRange{ vk::ImageSubresourceRange{}
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseArrayLayer(subresources.BaseArraySlice)
			.setLayerCount(subresources.ArraySliceCount)
			.setBaseMipLevel(subresources.BaseMipLevel)
			.setLevelCount(subresources.MipLevelCount)
		};

		this->m_CurrentCmdBuf->CmdBuf.clearColorImage(texture->m_Image, vk::ImageLayout::eTransferDstOptimal, &clearValue, 1, &subresourceRange);
	}

	inline void CommandList::WriteVolatileBuffer(Buffer* buffer, const void* data, Uint64 dataSize) {
		auto& state{ this->m_VolatileBufferStates[buffer] };

		if (!state.Initialized) {
			state.MinVersion = buffer->m_Desc.MaxVersions;
			state.MaxVersion = 0;//TODO :
			state.Initialized = true;
		}

		Uint64 queueCompletionValues{ this->m_Queue->Get_LastFinishedID() };

		Uint32 searchStart{ buffer->m_VersionSearchStart };
		Uint32 maxVersions{ buffer->m_Desc.MaxVersions };
		Uint32 version{ 0 };

		Uint64 originalVersionInfo{ 0 };

		// Since versionTracking[] can be accessed by multiple threads concurrently,
		// perform the search in a loop ending with compare_exchange until the exchange is successful.
		while (true) {
			bool found{ false };

			// Search through the versions of this buffer, looking for either unused (0)
			// or submitted and already finished versions

			for (Uint32 searchIndex = 0; searchIndex < maxVersions; ++searchIndex) {
				version = searchIndex + searchStart;
				version = (version >= maxVersions) ? (version - maxVersions) : version;

				originalVersionInfo = buffer->m_VersionTracking[version];

				if (0 == originalVersionInfo) {
					// Previously unused version - definitely available
					found = true;
					break;
				}

				if (VersionGetSubmitted(originalVersionInfo) && VersionGetInstance(originalVersionInfo) <= queueCompletionValues) {

					found = true;
					break;
				}
			}

			if (!found) {
				// Not enough versions - need to relay this information to the developer.
				// This has to be a real message and not assert, because asserts only happen in the
				// debug mode, and buffer versioning will behave differently in debug vs. release,
				// or validation on vs. off, because it is timing related.

				ASSERT(false);
				return;
			}

			// Encode the current CL ID for this version of the buffer, in a "pending" state
			Uint64 newVersionInfo{ (static_cast<Uint64>(this->m_CommandListParameters.QueueType) << c_VersionQueueShift) | (this->m_CurrentCmdBuf->RecordingID) };

			// Try to store the new version info, end the loop if we actually won this version, i.e. no other thread has claimed it
			if (buffer->m_VersionTracking[version].compare_exchange_weak(originalVersionInfo, newVersionInfo))
				break;
		}

		buffer->m_VersionSearchStart = (version + 1 < maxVersions) ? (version + 1) : 0;

		// Store the current version and expand the version range in this CL
		state.LatestVersion = version;
		state.MinVersion = Math::Min(version, state.MinVersion);
		state.MaxVersion = Math::Max(version, state.MaxVersion);

		// Finally, write the actual data
		memcpy(static_cast<char*>(buffer->m_MappedMemory) + version * buffer->m_Desc.ByteSize, data, dataSize);

		this->m_AnyVolatileBufferWrites = true;
	}

	inline void CommandList::FlushVolatileBufferWrites(void) {
		// The volatile CBs are permanently mapped with the eHostVisible flag, but not eHostCoherent,
		// so before using the data on the GPU, we need to make sure it's available there.
		// Go over all the volatile CBs that were used in this CL and flush their written versions.

		Vector<vk::MappedMemoryRange> ranges;

		for (auto& [buffer, state] : this->m_VolatileBufferStates) {
			if (state.MaxVersion < state.MinVersion || !state.Initialized)
				continue;

			// Flush all the versions between min and max - that might be too conservative,
			// but that should be fine - better than using potentially hundreds of ranges.
			Uint32 numVersions{ state.MaxVersion - state.MinVersion + 1 };

			auto range{ vk::MappedMemoryRange{}
				.setMemory(buffer->MemoryResource::Memory)
				.setOffset(state.MinVersion * buffer->m_Desc.ByteSize)
				.setSize(numVersions * buffer->m_Desc.ByteSize)
			};

			ranges.push_back(range);
		}

		if (!ranges.empty())
			this->m_Context.Device.flushMappedMemoryRanges(ranges);
	}

	inline void CommandList::TrackResourcesAndBarriers(const RHIGraphicsState<VulkanTag>& state) {
		ASSERT(nullptr != this->m_CurrentCmdBuf);

		if (!ArrayEqual(state.BindingSets, state.BindingSetCount, this->m_CurrentGraphicsState.BindingSets, this->m_CurrentGraphicsState.BindingSetCount))
			for (Uint32 Index = 0; Index < state.BindingSetCount; ++Index)//TODO :Span
				this->SetResourceStatesForBindingSet(state.BindingSets[Index]);

		if (nullptr != state.IndexBuffer.Buffer && state.IndexBuffer.Buffer != this->m_CurrentGraphicsState.IndexBuffer.Buffer)
			this->m_StateTracker.RequireBufferState(&state.IndexBuffer.Buffer->m_StateExtension, RHIResourceState::IndexBuffer);

		if (!ArrayEqual(state.VertexBuffers, state.VertexBufferCount, this->m_CurrentGraphicsState.VertexBuffers, this->m_CurrentGraphicsState.VertexBufferCount))
			for (const auto& vb : Span<const RHIVertexBufferBinding<VulkanTag>>{ state.VertexBuffers.data(), state.VertexBufferCount })
				this->m_StateTracker.RequireBufferState(&vb.Buffer->m_StateExtension, RHIResourceState::VertexBuffer);

		if (this->m_CurrentGraphicsState.FrameBuffer != state.FrameBuffer)
			this->SetResourceStatesForFramebuffer(state.FrameBuffer);

		if (nullptr != state.IndirectParams && state.IndirectParams != m_CurrentGraphicsState.IndirectParams)
			this->m_StateTracker.RequireBufferState(&state.IndirectParams->m_StateExtension, RHIResourceState::IndirectArgument);
	}

	inline void CommandList::BindBindingSets(vk::PipelineBindPoint bindPoint, vk::PipelineLayout pipelineLayout, Span<const BindingSet* const> bindings, Span<const Uint32> descriptorSetIdxToBindingIdx) {
		const Uint32 numBindings{ static_cast<Uint32>(bindings.size()) };//TODO :Check
		const Uint32 numDescriptorSets{ static_cast<Uint32>(descriptorSetIdxToBindingIdx.size()) };

		Array<vk::DescriptorSet, g_MaxBindingLayoutCount> descriptorSets;
		RemoveCV<decltype(g_MaxBindingLayoutCount)>::type descriptorSetsCount{ 0 };

		Array<Uint32, g_MaxVolatileConstantBuffers> dynamicOffsets{};
		RemoveCV<decltype(g_MaxVolatileConstantBuffers)>::type dynamicOffsetsCount{ 0 };

		Uint32 nextDescriptorSetToBind{ 0 };

		for (Uint32 Index = 0; Index < numDescriptorSets; ++Index) {
			const BindingSet* bindingSet{ nullptr };
			bindingSet = bindings[descriptorSetIdxToBindingIdx[Index]];

			if (nullptr == bindingSet) {
				// This is a hole in the descriptor sets, so bind the contiguous descriptor sets we've got so far
				if (!descriptorSets.empty()) {
					this->m_CurrentCmdBuf->CmdBuf.bindDescriptorSets(bindPoint, pipelineLayout,
						/* firstSet = */ nextDescriptorSetToBind,
						descriptorSetsCount, descriptorSets.data(),
						dynamicOffsetsCount, dynamicOffsets.data());

					descriptorSetsCount = 0;
					dynamicOffsetsCount = 0;
				}
				nextDescriptorSetToBind = Index + 1;
			}
			else {
				const auto& desc{ *bindingSet->Get_Desc() };

				descriptorSets[descriptorSetsCount++] = bindingSet->m_DescriptorSet;

				for (Buffer* constantBuffer : Span<Buffer* const>{ bindingSet->m_VolatileConstantBuffers.data(), bindingSet->m_VolatileConstantBuffersCount }) {
					if (auto found{ this->m_VolatileBufferStates.find(constantBuffer) }; found != this->m_VolatileBufferStates.end()) {
						Uint32 version{ found->second.LatestVersion };
						Uint64 offset{ version * constantBuffer->m_Desc.ByteSize };

						dynamicOffsets[dynamicOffsetsCount++] = static_cast<Uint32>(offset);
					}
					else
						LOG_ERROR("Volatile constant buffer not found in the command list's volatile buffer states.");
				}

				if (desc.TrackLiveness)
					this->m_CurrentCmdBuf->ReferencedResources.push_back(bindingSet);
			}
		}

		if (0 != descriptorSetsCount)// Bind the remaining sets
			m_CurrentCmdBuf->CmdBuf.bindDescriptorSets(bindPoint, pipelineLayout,
				/* firstSet = */ nextDescriptorSetToBind,
				descriptorSetsCount, descriptorSets.data(),
				dynamicOffsetsCount, dynamicOffsets.data()
			);
	}

	inline void CommandList::UpdateGraphicsVolatileBuffers(void) {
		if (this->m_AnyVolatileBufferWrites && nullptr != this->m_CurrentGraphicsState.Pipeline) {
			const auto& pso{ this->m_CurrentGraphicsState.Pipeline };

			this->BindBindingSets(vk::PipelineBindPoint::eGraphics, pso->m_PipelineLayout, Span<const BindingSet* const>{ this->m_CurrentGraphicsState.BindingSets.data(),this->m_CurrentGraphicsState.BindingSetCount }, Span<const Uint32>{ pso->m_DescriptorSetIdxToBindingIdx.data(), pso->m_BindingLayoutCount });

			this->m_AnyVolatileBufferWrites = false;
		}
	}

	inline void CommandList::CommitBarriersInternal(void) {
		Vector<vk::ImageMemoryBarrier> imageBarriers;
		Vector<vk::BufferMemoryBarrier> bufferBarriers;
		vk::PipelineStageFlags beforeStageFlags{};
		vk::PipelineStageFlags afterStageFlags{};

		for (const auto& barrier : this->m_StateTracker.Get_TextureBarriers()) {
			ResourceStateMapping before{ ConvertResourceState(barrier.StateBefore) };
			ResourceStateMapping after{ ConvertResourceState(barrier.StateAfter) };

			if ((before.StageFlags != beforeStageFlags || after.StageFlags != afterStageFlags) && !imageBarriers.empty()) {
				this->m_CurrentCmdBuf->CmdBuf.pipelineBarrier(beforeStageFlags, afterStageFlags, vk::DependencyFlags{}, {}, {}, imageBarriers);

				imageBarriers.clear();
			}

			beforeStageFlags = before.StageFlags;
			afterStageFlags = after.StageFlags;

			ASSERT(after.ImageLayout != vk::ImageLayout::eUndefined);

			const Texture* texture{ static_cast<const Texture*>(barrier.Texture->ParentTextureRef) };

			const auto& formatInfo{ Get_RHIFormatInfo(texture->m_Desc.Format) };

			vk::ImageAspectFlags aspectMask{};
			if (formatInfo.HasDepth)
				aspectMask |= vk::ImageAspectFlagBits::eDepth;
			if (formatInfo.HasStencil)
				aspectMask |= vk::ImageAspectFlagBits::eStencil;
			if (!aspectMask)
				aspectMask = vk::ImageAspectFlagBits::eColor;

			vk::ImageSubresourceRange subresourceRange{ vk::ImageSubresourceRange{}
				.setBaseArrayLayer(barrier.EntireTexture ? 0 : barrier.ArraySlice)
				.setLayerCount(barrier.EntireTexture ? texture->m_Desc.ArrayCount : 1)
				.setBaseMipLevel(barrier.EntireTexture ? 0 : barrier.MipLevel)
				.setLevelCount(barrier.EntireTexture ? texture->m_Desc.MipLevelCount : 1)
				.setAspectMask(aspectMask)
			};

			imageBarriers.push_back(vk::ImageMemoryBarrier{}
				.setSrcAccessMask(before.AccessMask)
				.setDstAccessMask(after.AccessMask)
				.setOldLayout(before.ImageLayout)
				.setNewLayout(after.ImageLayout)
				.setSrcQueueFamilyIndex((~0U /*VK_QUEUE_FAMILY_IGNORED*/))
				.setDstQueueFamilyIndex((~0U /*VK_QUEUE_FAMILY_IGNORED*/))
				.setImage(texture->m_Image)
				.setSubresourceRange(subresourceRange));
		}

		if (!imageBarriers.empty())
			this->m_CurrentCmdBuf->CmdBuf.pipelineBarrier(beforeStageFlags, afterStageFlags, vk::DependencyFlags{}, {}, {}, imageBarriers);

		beforeStageFlags = vk::PipelineStageFlags{};
		afterStageFlags = vk::PipelineStageFlags{};
		imageBarriers.clear();

		for (const auto& barrier : m_StateTracker.Get_BufferBarriers()) {
			ResourceStateMapping before{ ConvertResourceState(barrier.StateBefore) };
			ResourceStateMapping after{ ConvertResourceState(barrier.StateAfter) };

			if ((before.StageFlags != beforeStageFlags || after.StageFlags != afterStageFlags) && !bufferBarriers.empty()) {
				this->m_CurrentCmdBuf->CmdBuf.pipelineBarrier(beforeStageFlags, afterStageFlags, vk::DependencyFlags{}, {}, bufferBarriers, {});

				bufferBarriers.clear();
			}

			beforeStageFlags = before.StageFlags;
			afterStageFlags = after.StageFlags;

			Buffer* buffer{ static_cast<Buffer*>(barrier.Buffer->ParentBuffer) };

			bufferBarriers.push_back(vk::BufferMemoryBarrier{}
				.setSrcAccessMask(before.AccessMask)
				.setDstAccessMask(after.AccessMask)
				.setSrcQueueFamilyIndex(~0U /*VK_QUEUE_FAMILY_IGNORED*/)
				.setDstQueueFamilyIndex(~0U /*VK_QUEUE_FAMILY_IGNORED*/)
				.setBuffer(buffer->m_Buffer)
				.setOffset(0)
				.setSize(buffer->m_Desc.ByteSize));
		}

		if (!bufferBarriers.empty())
			this->m_CurrentCmdBuf->CmdBuf.pipelineBarrier(beforeStageFlags, afterStageFlags, vk::DependencyFlags{}, {}, bufferBarriers, {});
		bufferBarriers.clear();

		this->m_StateTracker.ClearBarriers();
	}

	inline void CommandList::CommitBarriersInternal_synchronization2(void) {
		Vector<vk::ImageMemoryBarrier2> imageBarriers;
		Vector<vk::BufferMemoryBarrier2> bufferBarriers;

		for (const auto& barrier : m_StateTracker.Get_TextureBarriers()) {
			ResourceStateMapping2 before{ ConvertResourceState2(barrier.StateBefore) };
			ResourceStateMapping2 after{ ConvertResourceState2(barrier.StateAfter) };

			ASSERT(after.ImageLayout != vk::ImageLayout::eUndefined);

			Texture* texture{ static_cast<Texture*>(barrier.Texture->ParentTextureRef) };

			const auto& formatInfo{ Get_RHIFormatInfo(texture->m_Desc.Format) };

			vk::ImageAspectFlags aspectMask{};
			if (formatInfo.HasDepth)
				aspectMask |= vk::ImageAspectFlagBits::eDepth;
			if (formatInfo.HasStencil)
				aspectMask |= vk::ImageAspectFlagBits::eStencil;
			if (!aspectMask)
				aspectMask = vk::ImageAspectFlagBits::eColor;

			vk::ImageSubresourceRange subresourceRange{ vk::ImageSubresourceRange{}
				.setBaseArrayLayer(barrier.EntireTexture ? 0 : barrier.ArraySlice)
				.setLayerCount(barrier.EntireTexture ? texture->m_Desc.ArrayCount : 1)
				.setBaseMipLevel(barrier.EntireTexture ? 0 : barrier.MipLevel)
				.setLevelCount(barrier.EntireTexture ? texture->m_Desc.MipLevelCount : 1)
				.setAspectMask(aspectMask)
			};

			imageBarriers.push_back(vk::ImageMemoryBarrier2{}
				.setSrcAccessMask(before.AccessMask)
				.setDstAccessMask(after.AccessMask)
				.setSrcStageMask(before.StageFlags)
				.setDstStageMask(after.StageFlags)
				.setOldLayout(before.ImageLayout)
				.setNewLayout(after.ImageLayout)
				.setSrcQueueFamilyIndex(~0U /*VK_QUEUE_FAMILY_IGNORED*/)
				.setDstQueueFamilyIndex(~0U /*VK_QUEUE_FAMILY_IGNORED*/)
				.setImage(texture->m_Image)
				.setSubresourceRange(subresourceRange)
			);
		}

		if (!imageBarriers.empty()) {
			vk::DependencyInfo dep_info{};
			dep_info.setImageMemoryBarriers(imageBarriers);

			this->m_CurrentCmdBuf->CmdBuf.pipelineBarrier2(dep_info);
		}

		imageBarriers.clear();

		for (const auto& barrier : this->m_StateTracker.Get_BufferBarriers()) {
			ResourceStateMapping2 before{ ConvertResourceState2(barrier.StateBefore) };
			ResourceStateMapping2 after{ ConvertResourceState2(barrier.StateAfter) };

			Buffer* buffer{ static_cast<Buffer*>(barrier.Buffer->ParentBuffer) };

			bufferBarriers.push_back(vk::BufferMemoryBarrier2{}
				.setSrcAccessMask(before.AccessMask)
				.setDstAccessMask(after.AccessMask)
				.setSrcStageMask(before.StageFlags)
				.setDstStageMask(after.StageFlags)
				.setSrcQueueFamilyIndex((~0U /*VK_QUEUE_FAMILY_IGNORED*/))
				.setDstQueueFamilyIndex((~0U /*VK_QUEUE_FAMILY_IGNORED*/))
				.setBuffer(buffer->m_Buffer)
				.setOffset(0)
				.setSize(buffer->m_Desc.ByteSize)
			);
		}

		if (!bufferBarriers.empty()) {
			vk::DependencyInfo dep_info{};
			dep_info.setBufferMemoryBarriers(bufferBarriers);

			this->m_CurrentCmdBuf->CmdBuf.pipelineBarrier2(dep_info);
		}

		bufferBarriers.clear();

		this->m_StateTracker.ClearBarriers();
	}

	inline void CommandList::SubmitVolatileBuffers(Uint64 recordingID, Uint64 submittedID) {
		Uint64 stateToFind{ (static_cast<Uint64>(this->m_CommandListParameters.QueueType) << c_VersionQueueShift) | (recordingID & c_VersionIDMask) };
		Uint64 stateToReplace{ (static_cast<Uint64>(this->m_CommandListParameters.QueueType) << c_VersionQueueShift) | (submittedID & c_VersionIDMask) | c_VersionSubmittedFlag };

		for (auto& [buffer, state] : this->m_VolatileBufferStates) {
			if (!state.Initialized)
				continue;

			for (auto version{ state.MinVersion }; version <= state.MaxVersion; ++version) {
				Uint64 expected{ stateToFind };
				buffer->m_VersionTracking[version].compare_exchange_strong(expected, stateToReplace);
			}
		}
	}

	inline void CommandList::Executed(Uint64 submissionID) {
		ASSERT(nullptr != this->m_CurrentCmdBuf);

		this->m_CurrentCmdBuf->SubmissionID = submissionID;

		const Uint64 recordingID{ this->m_CurrentCmdBuf->RecordingID };

		this->m_CurrentCmdBuf = nullptr;

		this->SubmitVolatileBuffers(recordingID, submissionID);

		this->m_StateTracker.CommandListSubmitted();

		this->m_UploadManager->SubmitChunks(
			MakeVersion(recordingID, this->m_CommandListParameters.QueueType, false),
			MakeVersion(submissionID, this->m_CommandListParameters.QueueType, true));

		this->m_VolatileBufferStates.clear();
	}

	//Imp

	void CommandList::Imp_Open(void) {
		this->m_CurrentCmdBuf = this->GetOrCreateCommandBuffer();

		auto beginInfo{ vk::CommandBufferBeginInfo{}
			.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)
		};

		VULKAN_CHECK(this->m_CurrentCmdBuf->CmdBuf.begin(&beginInfo));
		this->m_CurrentCmdBuf->ReferencedResources.push_back(this); //TODO : prevent deletion of e.g. UploadManager

		this->ClearState();
	}

	void CommandList::Imp_Close(void) {
		this->EndRenderPass();

		this->m_StateTracker.KeepBufferInitialStates();
		this->m_StateTracker.KeepTextureInitialStates();
		this->CommitBarriers();

		this->m_CurrentCmdBuf->CmdBuf.end();

		this->ClearState();

		this->FlushVolatileBufferWrites();
	}

	void CommandList::Imp_ClearState(void) {
		this->EndRenderPass();

		this->m_CurrentPipelineLayout = nullptr;
		this->m_CurrentPushConstantsVisibility = vk::ShaderStageFlagBits{};

		this->m_CurrentGraphicsState = RHIGraphicsState<VulkanTag>{};
		this->m_CurrentComputeState = RHIComputeState<VulkanTag>{};
		this->m_CurrentMeshletState = RHIMeshletState<VulkanTag>{};

		this->m_AnyVolatileBufferWrites = false;
	}

	void CommandList::Imp_ClearTextureFloat(Texture* texture, RHITextureSubresourceSet subresources, const Color& clearColor) {
		auto clearValue{ vk::ClearColorValue{}
			.setFloat32({ clearColor.R, clearColor.G, clearColor.B, clearColor.A })
		};

		this->ClearTexture(texture, subresources, clearValue);
	}

	void CommandList::Imp_ClearDepthStencilTexture(Texture* texture, RHITextureSubresourceSet subresources, Optional<float> depth, Optional<Uint8> stencil) {
		this->EndRenderPass();

		if (!depth.has_value() && !stencil.has_value()) {
			LOG_WARN("Attempting to clear depth/stencil texture without any values specified. No operation will be performed.");

			return;
		}

		ASSERT(nullptr != texture);
		ASSERT(nullptr != this->m_CurrentCmdBuf);

		subresources = subresources.Resolve(texture->m_Desc, false);

		if (this->m_EnableAutomaticBarriers)
			this->m_StateTracker.RequireTextureState(&texture->m_StateExtension, subresources, RHIResourceState::CopyDest);
		this->CommitBarriers();

		vk::ImageAspectFlags aspectFlags{};
		vk::ClearDepthStencilValue clearValue{};

		if (depth.has_value()) {
			aspectFlags |= vk::ImageAspectFlagBits::eDepth;
			clearValue.setDepth(depth.value());
		}

		if (stencil.has_value()) {
			aspectFlags |= vk::ImageAspectFlagBits::eStencil;
			clearValue.setStencil(stencil.value());
		}

		auto subresourceRange{ vk::ImageSubresourceRange{}
			.setAspectMask(aspectFlags)
			.setBaseArrayLayer(subresources.BaseArraySlice)
			.setLayerCount(subresources.ArraySliceCount)
			.setBaseMipLevel(subresources.BaseMipLevel)
			.setLevelCount(subresources.MipLevelCount)
		};

		this->m_CurrentCmdBuf->CmdBuf.clearDepthStencilImage(texture->m_Image, vk::ImageLayout::eTransferDstOptimal, &clearValue, 1, &subresourceRange
		);
	}

	void CommandList::Imp_ClearTextureUInt(Texture* texture, RHITextureSubresourceSet subresources, Uint32 clearColor) {
		Int32 clearColorInt{ static_cast<Int32>(clearColor) };

		auto clearValue{ vk::ClearColorValue{}
			.setUint32({ clearColor, clearColor, clearColor, clearColor })
			.setInt32({ clearColorInt, clearColorInt, clearColorInt, clearColorInt })
		};

		this->ClearTexture(texture, subresources, clearValue);
	}

	void CommandList::Imp_CopyTexture(Texture* des, RHITextureSlice desSlice, Texture* src, RHITextureSlice srcSlice) {
		ASSERT(false);
	}

	void CommandList::Imp_CopyTexture(Texture* des, RHITextureSlice desSlice, StagingTexture* src, RHITextureSlice srcSlice) {
		ASSERT(false);
	}

	void CommandList::Imp_CopyTexture(StagingTexture* des, RHITextureSlice desSlice, Texture* src, RHITextureSlice srcSlice) {
		ASSERT(false);
	}

	void CommandList::Imp_WriteTexture(Texture* dest, Uint32 arraySlice, Uint32 mipLevel, const void* data, Uint64 rowPitch, Uint64 depthPitch) {
		this->EndRenderPass();

		const auto& desc{ dest->Get_Desc() };

		const auto& [mipWidth, mipHeight, mipDepth] { ComputeMipLevelInformation(desc, mipLevel)};

		const auto& formatInfo{ Get_RHIFormatInfo(desc.Format) };
		Uint32 deviceNumCols{ (mipWidth + formatInfo.BlockSize - 1) / formatInfo.BlockSize };
		Uint32 deviceNumRows{ (mipHeight + formatInfo.BlockSize - 1) / formatInfo.BlockSize };
		Uint32 deviceRowPitch{ deviceNumCols * formatInfo.BytesPerBlock };
		Uint64 deviceMemSize{ static_cast<Uint64>(deviceRowPitch) * static_cast<Uint64>(deviceNumRows) * mipDepth };

		vk::Buffer* uploadBuffer;
		Uint64 uploadOffset;
		void* uploadCpuVA;
		this->m_UploadManager->SuballocateBuffer(deviceMemSize, &uploadBuffer, &uploadOffset, &uploadCpuVA, MakeVersion(this->m_CurrentCmdBuf->RecordingID, this->m_CommandListParameters.QueueType, false));

		ASSERT(nullptr != uploadBuffer);

		Uint64 minRowPitch{ Math::Min(Uint64(deviceRowPitch), rowPitch) };
		Uint8* mappedPtr{ static_cast<Uint8*>(uploadCpuVA) };

		for (Uint32 slice = 0; slice < mipDepth; ++slice) {
			const Uint8* sourcePtr{ static_cast<const Uint8*>(data) + depthPitch * slice };
			for (Uint32 row = 0; row < deviceNumRows; ++row) {
				memcpy(mappedPtr, sourcePtr, minRowPitch);
				mappedPtr += deviceRowPitch;
				sourcePtr += rowPitch;
			}
		}

		auto imageCopy{ vk::BufferImageCopy{}
			.setBufferOffset(uploadOffset)
			.setBufferRowLength(deviceNumCols * formatInfo.BlockSize)
			.setBufferImageHeight(deviceNumRows * formatInfo.BlockSize)
			.setImageSubresource(vk::ImageSubresourceLayers{}
				.setAspectMask(GuessImageAspectFlags(dest->m_ImageInfo.format))
				.setMipLevel(mipLevel)
				.setBaseArrayLayer(arraySlice)
				.setLayerCount(1))
			.setImageExtent(vk::Extent3D{}.setWidth(mipWidth).setHeight(mipHeight).setDepth(mipDepth))
		};

		ASSERT(nullptr != this->m_CurrentCmdBuf);

		if (this->m_EnableAutomaticBarriers)
			this->m_StateTracker.RequireTextureState(&dest->m_StateExtension, RHITextureSubresourceSet{ .BaseMipLevel{ mipLevel }, .BaseArraySlice { arraySlice } }, RHIResourceState::CopyDest);
		this->CommitBarriers();

		this->m_CurrentCmdBuf->ReferencedResources.push_back(dest);

		this->m_CurrentCmdBuf->CmdBuf.copyBufferToImage(*uploadBuffer, dest->m_Image, vk::ImageLayout::eTransferDstOptimal, 1, &imageCopy);
	}

	void CommandList::Imp_ResolveTexture(Texture* dest, const RHITextureSubresourceSet& dstSubresources, Texture* src, const RHITextureSubresourceSet& srcSubresources) {
		this->EndRenderPass();

		const auto dstSR{ dstSubresources.Resolve(dest->m_Desc, false) };
		const auto srcSR{ srcSubresources.Resolve(src->m_Desc, false) };

		if (dstSR.ArraySliceCount != srcSR.ArraySliceCount || dstSR.MipLevelCount != srcSR.MipLevelCount) {
			LOG_ERROR("Cannot resolve texture: destination and source subresources must have the same number of array slices and mip levels.");
			return;
		}

		ASSERT(nullptr != this->m_CurrentCmdBuf);

		Vector<vk::ImageResolve> regions;
		auto dstLayers{ vk::ImageSubresourceLayers{}.setAspectMask(vk::ImageAspectFlagBits::eColor) };
		auto srcLayers{ vk::ImageSubresourceLayers{}.setAspectMask(vk::ImageAspectFlagBits::eColor) };

		for (Uint32 mipLevel = 0; mipLevel < dstSR.MipLevelCount; ++mipLevel) {
			dstLayers.setMipLevel(mipLevel + dstSR.BaseMipLevel).setBaseArrayLayer(dstSR.BaseArraySlice).setLayerCount(dstSR.ArraySliceCount);
			srcLayers.setMipLevel(mipLevel + srcSR.BaseMipLevel).setBaseArrayLayer(srcSR.BaseArraySlice).setLayerCount(srcSR.ArraySliceCount);

			regions.push_back(vk::ImageResolve{}
				.setSrcSubresource(srcLayers)
				.setDstSubresource(dstLayers)
				.setExtent(vk::Extent3D{}
					.setWidth(Math::Max(dest->m_Desc.Extent.Width >> dstLayers.mipLevel, 1u))
					.setHeight(Math::Max(dest->m_Desc.Extent.Height >> dstLayers.mipLevel, 1u))
					.setDepth(Math::Max(dest->m_Desc.Extent.Depth >> dstLayers.mipLevel, 1u)))
			);
		}

		if (this->m_EnableAutomaticBarriers) {
			this->m_StateTracker.RequireTextureState(&src->m_StateExtension, srcSR, RHIResourceState::ResolveSource);
			this->m_StateTracker.RequireTextureState(&dest->m_StateExtension, dstSR, RHIResourceState::ResolveDest);
		}
		this->CommitBarriers();

		this->m_CurrentCmdBuf->CmdBuf.resolveImage(src->m_Image, vk::ImageLayout::eTransferSrcOptimal, dest->m_Image, vk::ImageLayout::eTransferDstOptimal, regions);
	}

	void CommandList::Imp_WriteBuffer(Buffer* buffer, const void* data, Uint64 dataSize, Uint64 destOffsetBytes) {
		ASSERT(dataSize <= buffer->m_Desc.ByteSize);
		ASSERT(nullptr != this->m_CurrentCmdBuf);

		this->EndRenderPass();

		this->m_CurrentCmdBuf->ReferencedResources.push_back(buffer);

		if (buffer->m_Desc.IsVolatile) {
			ASSERT(destOffsetBytes == 0);

			this->WriteVolatileBuffer(buffer, data, dataSize);

			return;
		}

		constexpr Uint64 vkCmdUpdateBufferLimit{ 65536 };

		// Per Vulkan spec, vkCmdUpdateBuffer requires that the data size is smaller than or equal to 64 kB,
		// and that the offset and data size are a multiple of 4. We can't change the offset, but data size
		// is rounded up later.
		if (dataSize <= vkCmdUpdateBufferLimit && (destOffsetBytes & 3) == 0) {
			if (this->m_EnableAutomaticBarriers)
				this->m_StateTracker.RequireBufferState(&buffer->m_StateExtension, RHIResourceState::CopyDest);
			this->CommitBarriers();

			// Round up the write size to a multiple of 4
			const Uint64 sizeToWrite{ (dataSize + 3) & ~3ull };

			this->m_CurrentCmdBuf->CmdBuf.updateBuffer(buffer->m_Buffer, destOffsetBytes, sizeToWrite, data);
		}
		else if (RHICPUAccessMode::Write != buffer->m_Desc.CPUAccess) {
			// use the upload manager
			vk::Buffer* uploadBuffer;
			Uint64 uploadOffset;
			void* uploadCpuVA;
			this->m_UploadManager->SuballocateBuffer(dataSize, &uploadBuffer, &uploadOffset, &uploadCpuVA, MakeVersion(this->m_CurrentCmdBuf->RecordingID, this->m_CommandListParameters.QueueType, false));

			ASSERT(nullptr != uploadBuffer);

			memcpy(uploadCpuVA, data, dataSize);

			if (RHICPUAccessMode::None != buffer->m_Desc.CPUAccess)
				this->m_CurrentCmdBuf->ReferencedStagingBuffers.push_back(buffer);
			else
				this->m_CurrentCmdBuf->ReferencedResources.push_back(buffer);

			if (this->m_EnableAutomaticBarriers)
				this->m_StateTracker.RequireBufferState(&buffer->m_StateExtension, RHIResourceState::CopyDest);
			this->CommitBarriers();

			auto copyRegion{ vk::BufferCopy{}
				.setSize(dataSize)
				.setSrcOffset(uploadOffset)
				.setDstOffset(destOffsetBytes)
			};

			this->m_CurrentCmdBuf->CmdBuf.copyBuffer(*uploadBuffer, buffer->m_Buffer, { copyRegion });
		}
		else
			LOG_ERROR("Attempting to write to a buffer with CPU access mode Write, but the data size exceeds the limit for vkCmdUpdateBuffer. Use a different method to write the data.");
	}

	void CommandList::Imp_ClearBufferUInt(Buffer* vkbuf, Uint32 clearvalue) {
		ASSERT(nullptr != this->m_CurrentCmdBuf);

		this->EndRenderPass();

		if (this->m_EnableAutomaticBarriers)
			this->m_StateTracker.RequireBufferState(&vkbuf->m_StateExtension, RHIResourceState::CopyDest);
		this->CommitBarriers();

		this->m_CurrentCmdBuf->CmdBuf.fillBuffer(vkbuf->m_Buffer, 0, vkbuf->m_Desc.ByteSize, clearvalue);
		this->m_CurrentCmdBuf->ReferencedResources.push_back(vkbuf);
	}

	void CommandList::Imp_CopyBuffer(Buffer* dest, Uint64 destOffsetBytes, Buffer* src, Uint64 srcOffsetBytes, Uint64 dataSizeBytes) {
		ASSERT(destOffsetBytes + dataSizeBytes <= dest->m_Desc.ByteSize);
		ASSERT(srcOffsetBytes + dataSizeBytes <= src->m_Desc.ByteSize);
		ASSERT(nullptr != this->m_CurrentCmdBuf);

		if (RHICPUAccessMode::None != dest->m_Desc.CPUAccess)
			this->m_CurrentCmdBuf->ReferencedStagingBuffers.push_back(dest);
		else
			this->m_CurrentCmdBuf->ReferencedResources.push_back(dest);

		if (RHICPUAccessMode::None != src->m_Desc.CPUAccess)
			this->m_CurrentCmdBuf->ReferencedStagingBuffers.push_back(src);
		else
			this->m_CurrentCmdBuf->ReferencedResources.push_back(src);

		if (this->m_EnableAutomaticBarriers) {
			this->m_StateTracker.RequireBufferState(&src->m_StateExtension, RHIResourceState::CopySource);
			this->m_StateTracker.RequireBufferState(&dest->m_StateExtension, RHIResourceState::CopyDest);
		}
		this->CommitBarriers();

		auto copyRegion{ vk::BufferCopy{}
			.setSize(dataSizeBytes)
			.setSrcOffset(srcOffsetBytes)
			.setDstOffset(destOffsetBytes)
		};

		this->m_CurrentCmdBuf->CmdBuf.copyBuffer(src->m_Buffer, dest->m_Buffer, { copyRegion });
	}

	void CommandList::Imp_SetPushConstants(const void* data, Uint32 byteSize) {
		ASSERT(nullptr != this->m_CurrentCmdBuf);

		this->m_CurrentCmdBuf->CmdBuf.pushConstants(this->m_CurrentPipelineLayout, this->m_CurrentPushConstantsVisibility, 0, static_cast<Uint32>(byteSize), data);
	}

	void CommandList::Imp_SetGraphicsState(const RHIGraphicsState<VulkanTag>& state) {
		ASSERT(nullptr != this->m_CurrentCmdBuf);

		GraphicsPipeline* pso{ state.Pipeline };
		FrameBuffer* fb{ state.FrameBuffer };

		if (this->m_EnableAutomaticBarriers)
			this->TrackResourcesAndBarriers(state);

		bool anyBarriers{ this->AnyBarriers() };
		bool updatePipeline{ false };

		if (this->m_CurrentGraphicsState.Pipeline != state.Pipeline) {
			this->m_CurrentCmdBuf->CmdBuf.bindPipeline(vk::PipelineBindPoint::eGraphics, pso->m_Pipeline);

			this->m_CurrentCmdBuf->ReferencedResources.push_back(state.Pipeline);
			updatePipeline = true;
		}

		if (this->m_CurrentGraphicsState.FrameBuffer != state.FrameBuffer || anyBarriers /* because barriers cannot be set inside a renderpass */)
			this->EndRenderPass();

		this->CommitBarriers();//TODO : 

		if (nullptr == this->m_CurrentGraphicsState.FrameBuffer) {
			this->m_CurrentCmdBuf->CmdBuf.beginRenderPass(vk::RenderPassBeginInfo{}
				.setRenderPass(fb->m_RenderPass)
				.setFramebuffer(fb->m_FrameBuffer)
				.setRenderArea(vk::Rect2D{}
					.setOffset(vk::Offset2D{})
					.setExtent(vk::Extent2D{}.setWidth(fb->m_Info.Width).setHeight(fb->m_Info.Height)))
				.setClearValueCount(0),
				vk::SubpassContents::eInline
			);

			this->m_CurrentCmdBuf->ReferencedResources.push_back(state.FrameBuffer);
		}

		this->m_CurrentPipelineLayout = pso->m_PipelineLayout;
		this->m_CurrentPushConstantsVisibility = pso->m_PushConstantVisibility;

		if (!ArrayEqual(this->m_CurrentComputeState.BindingSets, this->m_CurrentComputeState.BindingSetCount, state.BindingSets, state.BindingSetCount) || this->m_AnyVolatileBufferWrites)
			this->BindBindingSets(vk::PipelineBindPoint::eGraphics, pso->m_PipelineLayout, Span<const BindingSet* const>{ state.BindingSets.data(), state.BindingSetCount }, Span<const Uint32>{ pso->m_DescriptorSetIdxToBindingIdx.data(), pso->m_BindingLayoutCount });

		if (!state.Viewport.Viewports.empty() && !ArrayEqual(state.Viewport.Viewports, state.Viewport.ViewportCount, this->m_CurrentGraphicsState.Viewport.Viewports, this->m_CurrentGraphicsState.Viewport.ViewportCount)) {
			Vector<vk::Viewport> viewports;
			for (const auto& vp : Span<const RHIViewport>{ state.Viewport.Viewports.data(), state.Viewport.ViewportCount })
				viewports.push_back(VKViewportWithDXCoords(vp));

			this->m_CurrentCmdBuf->CmdBuf.setViewport(0, viewports);
		}

		if (!state.Viewport.ScissorRects.empty() && !ArrayEqual(state.Viewport.ScissorRects, state.Viewport.ScissorCount, this->m_CurrentGraphicsState.Viewport.ScissorRects, this->m_CurrentGraphicsState.Viewport.ScissorCount)) {
			Vector<vk::Rect2D> scissors;
			for (const auto& sc : Span<const RHIRect2D>{ state.Viewport.ScissorRects.data(), state.Viewport.ScissorCount })
				scissors.push_back(vk::Rect2D{}
					.setOffset(vk::Offset2D{}.setX(sc.Offset.X).setY(sc.Offset.Y))
					.setExtent(vk::Extent2D{ sc.Extent.Width, sc.Extent.Height })
				);

			this->m_CurrentCmdBuf->CmdBuf.setScissor(0, scissors);
		}

		if (pso->m_Desc.RenderState.DepthStencilState.DynamicStencilRef && (updatePipeline || this->m_CurrentGraphicsState.DynamicStencilRefValue != state.DynamicStencilRefValue))
			this->m_CurrentCmdBuf->CmdBuf.setStencilReference(vk::StencilFaceFlagBits::eFrontAndBack, state.DynamicStencilRefValue);

		if (pso->m_UsesBlendConstants && (updatePipeline || this->m_CurrentGraphicsState.BlendConstantColor != state.BlendConstantColor))
			this->m_CurrentCmdBuf->CmdBuf.setBlendConstants(&state.BlendConstantColor.R);

		if (nullptr != state.IndexBuffer.Buffer && this->m_CurrentGraphicsState.IndexBuffer != state.IndexBuffer) {
			this->m_CurrentCmdBuf->CmdBuf.bindIndexBuffer(state.IndexBuffer.Buffer->m_Buffer,
				state.IndexBuffer.Offset,
				state.IndexBuffer.Format == RHIFormat::R16_UINT ? vk::IndexType::eUint16 : vk::IndexType::eUint32
			);

			this->m_CurrentCmdBuf->ReferencedResources.push_back(state.IndexBuffer.Buffer);
		}

		if (!state.VertexBuffers.empty() && !ArrayEqual(state.VertexBuffers, state.VertexBufferCount, this->m_CurrentGraphicsState.VertexBuffers, this->m_CurrentGraphicsState.VertexBufferCount)) {
			Array<vk::Buffer, g_MaxVertexAttributeCount> vertexBuffers;
			Array<vk::DeviceSize, g_MaxVertexAttributeCount> vertexBufferOffsets{};
			Uint64 maxVbIndex{ 0 };

			for (const auto& binding : Span<const RHIVertexBufferBinding<VulkanTag>>{ state.VertexBuffers.data(),state.VertexBufferCount }) {
				// This is tested by the validation layer, skip invalid slots here if VL is not used.
				if (binding.Slot >= g_MaxVertexAttributeCount)
					continue;

				vertexBuffers[binding.Slot] = binding.Buffer->m_Buffer;
				vertexBufferOffsets[binding.Slot] = vk::DeviceSize{ binding.Offset };
				maxVbIndex = Math::Max<Uint64>(maxVbIndex, binding.Slot);

				this->m_CurrentCmdBuf->ReferencedResources.push_back(binding.Buffer);
			}

			this->m_CurrentCmdBuf->CmdBuf.bindVertexBuffers(0, maxVbIndex + 1, vertexBuffers.data(), vertexBufferOffsets.data());
		}

		if (nullptr != state.IndirectParams)
			this->m_CurrentCmdBuf->ReferencedResources.push_back(state.IndirectParams);

		this->m_CurrentGraphicsState = state;
		this->m_CurrentComputeState = RHIComputeState<VulkanTag>{};
		this->m_CurrentMeshletState = RHIMeshletState<VulkanTag>{};
		this->m_AnyVolatileBufferWrites = false;
	}

	void CommandList::Imp_Draw(const RHIDrawArguments& args) {
		ASSERT(nullptr != this->m_CurrentCmdBuf);

		this->UpdateGraphicsVolatileBuffers();

		this->m_CurrentCmdBuf->CmdBuf.draw(args.VertexCount, args.InstanceCount, args.StartVertexLocation, args.StartInstanceLocation);
	}

	void CommandList::Imp_DrawIndexed(const RHIDrawArguments& args) {
		ASSERT(nullptr != this->m_CurrentCmdBuf);

		this->UpdateGraphicsVolatileBuffers();

		this->m_CurrentCmdBuf->CmdBuf.drawIndexed(args.VertexCount, args.InstanceCount, args.StartIndexLocation, args.StartVertexLocation, args.StartInstanceLocation);
	}

	void CommandList::Imp_DrawIndirect(Uint32 OffsetBytes, Uint32 Count) {
		ASSERT(nullptr != this->m_CurrentCmdBuf);

		this->UpdateGraphicsVolatileBuffers();

		this->m_CurrentCmdBuf->CmdBuf.drawIndirect(this->m_CurrentGraphicsState.IndirectParams->m_Buffer, OffsetBytes, Count, sizeof(RHIDrawIndirectArguments));
	}

	void CommandList::Imp_DrawIndexedIndirect(Uint32 OffsetBytes, Uint32 Count) {
		ASSERT(nullptr != this->m_CurrentCmdBuf);

		this->UpdateGraphicsVolatileBuffers();

		this->m_CurrentCmdBuf->CmdBuf.drawIndexedIndirect(this->m_CurrentGraphicsState.IndirectParams->m_Buffer, OffsetBytes, Count, sizeof(RHIDrawIndexedIndirectArguments));
	}

	void CommandList::Imp_SetComputeState(const RHIComputeState<VulkanTag>& state) {
		this->EndRenderPass();

		ASSERT(nullptr != this->m_CurrentCmdBuf);

		ComputePipeline* pso{ state.Pipeline };

		if (this->m_EnableAutomaticBarriers && !ArrayEqual(state.BindingSets, state.BindingSetCount, this->m_CurrentComputeState.BindingSets, this->m_CurrentComputeState.BindingSetCount))
			for (Uint64 Index = 0; Index < state.BindingSetCount && Index < pso->m_Desc.BindingLayoutCount; ++Index) {
				BindingLayout* layout{ pso->m_Desc.BindingLayouts[Index].Get() };

				if (RHIShaderType::None == (RHIShaderType::Compute & layout->m_Desc.Visibility))
					continue;

				if (this->m_EnableAutomaticBarriers)
					this->SetResourceStatesForBindingSet(state.BindingSets[Index]);
			}

		if (this->m_CurrentComputeState.Pipeline != state.Pipeline) {
			this->m_CurrentCmdBuf->CmdBuf.bindPipeline(vk::PipelineBindPoint::eCompute, pso->m_Pipeline);

			this->m_CurrentCmdBuf->ReferencedResources.push_back(state.Pipeline);
		}

		if (this->m_AnyVolatileBufferWrites || !ArrayEqual(state.BindingSets, state.BindingSetCount, this->m_CurrentComputeState.BindingSets, this->m_CurrentComputeState.BindingSetCount))
			this->BindBindingSets(vk::PipelineBindPoint::eCompute, pso->m_PipelineLayout, Span<const BindingSet* const>{ state.BindingSets.data(), state.BindingSetCount }, Span<const Uint32>{ pso->m_DescriptorSetIdxToBindingIdx.data(), pso->m_BindingLayoutCount });

		this->m_CurrentPipelineLayout = pso->m_PipelineLayout;
		this->m_CurrentPushConstantsVisibility = pso->m_PushConstantVisibility;

		if (nullptr != state.IndirectParams && state.IndirectParams != this->m_CurrentComputeState.IndirectParams) {
			Buffer* indirectParams{ state.IndirectParams };

			this->m_CurrentCmdBuf->ReferencedResources.push_back(state.IndirectParams);

			if (this->m_EnableAutomaticBarriers)
				this->m_StateTracker.RequireBufferState(&indirectParams->m_StateExtension, RHIResourceState::IndirectArgument);
		}

		this->CommitBarriers();

		this->m_CurrentGraphicsState = RHIGraphicsState<VulkanTag>{};
		this->m_CurrentComputeState = state;
		this->m_CurrentMeshletState = RHIMeshletState<VulkanTag>{};
		this->m_AnyVolatileBufferWrites = false;
	}

	void CommandList::Imp_Dispatch(Uint32 GroupsX, Uint32 GroupsY, Uint32 GroupsZ) {
		ASSERT(nullptr != this->m_CurrentCmdBuf);

		this->UpdateGraphicsVolatileBuffers();

		this->m_CurrentCmdBuf->CmdBuf.dispatch(GroupsX, GroupsY, GroupsZ);
	}

	void CommandList::Imp_DispatchIndirect(Uint32 OffsetBytes) {
		ASSERT(nullptr != this->m_CurrentCmdBuf);

		this->UpdateGraphicsVolatileBuffers();

		this->m_CurrentCmdBuf->CmdBuf.dispatchIndirect(this->m_CurrentComputeState.IndirectParams->m_Buffer, OffsetBytes);
	}

	void CommandList::Imp_SetMeshletState(const RHIMeshletState<VulkanTag>& pipeline) {
		ASSERT(false);
	}

	void CommandList::Imp_DispatchMesh(Uint32 GroupsX, Uint32 GroupsY, Uint32 GroupsZ) {
		ASSERT(false);
	}

	void CommandList::Imp_BeginTimerQuery(TimerQuery* query) {
		this->EndRenderPass();

		ASSERT(query->m_BeginQueryIndex >= 0);
		ASSERT(!query->m_Started);
		ASSERT(nullptr != this->m_CurrentCmdBuf);

		query->m_Resolved = false;

		ASSERT(false);
		/*this->m_CurrentCmdBuf->CmdBuf.resetQueryPool(m_Device->getTimerQueryPool(), query->m_BeginQueryIndex, 2);
		this->m_CurrentCmdBuf->CmdBuf.writeTimestamp(vk::PipelineStageFlagBits::eBottomOfPipe, m_Device->getTimerQueryPool(), query->m_BeginQueryIndex);*/
	}

	void CommandList::Imp_EndTimerQuery(TimerQuery* query) {
		this->EndRenderPass();

		ASSERT(query->m_BeginQueryIndex >= 0);
		ASSERT(!query->m_Started);
		ASSERT(!query->m_Resolved);
		ASSERT(nullptr != this->m_CurrentCmdBuf);


		ASSERT(false);
		/*this->m_CurrentCmdBuf->CmdBuf.writeTimestamp(vk::PipelineStageFlagBits::eBottomOfPipe, m_Device->getTimerQueryPool(), query->m_EndQueryIndex);*/
		query->m_Started = true;
	}

	void CommandList::Imp_BeginMarker(const char* Name) {
		if (this->m_Context.Extensions.EXT_debug_utils) {
			ASSERT(nullptr != this->m_CurrentCmdBuf);

			auto label{ vk::DebugUtilsLabelEXT{}
				.setPLabelName(Name)
			};

			this->m_CurrentCmdBuf->CmdBuf.beginDebugUtilsLabelEXT(&label);
		}
		else if (this->m_Context.Extensions.EXT_debug_marker) {
			ASSERT(nullptr != this->m_CurrentCmdBuf);

			auto markerInfo{ vk::DebugMarkerMarkerInfoEXT{}
				.setPMarkerName(Name)
			};

			this->m_CurrentCmdBuf->CmdBuf.debugMarkerBeginEXT(&markerInfo);
		}
	}

	void CommandList::Imp_EndMarker(void) {
		if (this->m_Context.Extensions.EXT_debug_utils) {
			ASSERT(nullptr != this->m_CurrentCmdBuf);

			this->m_CurrentCmdBuf->CmdBuf.endDebugUtilsLabelEXT();
		}
		else if (this->m_Context.Extensions.EXT_debug_marker) {
			ASSERT(nullptr != this->m_CurrentCmdBuf);

			this->m_CurrentCmdBuf->CmdBuf.debugMarkerEndEXT();
		}
	}

	void CommandList::Imp_SetEnableAutomaticBarriers(bool enable) {
		this->m_EnableAutomaticBarriers = enable;
	}

	void CommandList::Imp_SetResourceStatesForBindingSet(BindingSet* bindingSet) {
		for (const auto bindingIndex : bindingSet->m_BindingsThatNeedTransitions) {
			const auto& binding{ bindingSet->m_Desc.Bindings[bindingIndex] };

			switch (binding.Type) {
				using enum RHIResourceType;
			case Texture_SRV:
				this->m_StateTracker.RequireTextureState(&Get<RefCountPtr<Texture>>(binding.ResourcePtr)->m_StateExtension, binding.Subresources, RHIResourceState::ShaderResource);
				break;

			case Texture_UAV:
				this->m_StateTracker.RequireTextureState(&Get<RefCountPtr<Texture>>(binding.ResourcePtr)->m_StateExtension, binding.Subresources, RHIResourceState::UnorderedAccess);
				break;

			case TypedBuffer_SRV:case StructuredBuffer_SRV:case RawBuffer_SRV:
				this->m_StateTracker.RequireBufferState(&Get<RefCountPtr<Buffer>>(binding.ResourcePtr)->m_StateExtension, RHIResourceState::ShaderResource);
				break;

			case TypedBuffer_UAV:case StructuredBuffer_UAV:case RawBuffer_UAV:
				this->m_StateTracker.RequireBufferState(&Get<RefCountPtr<Buffer>>(binding.ResourcePtr)->m_StateExtension, RHIResourceState::UnorderedAccess);
				break;

			case ConstantBuffer:
				this->m_StateTracker.RequireBufferState(&Get<RefCountPtr<Buffer>>(binding.ResourcePtr)->m_StateExtension, RHIResourceState::ConstantBuffer);
				break;

			default:// do nothing
				break;
			}
		}
	}

	void CommandList::Imp_SetEnableUAVBarriersForTexture(Texture* texture, bool enable) {
		this->m_StateTracker.SetEnableUavBarriersForTexture(&texture->m_StateExtension, enable);
	}

	void CommandList::Imp_SetEnableUAVBarriersForBuffer(Buffer* buffer, bool enable) {
		this->m_StateTracker.SetEnableUavBarriersForBuffer(&buffer->m_StateExtension, enable);
	}

	void CommandList::Imp_BeginTrackingTextureState(Texture* texture, RHITextureSubresourceSet subresources, RHIResourceState state) {
		this->m_StateTracker.BeginTrackingTextureState(&texture->m_StateExtension, subresources, state);
	}

	void CommandList::Imp_BeginTrackingBufferState(Buffer* buffer, RHIResourceState state) {
		this->m_StateTracker.BeginTrackingBufferState(&buffer->m_StateExtension, state);
	}

	void CommandList::Imp_SetTextureState(Texture* texture, RHITextureSubresourceSet subresources, RHIResourceState state) {
		this->m_StateTracker.RequireTextureState(&texture->m_StateExtension, subresources, state);

		if (nullptr != this->m_CurrentCmdBuf)
			this->m_CurrentCmdBuf->ReferencedResources.push_back(texture);
	}

	void CommandList::Imp_SetBufferState(Buffer* buffer, RHIResourceState state) {
		this->m_StateTracker.RequireBufferState(&buffer->m_StateExtension, state);

		if (nullptr != this->m_CurrentCmdBuf)
			this->m_CurrentCmdBuf->ReferencedResources.push_back(buffer);
	}

	void CommandList::Imp_SetPermanentTextureState(Texture* texture, RHIResourceState state) {
		this->m_StateTracker.SetPermanentTextureState(&texture->m_StateExtension, g_AllSubResourceSet, state);

		if (nullptr != this->m_CurrentCmdBuf)
			this->m_CurrentCmdBuf->ReferencedResources.push_back(texture);
	}

	void CommandList::Imp_SetPermanentBufferState(Buffer* buffer, RHIResourceState state) {
		this->m_StateTracker.SetPermanentBufferState(&buffer->m_StateExtension, state);

		if (nullptr != this->m_CurrentCmdBuf)
			this->m_CurrentCmdBuf->ReferencedResources.push_back(buffer);
	}

	void CommandList::Imp_CommitBarriers(void) {
		if (this->m_StateTracker.Get_BufferBarriers().empty() && this->m_StateTracker.Get_TextureBarriers().empty())
			return;

		this->EndRenderPass();

		if (this->m_Context.Extensions.KHR_synchronization2)
			this->CommitBarriersInternal_synchronization2();
		else
			this->CommitBarriersInternal();
	}

	RHIResourceState CommandList::Imp_Get_TextureSubresourceState(Texture* texture, Uint32 arraySlice, Uint32 mipLevel) {
		return this->m_StateTracker.Get_TextureSubresourceState(&texture->m_StateExtension, arraySlice, mipLevel);
	}

	RHIResourceState CommandList::Imp_Get_BufferState(Buffer* buffer) {
		return this->m_StateTracker.Get_BufferState(&buffer->m_StateExtension);
	}

	Device* CommandList::Imp_Get_Device(void) {
		return this->m_Device;
	}

	const RHICommandListParameters& CommandList::Imp_Get_Desc(void) {
		return this->m_CommandListParameters;
	}
}