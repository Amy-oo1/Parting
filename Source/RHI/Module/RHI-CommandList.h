#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"


PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"

PARTING_SUBMODULE(RHI, Texture)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Container;
PARTING_IMPORT Algorithm;
PARTING_IMPORT String;
PARTING_IMPORT Color;
PARTING_IMPORT Logger;

PARTING_SUBMODE_IMPORT(Resource)
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
PARTING_SUBMODE_IMPORT(Draw)


#else
#pragma once

#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"
//Global

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Container/Module/Container.h"
#include "Core/String/Module/String.h"
#include "Core/Color/Module/Color.h"
#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI-Resource.h"
#include "RHI/Module/RHI-Traits.h"
#include "RHI/Module/RHI-Common.h"
#include "RHI/Module/RHI-Format.h"
#include "RHI/Module/RHI-Heap.h"
#include "RHI/Module/RHI-Buffer.h"
#include "RHI/Module/RHI-Texture.h"
#include "RHI/Module/RHI-Sampler.h"
#include "RHI/Module/RHI-InputLayout.h"
#include "RHI/Module/RHI-Shader.h"
#include "RHI/Module/RHI-BlendState.h"
#include "RHI/Module/RHI-RasterState.h"
#include "RHI/Module/RHI-DepthStencilState.h"
#include "RHI/Module/RHI-ViewportState.h"
#include "RHI/Module/RHI-FrameBuffer.h"
#include "RHI/Module/RHI-ShaderBinding.h"
#include "RHI/Module/RHI-Pipeline.h"
#include "RHI/Module/RHI-Draw.h"

#endif // PARTING_MODULE_BUILD

namespace RHI {

	PARTING_EXPORT enum class RHICommandQueue : Uint8 {
		Graphics = 0,
		Compute,
		Copy,

		Count
	};

	PARTING_EXPORT HEADER_INLINE constexpr Uint64 c_VersionSubmittedFlag{ 0x8000000000000000 };
	PARTING_EXPORT HEADER_INLINE constexpr Uint32 c_VersionQueueShift{ 60 };
	PARTING_EXPORT HEADER_INLINE constexpr Uint32 c_VersionQueueMask{ 0x7 };
	PARTING_EXPORT HEADER_INLINE constexpr Uint64 c_VersionIDMask{ 0x0FFFFFFFFFFFFFFF };

	PARTING_EXPORT STDNODISCARD constexpr Uint64 MakeVersion(Uint64 id, RHICommandQueue queue, bool submitted) {
		Uint64 result{ (id & c_VersionIDMask) | (static_cast<Uint64>(queue) << c_VersionQueueShift) };
		if (submitted)
			result |= c_VersionSubmittedFlag;

		return result;
	}

	PARTING_EXPORT STDNODISCARD constexpr Uint64 VersionGetInstance(Uint64 version) { return version & c_VersionIDMask; }

	PARTING_EXPORT STDNODISCARD constexpr RHICommandQueue VersionGetQueue(Uint64 version) { return static_cast<RHICommandQueue>((version >> c_VersionQueueShift) & c_VersionQueueMask); }

	PARTING_EXPORT STDNODISCARD constexpr bool VersionGetSubmitted(Uint64 version) { return (version & c_VersionSubmittedFlag) != 0; }

	PARTING_EXPORT struct RHICommandListParameters final {
		// A command list with enableImmediateExecution = true maps to the immediate context on DX11.
		// Two immediate command lists cannot be open at the same time, which is checked by the validation layer.
		bool EnableImmediateExecution{ true };

		// Minimum size of memory chunks created to upload data to the device on DX12.
		Uint64 UploadChunkSize{ 64 * 1024 };

		// Minimum size of memory chunks created for AS build scratch buffers.
		Uint64 ScratchChunkSize{ 64 * 1024 };

		// Maximum total memory size used for all AS build scratch buffers owned by this command list.
		Uint64 ScratchMaxMemory{ 1024 * 1024 * 1024 };

		// Type of the queue that this command list is to be executed on.
		// COPY and COMPUTE queues have limited subsets of methods available.
		RHICommandQueue QueueType{ RHICommandQueue::Graphics };

		STDNODISCARD constexpr bool operator==(const RHICommandListParameters&)const noexcept = default;
		STDNODISCARD constexpr bool operator!=(const RHICommandListParameters&)const noexcept = default;
	};

	// Represents a sequence of GPU operations.
	// - DX11: All command list objects map to the single immediate context. Only one command list may be in the open
	//	 state at any given time, and all command lists must have CommandListParameters::enableImmediateExecution = true.
	// - DX12: One command list object may contain multiple instances of ID3D12GraphicsCommandList* and
	//	 ID3D12CommandAllocator objects, reusing older ones as they finish executing on the GPU. A command list object
	//	 also contains the upload manager (for suballocating memory from the upload heap on operations such as
	//	 writeBuffer) and the DXR scratch manager (for suballocating memory for acceleration structure builds).
	//	 The upload and scratch managers' memory is reused when possible, but it is only freed when the command list
	//	 object is destroyed. Thus, it might be a good idea to use a dedicated NVRHI command list for uploading large
	//	 amounts of data and to destroy it when uploading is finished.
	// - Vulkan: The command list objects don't own the VkCommandBuffer-s but request available ones from the queue
	//	 instead. The upload and scratch buffers behave the same way they do on DX12.


	PARTING_EXPORT template<typename Derived, APITagConcept APITag>
		class RHICommandList :public RHIResource<Derived> {
		friend class RHIResource<Derived>;

		protected:
			using Imp_Texture = typename RHITypeTraits<APITag>::Imp_Texture;
			using Imp_StagingTexture = typename RHITypeTraits<APITag>::Imp_StagingTexture;
			using Imp_Buffer = typename RHITypeTraits<APITag>::Imp_Buffer;
			using Imp_Sampler = typename RHITypeTraits<APITag>::Imp_Sampler;
			using Imp_SamplerFeedbackTexture = typename RHITypeTraits<APITag>::Imp_SamplerFeedbackTexture;
			using Imp_FrameBuffer = typename RHITypeTraits<APITag>::Imp_FrameBuffer;
			using Imp_TimerQuery = typename RHITypeTraits<APITag>::Imp_TimerQuery;
			using Imp_EventQuery = typename RHITypeTraits<APITag>::Imp_EventQuery;
			using Imp_Heap = typename RHITypeTraits<APITag>::Imp_Heap;
			using Imp_BindingSet = typename RHITypeTraits<APITag>::Imp_BindingSet;
			using Imp_GraphicsPipeline = typename RHITypeTraits<APITag>::Imp_GraphicsPipeline;
			using Imp_ComputePipeline = typename RHITypeTraits<APITag>::Imp_ComputePipeline;
			using Imp_MeshletPipeline = typename RHITypeTraits<APITag>::Imp_MeshletPipeline;
			using Imp_Device = typename RHITypeTraits<APITag>::Imp_Device;
			//TODO :using Imp_DescriptorSet = typename RHITypeTraits<APITag>::Imp_DescriptorSet;
		protected:
			RHICommandList(void) = default;
			PARTING_VIRTUAL ~RHICommandList(void) = default;

		public:
			// Prepares the command list for recording a new sequence of commands.
			// All other methods of ICommandList must only be used when the command list is open.
			// - DX11: The immediate command list may always stay in the open state, although that prohibits other
			//	 command lists from opening.
			// - DX12, Vulkan: Creates or reuses the command list or buffer object and the command allocator (DX12),
			//	 starts tracking the resources being referenced in the command list.
		public:
			void Open(void) { this->Get_Derived()->Imp_Open(); }

			void Close(void) { this->Get_Derived()->Imp_Close(); }

			void ClearState(void) { this->Get_Derived()->Imp_ClearState(); }

			void ClearTextureFloat(Imp_Texture* texture, RHITextureSubresourceSet subresources, const Color& color) { this->Get_Derived()->Imp_ClearTextureFloat(texture, subresources, color); }

			void ClearDepthStencilTexture(Imp_Texture* texture, RHITextureSubresourceSet subresources, Optional<float> depth, Optional<Uint8> stencil) { this->Get_Derived()->Imp_ClearDepthStencilTexture(texture, subresources, depth, stencil); }

			void ClearTextureUInt(Imp_Texture* texture, RHITextureSubresourceSet subresources, Uint32 clearColor) { this->Get_Derived()->Imp_ClearTextureUInt(texture, subresources, clearColor); }

			void CopyTexture(Imp_Texture* des, RHITextureSlice desSlice, Imp_Texture* src, RHITextureSlice srcSlice) { this->Get_Derived()->Imp_CopyTexture(des, desSlice, src, srcSlice); }

			void CopyTexture(Imp_Texture* des, RHITextureSlice desSlice, Imp_StagingTexture* src, RHITextureSlice srcSlice) { this->Get_Derived()->Imp_CopyTexture(des, desSlice, src, srcSlice); }

			void CopyTexture(Imp_StagingTexture* des, RHITextureSlice desSlice, Imp_Texture* src, RHITextureSlice srcSlice) { this->Get_Derived()->Imp_CopyTexture(des, desSlice, src, srcSlice); }

			void WriteTexture(Imp_Texture* texture, Uint32 ArraySlice, Uint32 MipLevel, const void* data, Uint64 RowPitch, Uint64 DepthPitch = 0) { this->Get_Derived()->Imp_WriteTexture(texture, ArraySlice, MipLevel, data, RowPitch, DepthPitch); }

			void ResolveTexture(Imp_Texture* dest, const RHITextureSubresourceSet& dstSubresources, Imp_Texture* src, const RHITextureSubresourceSet& srcSubresources) { this->Get_Derived()->Imp_ResolveTexture(dest, dstSubresources, src, srcSubresources); }

			void WriteBuffer(Imp_Buffer* buffer, const void* data, Uint64 dataSize, Uint64 destOffsetBytes = 0) { this->Get_Derived()->Imp_WriteBuffer(buffer, data, dataSize, destOffsetBytes); }

			void ClearBufferUInt(Imp_Buffer* buffer, Uint32 clearvalue) { this->Get_Derived()->Imp_ClearBufferUInt(buffer, clearvalue); }

			void CopyBuffer(Imp_Buffer* des, Uint64 desOffset, Imp_Buffer* src, Uint64 srcOffset, Uint64 dataSizeBytes) { this->Get_Derived()->Imp_CopyBuffer(des, desOffset, src, srcOffset, dataSizeBytes); }

			void ClearSamplerFeedbackTexture(Imp_SamplerFeedbackTexture* texture) { this->Get_Derived()->Imp_ClearSamplerFeedbackTexture(texture); }

			void DecodeSamplerFeedbackTexture(Imp_Buffer* buffer, Imp_SamplerFeedbackTexture* texture, RHIFormat format) { this->Get_Derived()->Imp_DecodeSamplerFeedbackTexture(buffer, texture, format); }

			void SetSamplerFeedbackTextureState(Imp_SamplerFeedbackTexture* texture, RHIResourceState state) { this->Get_Derived()->Imp_SetSamplerFeedbackTextureState(texture, state); }

			void SetPushConstants(const void* data, Uint32 ByteSize) { this->Get_Derived()->Imp_SetPushConstants(data, ByteSize); }

			void SetGraphicsState(const RHIGraphicsState<APITag>& pipeline) { this->Get_Derived()->Imp_SetGraphicsState(pipeline); }

			void Draw(const RHIDrawArguments& args) { this->Get_Derived()->Imp_Draw(args); }

			void DrawIndexed(const RHIDrawArguments& args) { this->Get_Derived()->Imp_Draw(args); }

			void DrawIndirect(Uint32 OffsetBytes, Uint32 Count = 1) { this->Get_Derived()->Imp_DrawIndirect(OffsetBytes, Count); }

			void DrawIndexedIndirect(Uint32 OffsetBytes, Uint32 Count = 1) { this->Get_Derived()->Imp_DrawIndirect(OffsetBytes, Count); }

			void SetComputeState(const RHIComputeState<APITag>& pipeline) { this->Get_Derived()->Imp_SetComputeState(pipeline); }

			void Dispatch(Uint32 GroupsX, Uint32 GroupsY = 1, Uint32 GroupsZ = 1) { this->Get_Derived()->Imp_Dispatch(GroupsX, GroupsY, GroupsZ); }

			void DispatchIndirect(Uint32 OffsetBytes) { this->Get_Derived()->Imp_DispatchIndirect(OffsetBytes); }

			void SetMeshletState(const RHIMeshletState<APITag>& pipeline) { this->Get_Derived()->Imp_SetMeshletState(pipeline); }

			void DispatchMesh(Uint32 GroupsX, Uint32 GroupsY = 1, Uint32 GroupsZ = 1) { this->Get_Derived()->Imp_Dispatch(GroupsX, GroupsY, GroupsZ); }

			void BeginTimerQuery(Imp_TimerQuery* query) { this->Get_Derived()->Imp_BeginTimerQuery(query); }

			void EndTimerQuery(Imp_TimerQuery* query) { this->Get_Derived()->Imp_EndTimerQuery(query); }

			void BeginMarker(const char* Name) { this->Get_Derived()->Imp_BeginMarker(Name); }

			void EndMarker(void) { this->Get_Derived()->Imp_EndMarker(); }

			void SetEnableAutomaticBarriers(bool enable) { this->Get_Derived()->Imp_SetEnableAutomaticBarriers(enable); }

			void SetResourceStatesForBindingSet(Imp_BindingSet* bindingSet) { this->Get_Derived()->Imp_SetResourceStatesForBindingSet(bindingSet); }

			void SetResourceStatesForFramebuffer(Imp_FrameBuffer* framebuffer) { this->Get_Derived()->Imp_SetResourceStatesForFramebuffer(framebuffer); }

			void SetEnableUAVBarriersForTexture(Imp_Texture* texture, bool enable) { this->Get_Derived()->Imp_SetEnableUAVBarriersForTexture(texture, enable); }

			void SetEnableUAVBarriersForBuffer(Imp_Buffer* buffer, bool enable) { this->Get_Derived()->Imp_SetEnableUAVBarriersForBuffer(buffer, enable); }

			void BeginTrackingTextureState(Imp_Texture* texture, RHITextureSubresourceSet subresources, RHIResourceState state) { this->Get_Derived()->Imp_BeginTrackingTextureState(texture, subresources, state); }

			void BeginTrackingBufferState(Imp_Buffer* buffer, RHIResourceState state) { this->Get_Derived()->Imp_BeginTrackingBufferState(buffer, state); }

			void SetTextureState(Imp_Texture* texture, RHITextureSubresourceSet subresources, RHIResourceState state) { this->Get_Derived()->Imp_SetTextureState(texture, subresources, state); }

			void SetBufferState(Imp_Buffer* buffer, RHIResourceState state) { this->Get_Derived()->Imp_SetBufferState(buffer, state); }

			void SetPermanentTextureState(Imp_Texture* texture, RHIResourceState state) { this->Get_Derived()->Imp_SetPermanentTextureState(texture, state); }

			void SetPermanentBufferState(Imp_Buffer* buffer, RHIResourceState state) { this->Get_Derived()->Imp_SetPermanentBufferState(buffer, state); }

			void CommitBarriers(void) { this->Get_Derived()->Imp_CommitBarriers(); }

			STDNODISCARD RHIResourceState Get_TextureSubresourceState(Imp_Texture* texture, Uint32 arraySlice, Uint32 mipLevel) { return this->Get_Derived()->Imp_Get_TextureSubresourceState(texture, arraySlice, mipLevel); }

			STDNODISCARD RHIResourceState Get_BufferState(Imp_Buffer* buffer) { return this->Get_Derived()->Imp_Get_BufferState(buffer); }

			STDNODISCARD Imp_Device* Get_Device(void) { return this->Get_Derived()->Imp_Get_Device(); }

			STDNODISCARD const RHICommandListParameters& Get_Desc(void) { return this->Get_Derived()->Imp_Get_Desc(); }

		private:
			STDNODISCARD Derived* Get_Derived(void)noexcept { return static_cast<Derived*>(this); }
			STDNODISCARD const Derived* Get_Derived(void)const noexcept { return static_cast<const Derived*>(this); }
		private:
			void Imp_Open(void) { LOG_ERROR("No Imp"); }
			void Imp_Close(void) { LOG_ERROR("No Imp"); }
			void Imp_ClearState(void) { LOG_ERROR("No Imp"); }
			void Imp_ClearTextureFloat(Imp_Texture*, RHITextureSubresourceSet, const Color&) { LOG_ERROR("No Imp"); }
			void Imp_ClearDepthStencilTexture(Imp_Texture*, RHITextureSubresourceSet, Optional<float>, Optional<Uint8>) { LOG_ERROR("No Imp"); }
			void Imp_ClearTextureUInt(Imp_Texture*, RHITextureSubresourceSet, Uint32) { LOG_ERROR("No Imp"); }
			void Imp_CopyTexture(Imp_Texture*, RHITextureSlice, Imp_Texture*, RHITextureSlice) { LOG_ERROR("No Imp"); }
			void Imp_CopyTexture(Imp_Texture*, RHITextureSlice, Imp_StagingTexture*, RHITextureSlice) { LOG_ERROR("No Imp"); }
			void Imp_CopyTexture(Imp_StagingTexture*, RHITextureSlice, Imp_Texture*, RHITextureSlice) { LOG_ERROR("No Imp"); }
			void Imp_WriteTexture(Imp_Texture*, Uint32, Uint32, const void*, Uint64, Uint64) { LOG_ERROR("No Imp"); }
			void Imp_ResolveTexture(Imp_Texture*, const RHITextureSubresourceSet&, Imp_Texture*, const RHITextureSubresourceSet&) { LOG_ERROR("No Imp"); }
			void Imp_WriteBuffer(Imp_Buffer*, const void*, Uint64, Uint64) { LOG_ERROR("No Imp"); }
			void Imp_ClearBufferUInt(Imp_Buffer*, Uint32) { LOG_ERROR("No Imp"); }
			void Imp_CopyBuffer(Imp_Buffer*, Uint64, Imp_Buffer*, Uint64, Uint64) { LOG_ERROR("No Imp"); }
			void Imp_ClearSamplerFeedbackTexture(Imp_SamplerFeedbackTexture*) { LOG_ERROR("No Imp"); }
			void Imp_DecodeSamplerFeedbackTexture(Imp_Buffer*, Imp_SamplerFeedbackTexture*, RHIFormat) { LOG_ERROR("No Imp"); }
			void Imp_SetSamplerFeedbackTextureState(Imp_SamplerFeedbackTexture*, RHIResourceState) { LOG_ERROR("No Imp"); }
			void Imp_SetPushConstants(const void*, Uint32) { LOG_ERROR("No Imp"); }
			void Imp_SetGraphicsState(const RHIGraphicsState<APITag>&) { LOG_ERROR("No Imp"); }
			void Imp_Draw(const RHIDrawArguments&) { LOG_ERROR("No Imp"); }
			void Imp_DrawIndexed(const RHIDrawArguments&) { LOG_ERROR("No Imp"); }
			void Imp_DrawIndirect(Uint32, Uint32) { LOG_ERROR("No Imp"); }
			void Imp_DrawIndexedIndirect(Uint32, Uint32) { LOG_ERROR("No Imp"); }
			void Imp_SetComputeState(const RHIComputeState<APITag>&) { LOG_ERROR("No Imp"); }
			void Imp_Dispatch(Uint32, Uint32, Uint32) { LOG_ERROR("No Imp"); }
			void Imp_DispatchIndirect(Uint32) { LOG_ERROR("No Imp"); }
			void Imp_SetMeshletState(const RHIMeshletState<APITag>&) { LOG_ERROR("No Imp"); }
			void Imp_DispatchMesh(Uint32, Uint32, Uint32) { LOG_ERROR("No Imp"); }
			void Imp_BeginTimerQuery(Imp_TimerQuery*) { LOG_ERROR("No Imp"); }
			void Imp_EndTimerQuery(Imp_TimerQuery*) { LOG_ERROR("No Imp"); }
			void Imp_BeginMarker(const char*) { LOG_ERROR("No Imp"); }
			void Imp_EndMarker(void) { LOG_ERROR("No Imp"); }
			void Imp_SetEnableAutomaticBarriers(bool) { LOG_ERROR("No Imp"); }
			void Imp_SetResourceStatesForBindingSet(Imp_BindingSet*) { LOG_ERROR("No Imp"); }
			void Imp_SetResourceStatesForFramebuffer(Imp_FrameBuffer*) { LOG_ERROR("No Imp"); }
			void Imp_SetEnableUAVBarriersForTexture(Imp_Texture*, bool) { LOG_ERROR("No Imp"); }
			void Imp_SetEnableUAVBarriersForBuffer(Imp_Buffer*, bool) { LOG_ERROR("No Imp"); }
			void Imp_BeginTrackingTextureState(Imp_Texture*, RHITextureSubresourceSet, RHIResourceState) { LOG_ERROR("No Imp"); }
			void Imp_BeginTrackingBufferState(Imp_Buffer*, RHIResourceState) { LOG_ERROR("No Imp"); }
			void Imp_SetTextureState(Imp_Texture*, RHITextureSubresourceSet, RHIResourceState) { LOG_ERROR("No Imp"); }
			void Imp_SetBufferState(Imp_Buffer*, RHIResourceState) { LOG_ERROR("No Imp"); }
			void Imp_SetPermanentTextureState(Imp_Texture*, RHIResourceState) { LOG_ERROR("No Imp"); }
			void Imp_SetPermanentBufferState(Imp_Buffer*, RHIResourceState) { LOG_ERROR("No Imp"); }
			void Imp_CommitBarriers(void) { LOG_ERROR("No Imp"); }
			RHIResourceState Imp_Get_TextureSubresourceState(Imp_Texture*, Uint32, Uint32) { LOG_ERROR("No Imp"); return RHIResourceState::Unknown; }
			RHIResourceState Imp_Get_BufferState(Imp_Buffer*) { LOG_ERROR("No Imp"); return RHIResourceState::Unknown; }
			Imp_Device* Imp_Get_Device(void) { LOG_ERROR("No Imp"); return nullptr; }
			const RHICommandListParameters& Imp_Get_Desc(void) { LOG_ERROR("No Imp"); return RHICommandListParameters{}; }
	};

}