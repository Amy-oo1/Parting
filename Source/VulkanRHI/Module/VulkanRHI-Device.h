#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"


PARTING_SUBMODULE(D3D12RHI, Pipeline)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Concurrent;
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
PARTING_SUBMODE_IMPORT(CommandList)

#else
#pragma once

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
#include "VulkanRHI/Module/VulkanRHI-CommandList.h"

#endif // PARTING_MODULE_BUILD

namespace RHI::Vulkan {
	class Device : public RHIDevice<Device, VulkanTag> {
		friend class RHIResource<Device>;
		friend class RHIDevice<Device, VulkanTag>;
	public:
		Device(const DeviceDesc& desc);

		~Device(void);

	public:
		void* MapBuffer(Buffer* b, RHICPUAccessMode flags, Uint64 offset, Uint64 size) const;

		void CreatePipelineLayout(vk::PipelineLayout& outPipelineLayout, Array<RefCountPtr<BindingLayout>, g_MaxBindingLayoutCount>& outBindingLayouts, Array<Uint32, g_MaxBindingLayoutCount>& outBindingLayoutCount, Uint32& OutnumRegularDescriptorSets, vk::ShaderStageFlags& outPushConstantVisibility, Span<const RefCountPtr<BindingLayout>> inBindingLayouts);

		void QueueWaitForSemaphore(RHICommandQueue waitQueue, VkSemaphore semaphore, Uint64 value);

		void QueueSignalSemaphore(RHICommandQueue executionQueue, VkSemaphore semaphore, Uint64 value);

	private:
		Context m_Context;
		VulkanAllocator m_Allocator{ this->m_Context };

		Mutex m_Mutex;

		Array<UniquePtr<VulkanQueue>, Tounderlying(RHICommandQueue::Count)> m_Queues;

	private:
		RHIObject Imp_GetNativeObject(RHIObjectType type)const noexcept;

		RefCountPtr<Heap> Imp_CreateHeap(const RHIHeapDesc& desc);
		RefCountPtr<Texture> Imp_CreateTexture(const RHITextureDesc& desc);
		RHIMemoryRequirements Imp_Get_TextureMemoryRequirements(Texture* texture);
		bool Imp_BindTextureMemory(Texture* texture, Heap* heap, Uint64 offset);
		RefCountPtr<Texture> Imp_CreateHandleForNativeTexture(RHIObjectType type, RHIObject texture, const RHITextureDesc& desc);
		RefCountPtr<StagingTexture> Imp_CreateStagingTexture(const RHITextureDesc& desc, RHICPUAccessMode CPUaccess);
		void* Imp_MapStagingTexture(StagingTexture* texture, const RHITextureSlice& slice, RHICPUAccessMode CPUaccess, Uint64* OutRowPitch);
		void Imp_UnmapStagingTexture(StagingTexture* texture);
		void Imp_Get_TextureTiling(Texture* texture, Uint32* TileCount, RHIPackedMipDesc* desc, RHITileShape* tileshadpe, Uint32* SubresourceTilingCount, RHISubresourceTiling* SubresourceTilings);
		void Imp_UpdateTextureTileMappings(Texture* texture, const RHITextureTilesMapping<VulkanTag>* TileMappings, Uint32 TileMappingCount, RHICommandQueue executionQueue);
		RefCountPtr<Buffer> Imp_CreateBuffer(const RHIBufferDesc& desc);
		void* Imp_MapBuffer(Buffer* buffer, RHICPUAccessMode CPUaccess);
		void Imp_UnmapBuffer(Buffer* buffer);
		RHIMemoryRequirements Imp_Get_BufferMemoryRequirements(Buffer* buffer);
		bool Imp_BindBufferMemory(Buffer* buffer, Heap* heap, Uint64 offset);
		RefCountPtr<Shader> Imp_CreateShader(const RHIShaderDesc& desc, const void* binary, Uint64 binarySize);
		RefCountPtr<Sampler> Imp_CreateSampler(const RHISamplerDesc& desc);
		RefCountPtr<InputLayout> Imp_CreateInputLayout(const RHIVertexAttributeDesc* attributes, Uint32 attributeCount);
		RefCountPtr<EventQuery> Imp_CreateEventQuery(void);
		void Imp_SetEventQuery(EventQuery* query, RHICommandQueue queue);
		bool Imp_PollEventQuery(EventQuery* query);
		void Imp_WaitEventQuery(EventQuery* query);
		void Imp_ResetEventQuery(EventQuery* query);
		RefCountPtr<TimerQuery> Imp_CreateTimerQuery(void);
		bool Imp_PollTimerQuery(TimerQuery* query);
		float Imp_Get_TimerQueryTime(TimerQuery* query);
		void Imp_ResetTimerQuery(TimerQuery* query);
		RefCountPtr<FrameBuffer> Imp_CreateFrameBuffer(const RHIFrameBufferDesc<VulkanTag>& desc);
		RefCountPtr<GraphicsPipeline> Imp_CreateGraphicsPipeline(const RHIGraphicsPipelineDesc<VulkanTag>& desc, FrameBuffer* framebuffer);
		RefCountPtr<ComputePipeline> Imp_CreateComputePipeline(const RHIComputePipelineDesc<VulkanTag>& desc);

		RefCountPtr<BindingLayout> Imp_CreateBindingLayout(const RHIBindingLayoutDesc& desc);
		RefCountPtr<BindingSet> Imp_CreateBindingSet(const RHIBindingSetDesc<VulkanTag>& desc, BindingLayout* layout);

		RefCountPtr<CommandList> Imp_CreateCommandList(const RHICommandListParameters& desc);
		Uint64 Imp_ExecuteCommandLists(CommandList* const* commandLists, Uint32 commandListCount, RHICommandQueue queue);//TOOD :

		bool Imp_WaitForIdle(void);
		void Imp_RunGarbageCollection(void);
		bool Imp_QueryFeatureSupport(RHIFeature feature, void* outData, Uint64 outDataSize);
		RHIFormatSupport Imp_QueryFormatSupport(RHIFormat format);
	};

	Device::Device(const DeviceDesc& desc) :
		m_Context{ .Instance{ desc.Instance }, .PhysicalDevice{ desc.PhysicalDevice }, .Device{ desc.Device }, .AllocationCallbacks{ reinterpret_cast<vk::AllocationCallbacks*>(desc.AllocationCallbacks)/*TODO*/ } } {
		if (nullptr != desc.GraphicsQueue)
			this->m_Queues[Tounderlying(RHICommandQueue::Graphics)] = MakeUnique<VulkanQueue>(this->m_Context, RHICommandQueue::Graphics, desc.GraphicsQueue, desc.GraphicsQueueIndex);

		if (nullptr != desc.ComputeQueue)
			this->m_Queues[Tounderlying(RHICommandQueue::Compute)] = MakeUnique<VulkanQueue>(this->m_Context, RHICommandQueue::Compute, desc.ComputeQueue, desc.ComputeQueueIndex);

		if (nullptr != desc.TransferQueue)
			this->m_Queues[Tounderlying(RHICommandQueue::Copy)] = MakeUnique<VulkanQueue>(this->m_Context, RHICommandQueue::Copy, desc.TransferQueue, desc.TransferQueueIndex);

		// maps Vulkan extension strings into the corresponding boolean flags in Device
		const UnorderedMap<String, bool*> extensionStringMap{
			{ "VK_KHR_maintenance1", &this->m_Context.Extensions.KHR_maintenance1 },
			{ "VK_EXT_debug_marker", &this->m_Context.Extensions.EXT_debug_marker },
			{ "VK_EXT_debug_report", &this->m_Context.Extensions.EXT_debug_report },
			{ "VK_EXT_debug_utils", &this->m_Context.Extensions.EXT_debug_utils },
			{ "VK_EXT_conservative_rasterization", &this->m_Context.Extensions.EXT_conservative_rasterization },
			{ "VK_KHR_synchronization2", &this->m_Context.Extensions.KHR_synchronization2 },
			{ "VK_NV_mesh_shader", &this->m_Context.Extensions.NV_mesh_shader },
		};

		// parse the extension/layer lists and figure out which extensions are enabled
		for (Uint64 Index = 0; Index < desc.NumInstanceExtensions; ++Index)
			if (auto ext{ extensionStringMap.find(desc.InstanceExtensions[Index]) }; ext != extensionStringMap.end())
				*(ext->second) = true;

		for (Uint64 Index = 0; Index < desc.NumDeviceExtensions; ++Index)
			if (auto ext = extensionStringMap.find(desc.DeviceExtensions[Index]); ext != extensionStringMap.end())
				*(ext->second) = true;

		void* pNext{ nullptr };
		vk::PhysicalDeviceConservativeRasterizationPropertiesEXT conservativeRasterizationProperties;

		vk::PhysicalDeviceProperties2 deviceProperties2;

		if (this->m_Context.Extensions.EXT_conservative_rasterization) {
			conservativeRasterizationProperties.pNext = pNext;
			pNext = &conservativeRasterizationProperties;
		}

		deviceProperties2.pNext = pNext;

		this->m_Context.PhysicalDevice.getProperties2(&deviceProperties2);

		this->m_Context.PhysicalDeviceProperties = deviceProperties2.properties;
		this->m_Context.ConservativeRasterizationProperties = conservativeRasterizationProperties;

		vk::PipelineCacheCreateInfo pipelineInfo{};
		VULKAN_CHECK(this->m_Context.Device.createPipelineCache(&pipelineInfo, this->m_Context.AllocationCallbacks, &this->m_Context.PipelineCache));

		// Create an empty Vk::DescriptorSetLayout
		vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
		VULKAN_CHECK(this->m_Context.Device.createDescriptorSetLayout(&descriptorSetLayoutInfo, this->m_Context.AllocationCallbacks, &this->m_Context.EmptyDescriptorSetLayout));
	}

	Device::~Device(void) {
		/*if (m_TimerQueryPool){
			m_Context.device.destroyQueryPool(m_TimerQueryPool);
			m_TimerQueryPool = vk::QueryPool();
		}*/

		//TODO : 

		for (auto& e : RHI::Vulkan::g_CommandBuffersInFlight)
			e.clear();
		for (auto& e : RHI::Vulkan::g_CommandBuffersPool)
			e.clear();

		if (nullptr != this->m_Context.PipelineCache) {
			this->m_Context.Device.destroyPipelineCache(this->m_Context.PipelineCache);
			this->m_Context.PipelineCache = nullptr;
		}

		if (nullptr != this->m_Context.EmptyDescriptorSetLayout) {
			this->m_Context.Device.destroyDescriptorSetLayout(this->m_Context.EmptyDescriptorSetLayout);
			this->m_Context.EmptyDescriptorSetLayout = nullptr;
		}
	}

	//Src

	void* Device::MapBuffer(Buffer* buffer, RHICPUAccessMode flags, Uint64 offset, Uint64 size) const {
		ASSERT(RHICPUAccessMode::None != flags);

		// If the buffer has been used in a command list before, wait for that CL to complete
		if (buffer->m_LastUseCommandListID != 0)
			this->m_Queues[Tounderlying(buffer->m_LastUseQueue)]->WaitCommandList(buffer->m_LastUseCommandListID, Max_Uint64);

		vk::AccessFlags accessFlags;

		switch (flags) {
		case RHICPUAccessMode::Read:
			accessFlags = vk::AccessFlagBits::eHostRead;
			break;

		case RHICPUAccessMode::Write:
			accessFlags = vk::AccessFlagBits::eHostWrite;
			break;

		case RHICPUAccessMode::None:
		default:
			ASSERT(false);
			break;
		}

		void* ptr{ nullptr };
		VULKAN_CHECK(this->m_Context.Device.mapMemory(buffer->MemoryResource::Memory, offset, size, vk::MemoryMapFlags{}, &ptr));

		return ptr;
	}

	void Device::CreatePipelineLayout(vk::PipelineLayout& outPipelineLayout, Array<RefCountPtr<BindingLayout>, g_MaxBindingLayoutCount>& outBindingLayouts, Array<Uint32, g_MaxBindingLayoutCount>& outDescriptorSetIdxToBindingIdx, Uint32& outBindingLayoutCount, vk::ShaderStageFlags& outPushConstantVisibility, Span<const RefCountPtr<BindingLayout>> inBindingLayouts) {
		Uint32 numRegularDescriptorSets{ 0 };
		for (const auto& layout : inBindingLayouts)
			numRegularDescriptorSets = Math::Max(numRegularDescriptorSets, layout->m_Desc.RegisterSpace + 1);

		outBindingLayoutCount = numRegularDescriptorSets;

		// Now create the layout
		for (Uint32 Index = 0; Index < numRegularDescriptorSets; ++Index)
			outDescriptorSetIdxToBindingIdx[Index] = Max_Uint32;
		for (Uint32 Index = 0; Index < static_cast<Uint32>(inBindingLayouts.size()); ++Index) {
			BindingLayout* layout{ inBindingLayouts[Index].Get() };

			const Uint32 descriptorSetIdx{ layout->m_Desc.RegisterSpace };

			ASSERT(nullptr == outBindingLayouts[descriptorSetIdx]);

			outBindingLayouts[descriptorSetIdx] = layout;
			outDescriptorSetIdxToBindingIdx[descriptorSetIdx] = Index;
		}

		Array<vk::DescriptorSetLayout, g_MaxBindingLayoutCount> descriptorSetLayouts;
		RemoveCV<decltype(g_MaxBindingLayoutCount)>::type descriptorSetLayoutCount{ 0 };

		Uint32 pushConstantSize{ 0 };
		outPushConstantVisibility = vk::ShaderStageFlagBits{};
		for (const BindingLayout* layout : Span<const RefCountPtr<BindingLayout>>{ outBindingLayouts.data(),numRegularDescriptorSets }) {
			if (nullptr != layout) {
				descriptorSetLayouts[descriptorSetLayoutCount++] = layout->m_DescriptorSetLayout;

				for (const auto& item : Span<const RHIBindingLayoutItem>{ layout->m_Desc.Bindings.data(), layout->m_Desc.BindingCount })
					if (RHIResourceType::PushConstants == item.Type) {
						pushConstantSize = item.ByteSize;
						outPushConstantVisibility = ConvertShaderTypeToShaderStageFlagBits(layout->m_Desc.Visibility);
						// assume there's only one push constant item in all layouts -- the validation layer makes sure of that
						break;
					}
			}
			else
				descriptorSetLayouts[descriptorSetLayoutCount++] = this->m_Context.EmptyDescriptorSetLayout;
		}

		auto pushConstantRange{ vk::PushConstantRange{}
			.setOffset(0)
			.setSize(pushConstantSize)
			.setStageFlags(outPushConstantVisibility)
		};

		auto pipelineLayoutInfo{ vk::PipelineLayoutCreateInfo{}
			.setSetLayoutCount(descriptorSetLayoutCount)
			.setPSetLayouts(descriptorSetLayouts.data())
			.setPushConstantRangeCount(pushConstantSize ? 1 : 0)
			.setPPushConstantRanges(&pushConstantRange)
		};

		VULKAN_CHECK(this->m_Context.Device.createPipelineLayout(&pipelineLayoutInfo, this->m_Context.AllocationCallbacks, &outPipelineLayout));
	}

	void Device::QueueWaitForSemaphore(RHICommandQueue waitQueue, VkSemaphore semaphore, Uint64 value) {
		this->m_Queues[Tounderlying(waitQueue)]->AddWaitSemaphore(semaphore, value);
	}

	void Device::QueueSignalSemaphore(RHICommandQueue executionQueue, VkSemaphore semaphore, Uint64 value) {
		this->m_Queues[Tounderlying(executionQueue)]->AddSignalSemaphore(semaphore, value);
	}

	//Imp 

	RHIObject Device::Imp_GetNativeObject(RHIObjectType objectType)const noexcept {
		switch (objectType) {
			using enum RHIObjectType;
		case VK_Device:return RHIObject{ .Pointer{this->m_Context.Device } };
		case VK_PhysicalDevice:return RHIObject{ .Pointer{this->m_Context.PhysicalDevice } };
		case VK_Instance:return RHIObject{ .Pointer{this->m_Context.Instance } };
		default:return {};
		}
	}

	RefCountPtr<Heap> Device::Imp_CreateHeap(const RHIHeapDesc& desc) {
		vk::MemoryRequirements memoryRequirements{};
		memoryRequirements.memoryTypeBits = Max_Uint32; // just pick whatever fits the property flags
		memoryRequirements.size = desc.Size;

		vk::MemoryPropertyFlags memoryPropertyFlags;
		switch (desc.Type) {
			using enum RHIHeapType;
		case DeviceLocal:
			memoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
			break;

		case Upload:
			memoryPropertyFlags = vk::MemoryPropertyFlagBits::eHostVisible;
			break;

		case Readback:
			memoryPropertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCached;
			break;

		default:
			ASSERT(false);
			break;
		}

		Heap* heap = new Heap{ this->m_Allocator, desc };
		heap->MemoryResource::Managed = true;

		this->m_Allocator.AllocateMemory(heap, memoryRequirements, memoryPropertyFlags);

		/*if (!desc.DebugName.empty())
			this->m_Context.NameVKObject(heap->MemoryResource::Memory, vk::ObjectType::eDeviceMemory, vk::DebugReportObjectTypeEXT::eDeviceMemory, desc.DebugName.c_str());*/

		return RefCountPtr<Heap>::Create(heap);
	}

	RefCountPtr<Texture> Device::Imp_CreateTexture(const RHITextureDesc& desc) {
		Texture* texture{ new Texture{ this->m_Context, this->m_Allocator, desc, ConvertTextureDesc(desc) } };

		ASSERT(nullptr != texture);

		VULKAN_CHECK(this->m_Context.Device.createImage(&texture->m_ImageInfo, this->m_Context.AllocationCallbacks, &texture->m_Image));

		this->m_Context.NameVKObject(texture->m_Image, vk::ObjectType::eImage, vk::DebugReportObjectTypeEXT::eImage, desc.DebugName.c_str());

		if (!desc.IsVirtual) {
			vk::MemoryRequirements memRequirements;
			this->m_Context.Device.getImageMemoryRequirements(texture->m_Image, &memRequirements);

			this->m_Allocator.AllocateMemory(texture, memRequirements, vk::MemoryPropertyFlagBits::eDeviceLocal);

			this->m_Context.Device.bindImageMemory(texture->m_Image, texture->MemoryResource::Memory, 0);

			this->m_Context.NameVKObject(texture->MemoryResource::Memory, vk::ObjectType::eDeviceMemory, vk::DebugReportObjectTypeEXT::eDeviceMemory, desc.DebugName.c_str());
		}

		return RefCountPtr<Texture>::Create(texture);
	}

	RHIMemoryRequirements Device::Imp_Get_TextureMemoryRequirements(Texture* texture) {
		vk::MemoryRequirements vulkanMemReq;
		this->m_Context.Device.getImageMemoryRequirements(texture->m_Image, &vulkanMemReq);

		return RHIMemoryRequirements{
			.Size{ vulkanMemReq.size },
			.Alignment{ vulkanMemReq.alignment }
		};
	}

	bool Device::Imp_BindTextureMemory(Texture* texture, Heap* heap, Uint64 offset) {
		if (nullptr != texture->m_Heap)
			return false;

		if (!texture->m_Desc.IsVirtual)
			return false;

		this->m_Context.Device.bindImageMemory(texture->m_Image, heap->MemoryResource::Memory, offset);

		texture->m_Heap = heap;

		return true;
	}

	RefCountPtr<Texture> Device::Imp_CreateHandleForNativeTexture(RHIObjectType type, RHIObject _texture, const RHITextureDesc& desc) {
		ASSERT(nullptr != _texture.Pointer);
		ASSERT(type == RHIObjectType::VK_Image);

		vk::Image image{ static_cast<VkImage>(_texture.Pointer) };

		Texture* texture{ new Texture{ this->m_Context, this->m_Allocator,desc, ConvertTextureDesc(desc) } };

		texture->m_Image = image;
		texture->MemoryResource::Managed = false;

		return RefCountPtr<Texture>::Create(texture);
	}

	RefCountPtr<StagingTexture> Device::Imp_CreateStagingTexture(const RHITextureDesc& desc, RHICPUAccessMode cpuAccess) {
		ASSERT(RHICPUAccessMode::None != cpuAccess);

		StagingTexture* tex{ new StagingTexture{} };
		tex->m_Desc = desc;
		tex->PopulateSliceRegions();

		RHIBufferDesc bufDesc{
			.ByteSize{  tex->Get_BufferSize() },
			.CPUAccess{ cpuAccess },
		};

		tex->m_Buffer = this->CreateBuffer(bufDesc);

		return RefCountPtr<StagingTexture>::Create(tex);
	}

	void* Device::Imp_MapStagingTexture(StagingTexture* texture, const RHITextureSlice& slice, RHICPUAccessMode cpuAccess, Uint64* outRowPitch) {
		ASSERT(0 == slice.Offset.X);
		ASSERT(0 == slice.Offset.Y);
		ASSERT(RHICPUAccessMode::None != cpuAccess);

		auto resolvedSlice{ slice.Resolve(texture->m_Desc) };

		auto region{ texture->Get_SliceRegion(resolvedSlice.MipLevel, resolvedSlice.ArraySlice, resolvedSlice.Offset.Z) };

		ASSERT(0 == (region.Offset & 0x3)); // per vulkan spec
		ASSERT(region.Size > 0);

		const auto& formatInfo{ Get_RHIFormatInfo(texture->m_Desc.Format) };

		ASSERT(nullptr != outRowPitch);

		auto wInBlocks{ resolvedSlice.Extent.Width / formatInfo.BlockSize };

		*outRowPitch = wInBlocks * formatInfo.BytesPerBlock;

		return this->MapBuffer(texture->m_Buffer, cpuAccess, region.Offset, region.Size);
	}

	void Device::Imp_UnmapStagingTexture(StagingTexture* texture) {
		this->UnmapBuffer(texture->m_Buffer);
	}

	void Device::Imp_Get_TextureTiling(Texture* texture, Uint32* numTiles, RHIPackedMipDesc* desc, RHITileShape* tileShape, Uint32* subresourceTilingsNum, RHISubresourceTiling* subresourceTilings) {
		Uint32 numStandardMips{ 0 };
		Uint32 tileWidth{ 1 };
		Uint32 tileHeight{ 1 };
		Uint32 tileDepth{ 1 };

		{
			auto memoryRequirements{ this->m_Context.Device.getImageSparseMemoryRequirements(texture->m_Image) };
			if (!memoryRequirements.empty())
				numStandardMips = memoryRequirements[0].imageMipTailFirstLod;

			if (nullptr != desc) {
				desc->StandardMipCount = numStandardMips;
				desc->PackedMipCount = texture->m_ImageInfo.mipLevels - memoryRequirements[0].imageMipTailFirstLod;
				desc->StartTileIndexInOverallResource = static_cast<Uint32>(memoryRequirements[0].imageMipTailOffset / Texture::TileByteSize);
				desc->TilesForPackedMipCount = static_cast<Uint32>(memoryRequirements[0].imageMipTailSize / Texture::TileByteSize);
			}
		}

		{
			auto formatProperties = this->m_Context.PhysicalDevice.getSparseImageFormatProperties(texture->m_ImageInfo.format, texture->m_ImageInfo.imageType, texture->m_ImageInfo.samples, texture->m_ImageInfo.usage, texture->m_ImageInfo.tiling);
			if (!formatProperties.empty()) {
				tileWidth = formatProperties[0].imageGranularity.width;
				tileHeight = formatProperties[0].imageGranularity.height;
				tileDepth = formatProperties[0].imageGranularity.depth;
			}

			if (nullptr != tileShape) {
				tileShape->WidthInTexels = tileWidth;
				tileShape->HeightInTexels = tileHeight;
				tileShape->DepthInTexels = tileDepth;
			}
		}

		if (nullptr != subresourceTilingsNum) {
			*subresourceTilingsNum = Math::Min(*subresourceTilingsNum, texture->m_Desc.MipLevelCount);
			Uint32 startTileIndexInOverallResource = 0;

			Uint32 width{ texture->m_Desc.Extent.Width };
			Uint32 height{ texture->m_Desc.Extent.Height };
			Uint32 depth{ texture->m_Desc.Extent.Depth };

			for (Uint32 Index = 0; Index < *subresourceTilingsNum; ++Index) {
				if (Index < numStandardMips) {
					subresourceTilings[Index].WidthInTiles = (width + tileWidth - 1) / tileWidth;
					subresourceTilings[Index].HeightInTiles = (height + tileHeight - 1) / tileHeight;
					subresourceTilings[Index].DepthInTiles = (depth + tileDepth - 1) / tileDepth;
					subresourceTilings[Index].StartTileIndexInOverallResource = startTileIndexInOverallResource;
				}
				else {
					subresourceTilings[Index].WidthInTiles = 0;
					subresourceTilings[Index].HeightInTiles = 0;
					subresourceTilings[Index].DepthInTiles = 0;
					subresourceTilings[Index].StartTileIndexInOverallResource = Max_Uint32;
				}

				width = Math::Max(width / 2, tileWidth);
				height = Math::Max(height / 2, tileHeight);
				depth = Math::Max(depth / 2, tileDepth);

				startTileIndexInOverallResource += subresourceTilings[Index].WidthInTiles * subresourceTilings[Index].HeightInTiles * subresourceTilings[Index].DepthInTiles;
			}
		}

		if (nullptr != numTiles) {
			auto memoryRequirements = this->m_Context.Device.getImageMemoryRequirements(texture->m_Image);
			*numTiles = static_cast<Uint32>(memoryRequirements.size / Texture::TileByteSize);
		}
	}

	void Device::Imp_UpdateTextureTileMappings(Texture* texture, const RHITextureTilesMapping<VulkanTag>* tileMappings, Uint32 numTileMappings, RHICommandQueue executionQueue) {
		Vector<vk::SparseImageMemoryBind> sparseImageMemoryBinds;
		Vector<vk::SparseMemoryBind> sparseMemoryBinds;

		for (Uint64 MapIndex = 0; MapIndex < numTileMappings; ++MapIndex) {
			Uint32 numRegions{ tileMappings[MapIndex].TextureRegionCount };
			vk::DeviceMemory deviceMemory{ nullptr != tileMappings[MapIndex].Heap ? tileMappings[MapIndex].Heap->MemoryResource::Memory : nullptr };

			for (Uint32 RegionIndex = 0; RegionIndex < numRegions; ++RegionIndex) {
				const auto& tiledTextureCoordinate{ tileMappings[MapIndex].TiledTextureCoordinates[RegionIndex] };
				const auto& tiledTextureRegion{ tileMappings[MapIndex].TiledTextureRegions[RegionIndex] };

				if (0 != tiledTextureRegion.TilesCount) {
					sparseMemoryBinds.push_back(vk::SparseMemoryBind{}
						.setResourceOffset(0)
						.setSize(tiledTextureRegion.TilesCount * Texture::TileByteSize)
						.setMemory(deviceMemory)
						.setMemoryOffset(nullptr != deviceMemory ? tileMappings[MapIndex].ByteOffsets[RegionIndex] : 0));
				}
				else {
					auto subresource{ vk::ImageSubresource{}
						.setArrayLayer(tiledTextureCoordinate.ArrayLevel)
						.setMipLevel(tiledTextureCoordinate.MipLevel)
					};

					sparseImageMemoryBinds.push_back(vk::SparseImageMemoryBind{}
						.setSubresource(subresource)
						.setOffset(vk::Offset3D{}.setX(tiledTextureCoordinate.TileCoordinate.Width).setY(tiledTextureCoordinate.TileCoordinate.Height).setZ(tiledTextureCoordinate.TileCoordinate.Depth))
						.setExtent(vk::Extent3D{ tiledTextureRegion.TileSize.Width,tiledTextureRegion.TileSize.Height,tiledTextureRegion.TileSize.Depth })
						.setMemory(deviceMemory)
						.setMemoryOffset(nullptr != deviceMemory ? tileMappings[MapIndex].ByteOffsets[RegionIndex] : 0));
				}
			}
		}

		vk::BindSparseInfo bindSparseInfo{};

		vk::SparseImageMemoryBindInfo sparseImageMemoryBindInfo;
		if (!sparseImageMemoryBinds.empty()) {
			sparseImageMemoryBindInfo.setImage(texture->m_Image);
			sparseImageMemoryBindInfo.setBinds(sparseImageMemoryBinds);
			bindSparseInfo.setImageBinds(sparseImageMemoryBindInfo);
		}

		vk::SparseImageOpaqueMemoryBindInfo sparseImageOpaqueMemoryBindInfo;
		if (!sparseMemoryBinds.empty()) {
			sparseImageOpaqueMemoryBindInfo.setImage(texture->m_Image);
			sparseImageOpaqueMemoryBindInfo.setBinds(sparseMemoryBinds);
			bindSparseInfo.setImageOpaqueBinds(sparseImageOpaqueMemoryBindInfo);
		}

		this->m_Queues[Tounderlying(executionQueue)]->Get_VkQueue().bindSparse(bindSparseInfo, vk::Fence{});
	}

	RefCountPtr<Buffer> Device::Imp_CreateBuffer(const RHIBufferDesc& desc) {
		ASSERT(!(desc.IsVolatile && desc.MaxVersions == 0));
		ASSERT(!(desc.IsVolatile && !desc.IsConstantBuffer));
		ASSERT(0 != desc.ByteSize);

		Buffer* buffer{ new Buffer{this->m_Context, this->m_Allocator, desc } };

		vk::BufferUsageFlags usageFlags{ vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst };

		if (desc.IsVertexBuffer)
			usageFlags |= vk::BufferUsageFlagBits::eVertexBuffer;

		if (desc.IsIndexBuffer)
			usageFlags |= vk::BufferUsageFlagBits::eIndexBuffer;

		if (desc.IsDrawIndirectArgs)
			usageFlags |= vk::BufferUsageFlagBits::eIndirectBuffer;

		if (desc.IsConstantBuffer)
			usageFlags |= vk::BufferUsageFlagBits::eUniformBuffer;

		if (desc.StructStride != 0 || desc.CanHaveUAVs || desc.CanHaveRawViews)
			usageFlags |= vk::BufferUsageFlagBits::eStorageBuffer;

		if (desc.CanHaveTypedViews)
			usageFlags |= vk::BufferUsageFlagBits::eUniformTexelBuffer;

		if (desc.CanHaveTypedViews && desc.CanHaveUAVs)
			usageFlags |= vk::BufferUsageFlagBits::eStorageTexelBuffer;

		Uint64 size{ desc.ByteSize };

		if (desc.IsVolatile) {
			ASSERT(!desc.IsVirtual);

			Uint64 alignment{ this->m_Context.PhysicalDeviceProperties.limits.minUniformBufferOffsetAlignment };

			Uint64 atomSize{ this->m_Context.PhysicalDeviceProperties.limits.nonCoherentAtomSize };
			alignment = Math::Max(alignment, atomSize);

			ASSERT(0 == (alignment & (alignment - 1))); // check if it's a power of 2

			size = (size + alignment - 1) & ~(alignment - 1);
			buffer->m_Desc.ByteSize = size;

			size *= desc.MaxVersions;

			buffer->m_VersionTracking.resize(desc.MaxVersions, 0);

			buffer->m_Desc.CPUAccess = RHICPUAccessMode::Write; // to get the right memory type allocated
		}
		else if (desc.ByteSize < 65536)
			size = (size + 3) & ~3ull;
		// vulkan allows for <= 64kb buffer updates to be done inline via vkCmdUpdateBuffer,
		// but the data size must always be a multiple of 4
		// enlarge the buffer slightly to allow for this

		auto bufferInfo{ vk::BufferCreateInfo{}
			.setSize(size)
			.setUsage(usageFlags)
			.setSharingMode(vk::SharingMode::eExclusive)
		};

		VULKAN_CHECK(this->m_Context.Device.createBuffer(&bufferInfo, this->m_Context.AllocationCallbacks, &buffer->m_Buffer));

		//this->m_Context.NameVKObject(VkBuffer(buffer->m_Buffer), vk::ObjectType::eBuffer, vk::DebugReportObjectTypeEXT::eBuffer, desc.DebugName.c_str());

		if (!desc.IsVirtual) {
			// figure out memory requirements
			vk::MemoryRequirements memRequirements;
			this->m_Context.Device.getBufferMemoryRequirements(buffer->m_Buffer, &memRequirements);

			this->m_Allocator.AllocateMemory(buffer, memRequirements, PickBufferMemoryProperties(buffer->m_Desc));

			this->m_Context.Device.bindBufferMemory(buffer->m_Buffer, buffer->MemoryResource::Memory, 0);

			//this->m_Context.NameVKObject(buffer->MemoryResource::Memory, vk::ObjectType::eDeviceMemory, vk::DebugReportObjectTypeEXT::eDeviceMemory, desc.DebugName.c_str());

			if (desc.IsVolatile) {
				buffer->m_MappedMemory = this->m_Context.Device.mapMemory(buffer->MemoryResource::Memory, 0, size);
				ASSERT(nullptr != buffer->m_MappedMemory);
			}
		}

		return RefCountPtr<Buffer>::Create(buffer);
	}

	void* Device::Imp_MapBuffer(Buffer* buffer, RHICPUAccessMode CPUaccess) {
		return this->MapBuffer(buffer, CPUaccess, 0, buffer->m_Desc.ByteSize);
	}

	void Device::Imp_UnmapBuffer(Buffer* buffer) {
		this->m_Context.Device.unmapMemory(buffer->MemoryResource::Memory);
	}

	RHIMemoryRequirements Device::Imp_Get_BufferMemoryRequirements(Buffer* buffer) {
		vk::MemoryRequirements vulkanMemReq;
		this->m_Context.Device.getBufferMemoryRequirements(buffer->m_Buffer, &vulkanMemReq);

		return RHIMemoryRequirements{
			.Size{ vulkanMemReq.size },
			.Alignment{ vulkanMemReq.alignment }
		};
	}

	bool Device::Imp_BindBufferMemory(Buffer* buffer, Heap* heap, Uint64 offset) {
		ASSERT(nullptr == buffer->m_Heap);
		ASSERT(!buffer->m_Desc.IsVirtual);

		this->m_Context.Device.bindBufferMemory(buffer->m_Buffer, heap->MemoryResource::Memory, offset);

		buffer->m_Heap = heap;

		return true;
	}

	RefCountPtr<Shader> Device::Imp_CreateShader(const RHIShaderDesc& desc, const void* binary, Uint64 binarySize) {
		Shader* shader{ new Shader{ this->m_Context, desc } };

		shader->m_StageFlagBits = ConvertShaderTypeToShaderStageFlagBits(desc.ShaderType);

		auto shaderInfo{ vk::ShaderModuleCreateInfo{}
			.setCodeSize(binarySize)
			.setPCode(static_cast<const Uint32*>(binary))
		};

		VULKAN_CHECK(this->m_Context.Device.createShaderModule(&shaderInfo, this->m_Context.AllocationCallbacks, &shader->m_ShaderModule));

		this->m_Context.NameVKObject(VkShaderModule(shader->m_ShaderModule), vk::ObjectType::eShaderModule, vk::DebugReportObjectTypeEXT::eShaderModule, desc.DebugName.c_str());

		return RefCountPtr<Shader>::Create(shader);
	}

	RefCountPtr<Sampler> Device::Imp_CreateSampler(const RHISamplerDesc& desc) {
		Sampler* sampler{ new Sampler{ this->m_Context, desc } };

		const bool anisotropyEnable{ desc.MaxAnisotropy > 1.0f };

		sampler->m_SamplerInfo = vk::SamplerCreateInfo{}
			.setMagFilter(desc.MagFilter ? vk::Filter::eLinear : vk::Filter::eNearest)
			.setMinFilter(desc.MinFilter ? vk::Filter::eLinear : vk::Filter::eNearest)
			.setMipmapMode(desc.MipFilter ? vk::SamplerMipmapMode::eLinear : vk::SamplerMipmapMode::eNearest)
			.setAddressModeU(ConvertSamplerAddressMode(desc.AddressModeU))
			.setAddressModeV(ConvertSamplerAddressMode(desc.AddressModeV))
			.setAddressModeW(ConvertSamplerAddressMode(desc.AddressModeW))
			.setMipLodBias(desc.MipBias)
			.setAnisotropyEnable(anisotropyEnable)
			.setMaxAnisotropy(anisotropyEnable ? desc.MaxAnisotropy : 1.f)
			.setCompareEnable(desc.ReductionType == RHISamplerReductionType::Comparison)
			.setCompareOp(vk::CompareOp::eLess)
			.setMinLod(0.f)
			.setMaxLod(Max_Float)
			.setBorderColor(PickSamplerBorderColor(desc.BorderColor));

		if (desc.ReductionType == RHISamplerReductionType::Minimum || desc.ReductionType == RHISamplerReductionType::Maximum) {
			auto samplerReductionCreateInfo{ vk::SamplerReductionModeCreateInfoEXT{}.setReductionMode(desc.ReductionType == RHISamplerReductionType::Maximum ? vk::SamplerReductionModeEXT::eMax : vk::SamplerReductionModeEXT::eMin) };

			sampler->m_SamplerInfo.setPNext(&samplerReductionCreateInfo);
		}

		VULKAN_CHECK(this->m_Context.Device.createSampler(&sampler->m_SamplerInfo, this->m_Context.AllocationCallbacks, &sampler->m_Sampler));

		return RefCountPtr<Sampler>::Create(sampler);
	}

	RefCountPtr<InputLayout> Device::Imp_CreateInputLayout(const RHIVertexAttributeDesc* attributeDesc, Uint32 attributeCount) {
		InputLayout* layout{ new InputLayout{} };

		Uint32 total_attribute_array_size{ 0 };

		// collect all buffer bindings
		UnorderedMap<Uint32, vk::VertexInputBindingDescription> bindingMap;
		for (Uint32 Index = 0; Index < attributeCount; ++Index) {
			const auto& desc{ attributeDesc[Index] };

			ASSERT(desc.ArrayCount > 0);

			total_attribute_array_size += desc.ArrayCount;

			if (auto It{ bindingMap.find(desc.BufferIndex) }; It == bindingMap.end()) {
				bindingMap[desc.BufferIndex] = vk::VertexInputBindingDescription{}
					.setBinding(desc.BufferIndex)
					.setStride(desc.ElementStride)
					.setInputRate(desc.IsInstanced ? vk::VertexInputRate::eInstance : vk::VertexInputRate::eVertex);
			}
			else {
				ASSERT(bindingMap[desc.BufferIndex].stride == desc.ElementStride);
				ASSERT(bindingMap[desc.BufferIndex].inputRate == (desc.IsInstanced ? vk::VertexInputRate::eInstance : vk::VertexInputRate::eVertex));
			}
		}

		for (const auto& b : bindingMap)
			layout->m_BindingDesc.push_back(b.second);

		// build attribute descriptions
		layout->m_InputDesc.resize(attributeCount);
		layout->m_AttributeDesc.resize(total_attribute_array_size);

		Uint32 attributeLocation{ 0 };
		for (Uint32 Index = 0; Index < attributeCount; ++Index) {
			const auto& in{ attributeDesc[Index] };
			layout->m_InputDesc[Index] = in;

			Uint32 element_size_bytes{ Get_RHIFormatInfo(in.Format).BytesPerBlock };

			Uint32 bufferOffset{ 0 };

			for (Uint32 slot = 0; slot < in.ArrayCount; ++slot) {
				auto& outAttrib{ layout->m_AttributeDesc[attributeLocation] };

				outAttrib.location = attributeLocation;
				outAttrib.binding = in.BufferIndex;
				outAttrib.format = vk::Format{ ConvertFormat(in.Format) };
				outAttrib.offset = bufferOffset + in.Offset;
				bufferOffset += element_size_bytes;

				++attributeLocation;
			}
		}

		return RefCountPtr<InputLayout>::Create(layout);
	}

	RefCountPtr<EventQuery> Device::Imp_CreateEventQuery(void) {
		return RefCountPtr<EventQuery>::Create(new EventQuery{});
	}

	void Device::Imp_SetEventQuery(EventQuery* query, RHICommandQueue queue) {
		query->m_Queue = queue;
		query->m_CommandListID = this->m_Queues[Tounderlying(queue)]->Get_LastSubmittedID();
	}

	bool Device::Imp_PollEventQuery(EventQuery* query) {
		return this->m_Queues[Tounderlying(query->m_Queue)]->PollCommandList(query->m_CommandListID);
	}

	void Device::Imp_WaitEventQuery(EventQuery* query) {
		if (query->m_CommandListID == 0)
			return;

		ASSERT(this->m_Queues[Tounderlying(query->m_Queue)]->WaitCommandList(query->m_CommandListID, Max_Uint64));
	}

	void Device::Imp_ResetEventQuery(EventQuery* query) {
		query->m_CommandListID = 0;
	}

	RefCountPtr<TimerQuery> Device::Imp_CreateTimerQuery(void) {
		ASSERT(false);
		return nullptr;
	}

	bool Device::Imp_PollTimerQuery(TimerQuery* query) {
		ASSERT(false);
		return false;
	}

	float Device::Imp_Get_TimerQueryTime(TimerQuery* query) {
		ASSERT(false);
		return 0.f;
	}

	void Device::Imp_ResetTimerQuery(TimerQuery* query) {
		ASSERT(false);
	}

	RefCountPtr<FrameBuffer> Device::Imp_CreateFrameBuffer(const RHIFrameBufferDesc<VulkanTag>& desc) {
		FrameBuffer* fb{ new FrameBuffer{ this->m_Context } };
		fb->m_Desc = desc;
		fb->m_Info = RHIFrameBufferInfo<VulkanTag>::Build(desc);

		Array<vk::AttachmentDescription2, g_MaxRenderTargetCount + 1> attachments;
		Array<vk::AttachmentReference2, g_MaxRenderTargetCount> colorAttachmentRefs;
		vk::AttachmentReference2 depthAttachmentRef;

		Array<vk::ImageView, g_MaxRenderTargetCount + 1> attachmentViews;

		Uint32 numArraySlices{ 0 };

		for (Uint32 Index = 0; Index < desc.ColorAttachmentCount; ++Index) {
			const auto& rt{ desc.ColorAttachments[Index] };
			Texture* t{ rt.Texture };

			ASSERT(fb->m_Info.Width == Math::Max(t->m_Desc.Extent.Width >> rt.Subresources.BaseMipLevel, 1u));
			ASSERT(fb->m_Info.Height == Math::Max(t->m_Desc.Extent.Height >> rt.Subresources.BaseMipLevel, 1u));

			const vk::Format attachmentFormat{ rt.Format == RHIFormat::UNKNOWN ? t->m_ImageInfo.format : vk::Format{ ConvertFormat(rt.Format) } };

			attachments[Index] = vk::AttachmentDescription2{}
				.setFormat(attachmentFormat)
				.setSamples(t->m_ImageInfo.samples)
				.setLoadOp(vk::AttachmentLoadOp::eLoad)
				.setStoreOp(vk::AttachmentStoreOp::eStore)
				.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
				.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);

			colorAttachmentRefs[Index] = vk::AttachmentReference2{}
				.setAttachment(Index)
				.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

			RHITextureSubresourceSet subresources{ rt.Subresources.Resolve(t->m_Desc, true) };

			const auto dimension{ GetDimensionForFramebuffer(t->m_Desc.Dimension, subresources.ArraySliceCount > 1) };

			const auto& view{ t->Get_SubresourceView(subresources, dimension, rt.Format, vk::ImageUsageFlagBits::eColorAttachment) };
			attachmentViews[Index] = view.View;

			fb->m_Resources.push_back(rt.Texture);

			ASSERT(0 == numArraySlices || (0 != numArraySlices && numArraySlices == subresources.ArraySliceCount));

			if (0 == numArraySlices)
				numArraySlices = subresources.ArraySliceCount;
		}

		// add depth/stencil attachment if present
		if (desc.DepthStencilAttachment.Is_Valid()) {
			const auto& att{ desc.DepthStencilAttachment };

			Texture* texture{ att.Texture };

			ASSERT(fb->m_Info.Width == Math::Max(texture->m_Desc.Extent.Width >> att.Subresources.BaseMipLevel, 1u));
			ASSERT(fb->m_Info.Height == Math::Max(texture->m_Desc.Extent.Height >> att.Subresources.BaseMipLevel, 1u));

			vk::ImageLayout depthLayout{ vk::ImageLayout::eDepthStencilAttachmentOptimal };
			if (att.IsReadOnly)
				depthLayout = vk::ImageLayout::eDepthStencilReadOnlyOptimal;

			attachments[desc.ColorAttachmentCount] = vk::AttachmentDescription2{}
				.setFormat(texture->m_ImageInfo.format)
				.setSamples(texture->m_ImageInfo.samples)
				.setLoadOp(vk::AttachmentLoadOp::eLoad)
				.setStoreOp(vk::AttachmentStoreOp::eStore)
				.setInitialLayout(depthLayout)
				.setFinalLayout(depthLayout);

			depthAttachmentRef = vk::AttachmentReference2{}
				.setAttachment(desc.ColorAttachmentCount)
				.setLayout(depthLayout);

			auto subresources{ att.Subresources.Resolve(texture->m_Desc, true) };

			auto dimension{ GetDimensionForFramebuffer(texture->m_Desc.Dimension, subresources.ArraySliceCount > 1) };

			const auto& view{ texture->Get_SubresourceView(subresources, dimension, att.Format, vk::ImageUsageFlagBits::eDepthStencilAttachment) };
			attachmentViews[desc.ColorAttachmentCount] = view.View;

			fb->m_Resources.push_back(att.Texture);

			ASSERT(0 == numArraySlices || (0 != numArraySlices && numArraySlices == subresources.ArraySliceCount));

			if (0 == numArraySlices)
				numArraySlices = subresources.ArraySliceCount;
		}

		auto subpass{ vk::SubpassDescription2{}
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachmentCount(desc.ColorAttachmentCount)
			.setPColorAttachments(colorAttachmentRefs.data())
			.setPDepthStencilAttachment(desc.DepthStencilAttachment.Is_Valid() ? &depthAttachmentRef : nullptr)
		};

		auto renderPassInfo{ vk::RenderPassCreateInfo2{}
			.setAttachmentCount(desc.ColorAttachmentCount + desc.DepthStencilAttachment.Is_Valid())
			.setPAttachments(attachments.data())
			.setSubpassCount(1)
			.setPSubpasses(&subpass)
		};

		VULKAN_CHECK(this->m_Context.Device.createRenderPass2(&renderPassInfo, this->m_Context.AllocationCallbacks, &fb->m_RenderPass));

		// set up the framebuffer object
		auto framebufferInfo{ vk::FramebufferCreateInfo{}
			.setRenderPass(fb->m_RenderPass)
			.setAttachmentCount(desc.ColorAttachmentCount + desc.DepthStencilAttachment.Is_Valid())
			.setPAttachments(attachmentViews.data())
			.setWidth(fb->m_Info.Width)
			.setHeight(fb->m_Info.Height)
			.setLayers(numArraySlices)
		};

		VULKAN_CHECK(this->m_Context.Device.createFramebuffer(&framebufferInfo, this->m_Context.AllocationCallbacks, &fb->m_FrameBuffer));

		return RefCountPtr<FrameBuffer>::Create(fb);
	}

	RefCountPtr<GraphicsPipeline> Device::Imp_CreateGraphicsPipeline(const RHIGraphicsPipelineDesc<VulkanTag>& desc, FrameBuffer* framebuffer) {
		InputLayout* inputLayout{ desc.InputLayout.Get() };

		GraphicsPipeline* pso{ new GraphicsPipeline{ this->m_Context } };
		pso->m_Desc = desc;
		pso->m_FrameBufferInfo = framebuffer->m_Info;

		auto MakeShaderStageCreateInfo = [](Shader* shader) {
			return vk::PipelineShaderStageCreateInfo{}
				.setStage(shader->m_StageFlagBits)
				.setModule(shader->m_ShaderModule)
				.setPName(shader->m_Desc.EntryName.c_str());
			};

		Vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		Vector<Uint32> specData;

		// Set up shader stages
		if (nullptr != desc.VS) {
			shaderStages.push_back(MakeShaderStageCreateInfo(desc.VS));
			pso->m_ShaderMask |= RHIShaderType::Vertex;
		}

		if (nullptr != desc.HS) {
			shaderStages.push_back(MakeShaderStageCreateInfo(desc.HS));
			pso->m_ShaderMask |= RHIShaderType::Hull;
		}

		if (nullptr != desc.DS) {
			shaderStages.push_back(MakeShaderStageCreateInfo(desc.DS));
			pso->m_ShaderMask |= RHIShaderType::Domain;
		}

		if (nullptr != desc.GS) {
			shaderStages.push_back(MakeShaderStageCreateInfo(desc.GS));
			pso->m_ShaderMask |= RHIShaderType::Geometry;
		}

		if (nullptr != desc.PS) {
			shaderStages.push_back(MakeShaderStageCreateInfo(desc.PS));
			pso->m_ShaderMask |= RHIShaderType::Pixel;
		}

		// set up vertex input state
		auto vertexInput{ vk::PipelineVertexInputStateCreateInfo{} };
		if (nullptr != inputLayout) {
			vertexInput
				.setVertexBindingDescriptionCount(static_cast<Uint32>(inputLayout->m_BindingDesc.size()))
				.setPVertexBindingDescriptions(inputLayout->m_BindingDesc.data())
				.setVertexAttributeDescriptionCount(static_cast<Uint32>(inputLayout->m_AttributeDesc.size()))
				.setPVertexAttributeDescriptions(inputLayout->m_AttributeDesc.data());
		}

		auto inputAssembly{
			vk::PipelineInputAssemblyStateCreateInfo{}
			.setTopology(ConvertPrimitiveTopology(desc.PrimType))
		};

		// fixed function state
		const auto& rasterState{ desc.RenderState.RasterState };
		const auto& depthStencilState{ desc.RenderState.DepthStencilState };
		const auto& blendState{ desc.RenderState.BlendState };

		auto viewportState{ vk::PipelineViewportStateCreateInfo{}
			.setViewportCount(1)
			.setScissorCount(1)
		};

		auto rasterizer{ vk::PipelineRasterizationStateCreateInfo{}
			// .setDepthClampEnable(??)
			// .setRasterizerDiscardEnable(??)
			.setPolygonMode(ConvertFillMode(rasterState.FillMode))
			.setCullMode(ConvertCullMode(rasterState.CullMode))
			.setFrontFace(rasterState.FrontCounterClockwise ? vk::FrontFace::eCounterClockwise : vk::FrontFace::eClockwise)
			.setDepthBiasEnable(rasterState.DepthBias != 0 ? true : false)
			.setDepthBiasConstantFactor(static_cast<float>(rasterState.DepthBias))
			.setDepthBiasClamp(rasterState.DepthBiasClamp)
			.setDepthBiasSlopeFactor(rasterState.SlopeScaledDepthBias)
			.setLineWidth(1.0f)
		};

		// Conservative raster state
		auto conservativeRasterState{ vk::PipelineRasterizationConservativeStateCreateInfoEXT{}
			.setConservativeRasterizationMode(vk::ConservativeRasterizationModeEXT::eOverestimate)
		};
		if (rasterState.ConservativeRasterEnable)
			rasterizer.setPNext(&conservativeRasterState);

		auto multisample{ vk::PipelineMultisampleStateCreateInfo{}
			.setRasterizationSamples(static_cast<vk::SampleCountFlagBits>(framebuffer->m_Info.SampleCount))
			.setAlphaToCoverageEnable(blendState.AlphaToCoverageEnable)
		};

		auto depthStencil{ vk::PipelineDepthStencilStateCreateInfo{}
			.setDepthTestEnable(depthStencilState.DepthTestEnable)
			.setDepthWriteEnable(depthStencilState.DepthWriteEnable)
			.setDepthCompareOp(ConvertCompareOp(depthStencilState.DepthFunc))
			.setStencilTestEnable(depthStencilState.StencilEnable)
			.setFront(ConvertStencilState(depthStencilState, depthStencilState.FrontFaceStencil))
			.setBack(ConvertStencilState(depthStencilState, depthStencilState.BackFaceStencil))
		};

		this->CreatePipelineLayout(pso->m_PipelineLayout, pso->m_PipelineBindingLayouts, pso->m_DescriptorSetIdxToBindingIdx, pso->m_BindingLayoutCount, pso->m_PushConstantVisibility, Span<const RefCountPtr<BindingLayout>>{ desc.BindingLayouts.data(), desc.BindingLayoutCount });

		Array<vk::PipelineColorBlendAttachmentState, g_MaxRenderTargetCount> colorBlendAttachments;

		for (Uint32 Index = 0; Index < framebuffer->m_Desc.ColorAttachmentCount; ++Index)
			colorBlendAttachments[Index] = ConvertBlendState(blendState.RenderTargets[Index]);

		auto colorBlend = vk::PipelineColorBlendStateCreateInfo{}
			.setAttachmentCount(framebuffer->m_Desc.ColorAttachmentCount)
			.setPAttachments(colorBlendAttachments.data());

		pso->m_UsesBlendConstants = blendState.Is_UsesConstantColor(framebuffer->m_Desc.ColorAttachmentCount);

		Array<vk::DynamicState, 4> dynamicStates{
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor
		};
		Uint32 dynamicStateCount{ 2 };

		if (pso->m_UsesBlendConstants)
			dynamicStates[dynamicStateCount++] = vk::DynamicState::eBlendConstants;
		if (pso->m_Desc.RenderState.DepthStencilState.DynamicStencilRef)
			dynamicStates[dynamicStateCount++] = vk::DynamicState::eStencilReference;

		auto dynamicStateInfo{ vk::PipelineDynamicStateCreateInfo{}
			.setDynamicStateCount(dynamicStateCount)
			.setPDynamicStates(dynamicStates.data())
		};

		auto pipelineInfo{ vk::GraphicsPipelineCreateInfo{}
			.setStageCount(static_cast<Uint32>(shaderStages.size()))
			.setPStages(shaderStages.data())
			.setPVertexInputState(&vertexInput)
			.setPInputAssemblyState(&inputAssembly)
			.setPViewportState(&viewportState)
			.setPRasterizationState(&rasterizer)
			.setPMultisampleState(&multisample)
			.setPDepthStencilState(&depthStencil)
			.setPColorBlendState(&colorBlend)
			.setPDynamicState(&dynamicStateInfo)
			.setLayout(pso->m_PipelineLayout)
			.setRenderPass(framebuffer->m_RenderPass)
			.setSubpass(0)//TODO :
			.setBasePipelineHandle(nullptr)
			.setBasePipelineIndex(-1)
			.setPTessellationState(nullptr)
		};

		vk::PipelineTessellationStateCreateInfo tessellationState{};

		if (RHIPrimitiveType::PatchList == desc.PrimType) {
			tessellationState.setPatchControlPoints(desc.PatchControlPoints);
			pipelineInfo.setPTessellationState(&tessellationState);
		}

		VULKAN_CHECK(this->m_Context.Device.createGraphicsPipelines(this->m_Context.PipelineCache, 1, &pipelineInfo, this->m_Context.AllocationCallbacks, &pso->m_Pipeline));

		return RefCountPtr<GraphicsPipeline>::Create(pso);
	}

	RefCountPtr<ComputePipeline> Device::Imp_CreateComputePipeline(const RHIComputePipelineDesc<VulkanTag>& desc) {
		ComputePipeline* pso{ new ComputePipeline{ this->m_Context } };
		pso->m_Desc = desc;

		this->CreatePipelineLayout(pso->m_PipelineLayout, pso->m_PipelineBindingLayouts, pso->m_DescriptorSetIdxToBindingIdx, pso->m_BindingLayoutCount, pso->m_PushConstantVisibility, Span<const RefCountPtr<BindingLayout>>{ desc.BindingLayouts.data(), desc.BindingLayoutCount });

		auto shaderStageInfo{ vk::PipelineShaderStageCreateInfo{}
			.setStage(desc.CS->m_StageFlagBits)
			.setModule(desc.CS->m_ShaderModule)
			.setPName(desc.CS->m_Desc.EntryName.c_str())
		};

		auto pipelineInfo{ vk::ComputePipelineCreateInfo{}
			.setStage(shaderStageInfo)
			.setLayout(pso->m_PipelineLayout)
		};

		VULKAN_CHECK(this->m_Context.Device.createComputePipelines(this->m_Context.PipelineCache, 1, &pipelineInfo, this->m_Context.AllocationCallbacks, &pso->m_Pipeline));

		return RefCountPtr<ComputePipeline>::Create(pso);
	}

	RefCountPtr<BindingLayout> Device::Imp_CreateBindingLayout(const RHIBindingLayoutDesc& desc) {
		BindingLayout* ret{ new BindingLayout{ this->m_Context, desc } };

		ret->Bake();

		return RefCountPtr<BindingLayout>::Create(ret);
	}

	RefCountPtr<BindingSet> Device::Imp_CreateBindingSet(const RHIBindingSetDesc<VulkanTag>& desc, BindingLayout* layout) {
		BindingSet* ret{ new BindingSet{ this->m_Context } };
		ret->m_Desc = desc;
		ret->m_Layout = layout;

		// create descriptor pool to allocate a descriptor from
		auto poolInfo{ vk::DescriptorPoolCreateInfo{}
			.setPoolSizeCount(static_cast<Uint32>(layout->m_DescriptorPoolSizeInfo.size()))
			.setPPoolSizes(layout->m_DescriptorPoolSizeInfo.data())
			.setMaxSets(1)
		};

		VULKAN_CHECK(this->m_Context.Device.createDescriptorPool(&poolInfo, this->m_Context.AllocationCallbacks, &ret->m_DescriptorPool));

		// create the descriptor set
		auto descriptorSetAllocInfo{ vk::DescriptorSetAllocateInfo{}
			.setDescriptorPool(ret->m_DescriptorPool)
			.setDescriptorSetCount(1)
			.setPSetLayouts(&layout->m_DescriptorSetLayout)
		};

		VULKAN_CHECK(this->m_Context.Device.allocateDescriptorSets(&descriptorSetAllocInfo, &ret->m_DescriptorSet));

		// collect all of the descriptor write data
		Array<vk::DescriptorImageInfo, g_MaxBindingsPerLayout> descriptorImageInfo;
		RemoveCV<decltype(g_MaxBindingsPerLayout)>::type descriptorImageInfoCount{ 0 };

		Array<vk::DescriptorBufferInfo, g_MaxBindingsPerLayout> descriptorBufferInfo;
		RemoveCV<decltype(g_MaxBindingsPerLayout)>::type descriptorBufferInfoCount{ 0 };

		Array<vk::WriteDescriptorSet, g_MaxBindingsPerLayout> descriptorWriteInfo;
		RemoveCV<decltype(g_MaxBindingsPerLayout)>::type descriptorWriteInfoCount{ 0 };

		// generates a vk::WriteDescriptorSet struct in descriptorWriteInfo
		auto generateWriteDescriptorData = [&](Uint32 bindingLocation, vk::DescriptorType descriptorType, vk::DescriptorImageInfo* imageInfo, vk::DescriptorBufferInfo* bufferInfo, vk::BufferView* bufferView, const void* pNext = nullptr) {
			descriptorWriteInfo[descriptorWriteInfoCount++] = vk::WriteDescriptorSet{}
				.setDstSet(ret->m_DescriptorSet)
				.setDstBinding(bindingLocation)
				.setDstArrayElement(0)
				.setDescriptorCount(1)
				.setDescriptorType(descriptorType)
				.setPImageInfo(imageInfo)
				.setPBufferInfo(bufferInfo)
				.setPTexelBufferView(bufferView)
				.setPNext(pNext);
			};

		for (Uint32 bindingIndex = 0; bindingIndex < desc.BindingCount; ++bindingIndex) {
			const auto& binding{ desc.Bindings[bindingIndex] };
			const auto& layoutBinding{ layout->m_VulkanLayoutBindings[bindingIndex] };

			if (nullptr != GetIf<Nullptr_T>(&binding.ResourcePtr))//TODO :
				continue;

			ret->m_Resources.push_back(binding.ResourcePtr); // keep a strong reference to the resource

			switch (binding.Type) {
				using enum RHIResourceType;
			case Texture_SRV: {
				Texture* texture{ Get<RefCountPtr<Texture>>(binding.ResourcePtr).Get() };

				const auto subresource{ binding.Subresources.Resolve(texture->m_Desc, false) };
				const auto textureViewType{ Get_TextureViewType(binding.Format, texture->m_Desc.Format) };
				auto& view{ texture->Get_SubresourceView(subresource, binding.Dimension, binding.Format, vk::ImageUsageFlagBits::eSampled, textureViewType) };

				descriptorImageInfo[descriptorImageInfoCount++] = vk::DescriptorImageInfo{}
					.setImageView(view.View)
					.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

				generateWriteDescriptorData(layoutBinding.binding, layoutBinding.descriptorType, &descriptorImageInfo[descriptorImageInfoCount - 1], nullptr, nullptr);

				if (RHIResourceState::Unknown == texture->m_StateExtension.PermanentState)
					ret->m_BindingsThatNeedTransitions.push_back(static_cast<Uint16>(bindingIndex));
				else
					ASSERT(RHIResourceState::ShaderResource == (texture->m_StateExtension.PermanentState & RHIResourceState::ShaderResource));//TODO :
			}
							break;

			case Texture_UAV: {
				Texture* texture{ Get<RefCountPtr<Texture>>(binding.ResourcePtr).Get() };

				const auto subresource{ binding.Subresources.Resolve(texture->m_Desc, true) };
				const auto textureViewType{ Get_TextureViewType(binding.Format, texture->m_Desc.Format) };
				auto& view{ texture->Get_SubresourceView(subresource, binding.Dimension, binding.Format, vk::ImageUsageFlagBits::eStorage, textureViewType) };

				descriptorImageInfo[descriptorImageInfoCount++] = vk::DescriptorImageInfo{}
					.setImageView(view.View)
					.setImageLayout(vk::ImageLayout::eGeneral);

				generateWriteDescriptorData(layoutBinding.binding, layoutBinding.descriptorType, &descriptorImageInfo[descriptorImageInfoCount - 1], nullptr, nullptr);

				if (RHIResourceState::Unknown == texture->m_StateExtension.PermanentState)
					ret->m_BindingsThatNeedTransitions.push_back(static_cast<Uint16>(bindingIndex));
				else
					ASSERT(RHIResourceState::UnorderedAccess == (texture->m_StateExtension.PermanentState & RHIResourceState::UnorderedAccess));//TODO :
			}
							break;

			case TypedBuffer_SRV:case TypedBuffer_UAV: {
				Buffer* buffer{ Get<RefCountPtr<Buffer>>(binding.ResourcePtr).Get() };

				ASSERT(buffer->m_Desc.CanHaveTypedViews);

				const bool isUAV{ RHIResourceType::TypedBuffer_UAV == binding.Type };

				ASSERT((!isUAV) || buffer->m_Desc.CanHaveUAVs);

				const RHIFormat format{ RHIFormat::UNKNOWN != binding.Format ? binding.Format : buffer->m_Desc.Format };
				auto vkformat{ ConvertFormat(format) };

				const auto range{ binding.Range.Resolve(buffer->m_Desc) };

				Uint64 viewInfoHash{ 0 };
				viewInfoHash = HashCombine(viewInfoHash, Hash<Uint64>{}(range.Offset));
				viewInfoHash = HashCombine(viewInfoHash, Hash<Uint64>{}(range.ByteSize));
				viewInfoHash = HashCombine(viewInfoHash, Hash<UnderlyingType<decltype(vkformat)>>{}(Tounderlying(vkformat)));

				const auto bufferViewFound{ buffer->m_ViewCache.find(viewInfoHash) };
				auto& bufferViewRef{ (bufferViewFound != buffer->m_ViewCache.end()) ? bufferViewFound->second : buffer->m_ViewCache[viewInfoHash] };//TODO :
				if (bufferViewFound == buffer->m_ViewCache.end()) {
					ASSERT(RHIFormat::UNKNOWN != format);

					auto bufferViewInfo{ vk::BufferViewCreateInfo{}
						.setBuffer(buffer->m_Buffer)
						.setOffset(range.Offset)
						.setRange(range.ByteSize)
						.setFormat(static_cast<vk::Format>(vkformat))
					};

					VULKAN_CHECK(this->m_Context.Device.createBufferView(&bufferViewInfo, this->m_Context.AllocationCallbacks, &bufferViewRef));
				}

				generateWriteDescriptorData(layoutBinding.binding, layoutBinding.descriptorType, nullptr, nullptr, &bufferViewRef);

				if (RHIResourceState::Unknown == buffer->m_StateExtension.PermanentState)
					ret->m_BindingsThatNeedTransitions.push_back(static_cast<Uint16>(bindingIndex));
				else {
					const auto RequiredState{ isUAV ? RHIResourceState::UnorderedAccess : RHIResourceState::ShaderResource };
					ASSERT(RequiredState == (buffer->m_StateExtension.PermanentState & RequiredState));//TODO :
				}
			}
								break;

			case StructuredBuffer_SRV:case StructuredBuffer_UAV:case RawBuffer_SRV:case RawBuffer_UAV:case ConstantBuffer:case VolatileConstantBuffer: {
				Buffer* buffer{ Get<RefCountPtr<Buffer>>(binding.ResourcePtr).Get() };

				if (binding.Type == RHIResourceType::StructuredBuffer_UAV || binding.Type == RHIResourceType::RawBuffer_UAV)
					ASSERT(buffer->m_Desc.CanHaveUAVs);
				if (binding.Type == RHIResourceType::StructuredBuffer_UAV || binding.Type == RHIResourceType::StructuredBuffer_SRV)
					ASSERT(buffer->m_Desc.StructStride != 0);
				if (binding.Type == RHIResourceType::RawBuffer_SRV || binding.Type == RHIResourceType::RawBuffer_UAV)
					ASSERT(buffer->m_Desc.CanHaveRawViews);

				const auto range{ binding.Range.Resolve(buffer->m_Desc) };

				descriptorBufferInfo[descriptorBufferInfoCount++] = vk::DescriptorBufferInfo{}
					.setBuffer(buffer->m_Buffer)
					.setOffset(range.Offset)
					.setRange(range.ByteSize);

				ASSERT(nullptr != buffer->m_Buffer);

				generateWriteDescriptorData(layoutBinding.binding, layoutBinding.descriptorType, nullptr, &descriptorBufferInfo[descriptorBufferInfoCount - 1], nullptr);

				if (RHIResourceType::VolatileConstantBuffer == binding.Type) {
					ASSERT(buffer->m_Desc.IsVolatile);
					ret->m_VolatileConstantBuffers[ret->m_VolatileConstantBuffersCount++] = buffer;
				}
				else {
					if (RHIResourceState::Unknown == buffer->m_StateExtension.PermanentState)
						ret->m_BindingsThatNeedTransitions.push_back(static_cast<Uint16>(bindingIndex));
					else {
						RHIResourceState requiredState;
						if (binding.Type == RHIResourceType::StructuredBuffer_UAV || binding.Type == RHIResourceType::RawBuffer_UAV)
							requiredState = RHIResourceState::UnorderedAccess;
						else if (binding.Type == RHIResourceType::ConstantBuffer)
							requiredState = RHIResourceState::ConstantBuffer;
						else
							requiredState = RHIResourceState::ShaderResource;

						ASSERT(requiredState == (buffer->m_StateExtension.PermanentState & requiredState));//TODO :
					}
				}
			}
									 break;

			case PushConstants:
				break;

			case Sampler: {
				Vulkan::Sampler* sampler{ Get<RefCountPtr<Vulkan::Sampler>>(binding.ResourcePtr).Get() };

				descriptorImageInfo[descriptorImageInfoCount++] = vk::DescriptorImageInfo{}
				.setSampler(sampler->m_Sampler);

				generateWriteDescriptorData(layoutBinding.binding, layoutBinding.descriptorType, &descriptorImageInfo[descriptorImageInfoCount - 1], nullptr, nullptr);
			}
						break;


			case Count:default:
				ASSERT(false);
				break;
			}
		}

		this->m_Context.Device.updateDescriptorSets(descriptorWriteInfoCount, descriptorWriteInfo.data(), 0, nullptr);

		return RefCountPtr<BindingSet>::Create(ret);
	}

	RefCountPtr<CommandList> Device::Imp_CreateCommandList(const RHICommandListParameters& params) {
		return RefCountPtr<CommandList>::Create(new CommandList{ this->m_Context, this, this->m_Queues[Tounderlying(params.QueueType)].get(), params });
	}

	Uint64 Device::Imp_ExecuteCommandLists(CommandList* const* ppCmd, Uint32 numCmd, RHICommandQueue queuetype) {
		auto& queue{ *this->m_Queues[Tounderlying(queuetype)] };

		Vector<vk::PipelineStageFlags> waitStageArray{ queue.m_WaitSemaphores.size() };
		Vector<vk::CommandBuffer> commandBuffers{ numCmd };

		for (Uint64 Index = 0; Index < queue.m_WaitSemaphores.size(); ++Index)
			waitStageArray[Index] = vk::PipelineStageFlagBits::eTopOfPipe;

		++queue.m_LastSubmittedID;

		for (Uint64 Index = 0; Index < numCmd; ++Index) {
			CommandList* commandList{ ppCmd[Index] };
			SharedPtr<TrackedCommandBuffer> commandBuffer{ commandList->Get_CurrentCmdBuf() };

			commandBuffers[Index] = commandBuffer->CmdBuf;
			g_CommandBuffersInFlight[Tounderlying(queue.m_QueueID)].push_back(commandBuffer);

			for (const auto& buffer : commandBuffer->ReferencedStagingBuffers) {
				buffer->m_LastUseQueue = queue.m_QueueID;
				buffer->m_LastUseCommandListID = queue.m_LastSubmittedID;
			}
		}

		queue.m_SignalSemaphores.push_back(queue.m_TrackingSemaphore);
		queue.m_SignalSemaphoreValues.push_back(queue.m_LastSubmittedID);

		auto timelineSemaphoreInfo{ vk::TimelineSemaphoreSubmitInfo{}
			.setSignalSemaphoreValueCount(static_cast<Uint32>(queue.m_SignalSemaphoreValues.size()))
			.setPSignalSemaphoreValues(queue.m_SignalSemaphoreValues.data())
		};

		if (!queue.m_WaitSemaphoreValues.empty()) {
			timelineSemaphoreInfo.setWaitSemaphoreValueCount(static_cast<Uint32>(queue.m_WaitSemaphoreValues.size()));
			timelineSemaphoreInfo.setPWaitSemaphoreValues(queue.m_WaitSemaphoreValues.data());
		}

		auto submitInfo{ vk::SubmitInfo{}
			.setPNext(&timelineSemaphoreInfo)
			.setCommandBufferCount(numCmd)
			.setPCommandBuffers(commandBuffers.data())
			.setWaitSemaphoreCount(static_cast<Uint32>(queue.m_WaitSemaphores.size()))
			.setPWaitSemaphores(queue.m_WaitSemaphores.data())
			.setPWaitDstStageMask(waitStageArray.data())
			.setSignalSemaphoreCount(static_cast<Uint32>(queue.m_SignalSemaphores.size()))
			.setPSignalSemaphores(queue.m_SignalSemaphores.data())
		};

		queue.m_Queue.submit(submitInfo);

		queue.m_WaitSemaphores.clear();
		queue.m_WaitSemaphoreValues.clear();
		queue.m_SignalSemaphores.clear();
		queue.m_SignalSemaphoreValues.clear();

		for (Uint64 Index = 0; Index < numCmd; ++Index)
			ppCmd[Index]->Executed(queue.m_LastSubmittedID);

		return queue.m_LastSubmittedID;
	}

	bool Device::Imp_WaitForIdle(void) {
		this->m_Context.Device.waitIdle();

		return true;
	}

	void Device::Imp_RunGarbageCollection(void) {
		for (auto& queue : this->m_Queues)
			if (nullptr != queue) {
				Uint64 lastFinishedID{ queue->UpdateLastFinishedID() };

				List<SharedPtr<TrackedCommandBuffer>> submissions{ ::MoveTemp(g_CommandBuffersInFlight[Tounderlying(queue->m_QueueID)])};
				for (const auto& cmd : submissions) {
					if (cmd->SubmissionID <= lastFinishedID) {
						cmd->ReferencedResources.clear();
						cmd->ReferencedStagingBuffers.clear();
						cmd->SubmissionID = 0;
						g_CommandBuffersPool[Tounderlying(queue->m_QueueID)].push_back(cmd);
					}
					else
						g_CommandBuffersInFlight[Tounderlying(queue->m_QueueID)].push_back(cmd);
				}
			}
	}

	bool Device::Imp_QueryFeatureSupport(RHIFeature feature, void* outData, Uint64 outDataSize) {
		switch (feature) {
			using enum RHIFeature;
		case Meshlets:return this->m_Context.Extensions.NV_mesh_shader;
		case ConservativeRasterization:return this->m_Context.Extensions.EXT_conservative_rasterization;
		case ComputeQueue:return nullptr != this->m_Queues[Tounderlying(RHICommandQueue::Compute)];
		case CopyQueue:return nullptr != this->m_Queues[Tounderlying(RHICommandQueue::Copy)];
		default:return false;
		}
	}

	RHIFormatSupport Device::Imp_QueryFormatSupport(RHIFormat format) {
		VkFormat vulkanFormat{ ConvertFormat(format) };

		vk::FormatProperties props;
		this->m_Context.PhysicalDevice.getFormatProperties(static_cast<vk::Format>(vulkanFormat), &props);

		RHIFormatSupport result{ RHIFormatSupport::None };

		if (props.bufferFeatures)
			result |= RHIFormatSupport::Buffer;

		if (format == RHIFormat::R32_UINT || format == RHIFormat::R16_UINT)
			result |= RHIFormatSupport::IndexBuffer;

		if (props.bufferFeatures & vk::FormatFeatureFlagBits::eVertexBuffer)
			result |= RHIFormatSupport::VertexBuffer;

		if (props.optimalTilingFeatures)
			result |= RHIFormatSupport::Texture;

		if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
			result |= RHIFormatSupport::DepthStencil;

		if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eColorAttachment)
			result |= RHIFormatSupport::RenderTarget;

		if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eColorAttachmentBlend)
			result |= RHIFormatSupport::Blendable;

		if ((props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImage) ||
			(props.bufferFeatures & vk::FormatFeatureFlagBits::eUniformTexelBuffer))
			result |= RHIFormatSupport::ShaderLoad;

		if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear)
			result |= RHIFormatSupport::ShaderSample;

		if ((props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eStorageImage) ||
			(props.bufferFeatures & vk::FormatFeatureFlagBits::eStorageTexelBuffer))
			result |= RHIFormatSupport::ShaderUavLoad | RHIFormatSupport::ShaderUavStore;

		if ((props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eStorageImageAtomic) ||
			(props.bufferFeatures & vk::FormatFeatureFlagBits::eStorageTexelBufferAtomic))
			result |= RHIFormatSupport::ShaderAtomic;

		return result;
	}
}