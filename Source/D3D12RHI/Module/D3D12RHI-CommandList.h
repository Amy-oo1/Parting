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
#include "D3D12RHI/Module/DirectX12Wrapper.h"

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Algorithm/Module/Algorithm.h"
#include "Core/Container/Module/Container.h"
#include "Core/String/Module/String.h"
#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI.h"
#include "RHI/Module/StateTracking.h"

#include "D3D12RHI/Module/D3D12RHI-Traits.h"
#include "D3D12RHI/Module/D3D12RHI-Common.h"
#include "D3D12RHI/Module/D3D12RHI-Format.h"
#include "D3D12RHI/Module/D3D12RHI-Heap.h"
#include "D3D12RHI/Module/D3D12RHI-Buffer.h"
#include "D3D12RHI/Module/D3D12RHI-Texture.h"
#include "D3D12RHI/Module/D3D12RHI-Sampler.h"
#include "D3D12RHI/Module/D3D12RHI-InputLayout.h"
#include "D3D12RHI/Module/D3D12RHI-Shader.h"
#include "D3D12RHI/Module/D3D12RHI-BlendState.h"
#include "D3D12RHI/Module/D3D12RHI-RasterState.h"
#include "D3D12RHI/Module/D3D12RHI-DepthStencilState.h"
#include "D3D12RHI/Module/D3D12RHI-ViewportState.h"
#include "D3D12RHI/Module/D3D12RHI-FrameBuffer.h"
#include "D3D12RHI/Module/D3D12RHI-ShaderBinding.h"
#include "D3D12RHI/Module/D3D12RHI-Pipeline.h"

#endif // PARTING_MODULE_BUILD

namespace RHI::D3D12 {


	PARTING_EXPORT class CommandList final :public RHICommandList<CommandList, D3D12Tag> {
		friend class RHIResource<CommandList>;
		friend class RHICommandList<CommandList, D3D12Tag>;

		struct VolatileConstantBufferBinding final {
			Uint32 BindingPoint; // RootParameterIndex
			Buffer* Buffer;
			D3D12_GPU_VIRTUAL_ADDRESS Address;
		};

	public:
		//TODO ;this queue(1) can from device(0), but typeless device in here unless def  device than def this com func
		CommandList(Device* device, D3D12Queue* queue, const Context& context, D3D12DeviceResources& resources, const RHICommandListParameters& params) :
			RHICommandList<CommandList, D3D12Tag>{},
			m_Context{ context },
			m_DeviceResourcesRef{ resources },
			m_Device{ device },
			m_Queue{ queue },
			m_Desc{ params },
			m_UploadManager{ context, queue, params.UploadChunkSize, 0, false },
			m_DxrScratchManager{ context, queue, params.ScratchChunkSize, params.ScratchMaxMemory, true }
		{}

		~CommandList(void) = default;

	public:
		bool AllocateUploadBuffer(Uint64 size, void** pCpuAddress, D3D12_GPU_VIRTUAL_ADDRESS* pGpuAddress) {
			return this->m_UploadManager.SuballocateBuffer(size, nullptr, nullptr, nullptr, pCpuAddress, pGpuAddress,
				this->m_RecordingVersion, g_D3D12ConstantBufferDataPlacementAlignment);
		}

		bool AllocateDxrScratchBuffer(Uint64 size, void** pCpuAddress, D3D12_GPU_VIRTUAL_ADDRESS* pGpuAddress) {
			return m_DxrScratchManager.SuballocateBuffer(size, m_ActiveCommandList->CommandList, nullptr, nullptr, pCpuAddress, pGpuAddress,
				this->m_RecordingVersion, g_D3D12ConstantBufferDataPlacementAlignment);
		}

		D3D12_GPU_VIRTUAL_ADDRESS Get_BufferGpuVA(Buffer* buffer) {
			if (!buffer)
				return 0;

			if (buffer->m_Desc.IsVolatile)
				return this->m_VolatileConstantBufferAddresses[buffer];

			return buffer->m_GPUVirtualAddress;
		}

		STDNODISCARD ID3D12CommandList* Get_D3D12CommandList(void) const { return this->m_ActiveCommandList->CommandList; }

		SharedPtr<CommandListInstance> Executed(D3D12Queue* pQueue);


	private:
		SharedPtr<InternalCommandList> CreateInternalCommandList(void)const;

		void ClearStateCache(void);

		bool CommitDescriptorHeaps(void);

		void BindGraphicsPipeline(GraphicsPipeline* pso, bool updateRootSignature) const;

		void BindMeshletPipeline(MeshletPipeline* pso, bool updateRootSignature) const;

		void BindFramebuffer(FrameBuffer* fb);

		void SetGraphicsBindings(const Array<BindingSet*, g_MaxBindingLayoutCount> bindings, Uint32 BindingSetCount, Uint32 bindingUpdateMask, Buffer* indirectParams, bool updateIndirectParams, const D3D12RootSignature* rootSignature);

		void SetComputeBindings(const Array<BindingSet*, g_MaxBindingLayoutCount> bindings, Uint32 BindingSetCount, Uint32 bindingUpdateMask, Buffer* indirectParams, bool updateIndirectParams, const D3D12RootSignature* rootSignature);

		void UnbindShadingRateState(void);

		void UpdateGraphicsVolatileBuffers(void);

		void UpdateComputeVolatileBuffers(void);

	private:
		const Context& m_Context;
		D3D12DeviceResources& m_DeviceResourcesRef;
		Device* m_Device{ nullptr };
		D3D12Queue* m_Queue{ nullptr };

		RHICommandListParameters m_Desc;

		UploadManager m_UploadManager;
		UploadManager m_DxrScratchManager;
		RHICommandListResourceStateTracker<D3D12Tag> m_StateTracker;
		bool m_EnableAutomaticBarriers{ true };

		SharedPtr<InternalCommandList> m_ActiveCommandList;
		List<SharedPtr<InternalCommandList>> m_CommandListPool;
		SharedPtr<CommandListInstance> m_Instance;
		Uint64 m_RecordingVersion{ 0 };

		RHIGraphicsState<D3D12Tag> m_CurrentGraphicsState;
		RHIComputeState<D3D12Tag> m_CurrentComputeState;
		RHIMeshletState<D3D12Tag> m_CurrentMeshletState;
		bool m_CurrentGraphicsStateValid{ false };
		bool m_CurrentComputeStateValid{ false };
		bool m_CurrentMeshletStateValid{ false };

		ID3D12DescriptorHeap* m_CurrentHeapSRVetc{ nullptr };
		ID3D12DescriptorHeap* m_CurrentHeapSamplers{ nullptr };
		ID3D12Resource* m_CurrentUploadBuffer{ nullptr };
		RHISinglePassStereoState m_CurrentSinglePassStereoState;

		UnorderedMap<Buffer*, D3D12_GPU_VIRTUAL_ADDRESS> m_VolatileConstantBufferAddresses;
		bool m_AnyVolatileBufferWrites{ false };

		Vector<D3D12_RESOURCE_BARRIER> m_D3DBarriers;

		Array<VolatileConstantBufferBinding, g_MaxVolatileConstantBuffers> m_CurrentGraphicsVolatileCBs{};
		RemoveCV<decltype(g_MaxVolatileConstantBuffers)>::type m_CurrentGraphicsVolatileCBCount{ 0 };

		Array<VolatileConstantBufferBinding, g_MaxVolatileConstantBuffers> m_CurrentComputeVolatileCBs{};
		RemoveCV<decltype(g_MaxVolatileConstantBuffers)>::type m_CurrentComputeVolatileCBCount{ 0 };


	private:
		RHIObject Imp_GetNativeObject(RHIObjectType)const noexcept { LOG_ERROR("Imp But Empty");  return RHIObject{}; }

		void Imp_Open(void);
		void Imp_Close(void);
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
		void Imp_ClearSamplerFeedbackTexture(SamplerFeedbackTexture* texture);
		void Imp_DecodeSamplerFeedbackTexture(Buffer* buffer, SamplerFeedbackTexture* texture, RHIFormat format);
		void Imp_SetSamplerFeedbackTextureState(SamplerFeedbackTexture* texture, RHIResourceState state);
		void Imp_SetPushConstants(const void* data, Uint32 ByteSize);
		void Imp_SetGraphicsState(const RHIGraphicsState<D3D12Tag>& pipeline);
		void Imp_Draw(const RHIDrawArguments& args);
		void Imp_DrawIndexed(const RHIDrawArguments& args);
		void Imp_DrawIndirect(Uint32 OffsetBytes, Uint32 Count);
		void Imp_DrawIndexedIndirect(Uint32 OffsetBytes, Uint32 Count);
		void Imp_SetComputeState(const RHIComputeState<D3D12Tag>& pipeline);
		void Imp_Dispatch(Uint32 GroupsX, Uint32 GroupsY, Uint32 GroupsZ);
		void Imp_DispatchIndirect(Uint32 OffsetBytes);
		void Imp_SetMeshletState(const RHIMeshletState<D3D12Tag>& pipeline);
		void Imp_DispatchMesh(Uint32 GroupsX, Uint32 GroupsY, Uint32 GroupsZ);
		void Imp_BeginTimerQuery(TimerQuery* query);
		void Imp_EndTimerQuery(TimerQuery* query);
		void Imp_BeginMarker(const char* Name);
		void Imp_EndMarker(void);
		void Imp_SetEnableAutomaticBarriers(bool enable);
		void Imp_SetResourceStatesForBindingSet(BindingSet* bindingSet);
		void Imp_SetResourceStatesForFramebuffer(FrameBuffer* framebuffer);
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


	//Src
	SharedPtr<CommandListInstance> CommandList::Executed(D3D12Queue* pQueue) {
		SharedPtr<CommandListInstance> instance{ this->m_Instance };
		instance->Fence = pQueue->m_Fence;
		instance->SubmittedInstance = pQueue->m_LastSubmittedInstance;
		this->m_Instance.reset();

		this->m_ActiveCommandList->LastSubmittedInstance = pQueue->m_LastSubmittedInstance;
		this->m_CommandListPool.push_back(this->m_ActiveCommandList);
		this->m_ActiveCommandList.reset();

		//NOTE: here can use const auto& is in top not in botton ,
		for (auto const& it : instance->ReferencedStagingTextures) {
			it->m_LastUseFence = pQueue->m_Fence;
			it->m_LastUseFenceValue = instance->SubmittedInstance;
		}

		for (auto& it : instance->ReferencedStagingBuffers) {
			it->m_LastUseFence = pQueue->m_Fence;
			it->m_LastUseFenceValue = instance->SubmittedInstance;
		}

		for (auto& it : instance->ReferencedTimerQueries) {
			it->m_Started = true;
			it->m_Resolved = false;
			it->m_Fence = pQueue->m_Fence;
			it->m_FenceCounter = instance->SubmittedInstance;
		}

		this->m_StateTracker.CommandListSubmitted();

		Uint64 submittedVersion{ MakeVersion(instance->SubmittedInstance, m_Desc.QueueType, true) };
		this->m_UploadManager.SubmitChunks(this->m_RecordingVersion, submittedVersion);
		this->m_DxrScratchManager.SubmitChunks(this->m_RecordingVersion, submittedVersion);
		this->m_RecordingVersion = 0;

		return instance;
	}



	inline SharedPtr<InternalCommandList> CommandList::CreateInternalCommandList(void)const {
		auto commandList{ MakeShared<InternalCommandList>() };

		D3D12_COMMAND_LIST_TYPE d3dCommandListType{};
		switch (this->m_Desc.QueueType) {
			using enum RHICommandQueue;
		case Graphics:d3dCommandListType = D3D12_COMMAND_LIST_TYPE_DIRECT; break;
		case Compute:d3dCommandListType = D3D12_COMMAND_LIST_TYPE_COMPUTE; break;
		case Copy:d3dCommandListType = D3D12_COMMAND_LIST_TYPE_COPY; break;
		case Count:default:ASSERT(false); return nullptr;
		}

		this->m_Context.Device->CreateCommandAllocator(d3dCommandListType, PARTING_IID_PPV_ARGS(&commandList->Allocator));
		this->m_Context.Device->CreateCommandList(1, d3dCommandListType, commandList->Allocator, nullptr, PARTING_IID_PPV_ARGS(&commandList->CommandList));

		commandList->CommandList->QueryInterface(PARTING_IID_PPV_ARGS(&commandList->CommandList4));
		commandList->CommandList->QueryInterface(PARTING_IID_PPV_ARGS(&commandList->CommandList6));

		return commandList;
	}

	inline void CommandList::ClearStateCache(void) {
		this->m_AnyVolatileBufferWrites = false;
		this->m_CurrentGraphicsStateValid = false;
		this->m_CurrentComputeStateValid = false;
		this->m_CurrentMeshletStateValid = false;
		this->m_CurrentHeapSRVetc = nullptr;
		this->m_CurrentHeapSamplers = nullptr;
		this->m_CurrentGraphicsVolatileCBCount = 0;
		this->m_CurrentComputeVolatileCBCount = 0;
		this->m_CurrentSinglePassStereoState = RHISinglePassStereoState{};
	}

	inline bool CommandList::CommitDescriptorHeaps(void) {
		const auto heapSRVetc{ this->m_DeviceResourcesRef.ShaderResourceViewHeap.Get_ShaderVisibleHeap() };
		const auto heapSamplers{ this->m_DeviceResourcesRef.SamplerHeap.Get_ShaderVisibleHeap() };

		if (heapSRVetc != this->m_CurrentHeapSRVetc || heapSamplers != this->m_CurrentHeapSamplers) {
			ID3D12DescriptorHeap* heaps[2]{ heapSRVetc, heapSamplers };
			m_ActiveCommandList->CommandList->SetDescriptorHeaps(2, heaps);

			m_CurrentHeapSRVetc = heapSRVetc;
			m_CurrentHeapSamplers = heapSamplers;

			m_Instance->ReferencedNativeResources.push_back(heapSRVetc);
			m_Instance->ReferencedNativeResources.push_back(heapSamplers);

			return true;
		}

		return false;
	}

	inline void CommandList::BindGraphicsPipeline(GraphicsPipeline* pso, bool updateRootSignature) const {
		const auto& pipelineDesc = pso->m_Desc;

		if (updateRootSignature)
			this->m_ActiveCommandList->CommandList->SetGraphicsRootSignature(pso->m_RootSignature->m_RootSignature);

		this->m_ActiveCommandList->CommandList->SetPipelineState(pso->m_PipelineState);
		this->m_ActiveCommandList->CommandList->IASetPrimitiveTopology(ConvertPrimitiveType(pipelineDesc.PrimType, pipelineDesc.PatchControlPoints));
	}

	void CommandList::BindMeshletPipeline(MeshletPipeline* pso, bool updateRootSignature) const {
		const auto& state{ pso->m_Desc };

		ID3D12GraphicsCommandList* commandList{ this->m_ActiveCommandList->CommandList };

		if (updateRootSignature)
			commandList->SetGraphicsRootSignature(pso->m_RootSignature->m_RootSignature);

		commandList->SetPipelineState(pso->m_PipelineState);

		commandList->IASetPrimitiveTopology(ConvertPrimitiveType(state.PrimType, 0));

		if (pso->m_viewportState.ViewportCount > 0)
			commandList->RSSetViewports(pso->m_viewportState.ViewportCount, pso->m_viewportState.Viewports.data());

		if (pso->m_viewportState.ScissorCount > 0)
			commandList->RSSetScissorRects(pso->m_viewportState.ScissorCount, pso->m_viewportState.Scissors.data());
	}

	inline void CommandList::BindFramebuffer(FrameBuffer* fb) {
		if (this->m_EnableAutomaticBarriers)
			this->SetResourceStatesForFramebuffer(fb);

		Array<D3D12_CPU_DESCRIPTOR_HANDLE, g_MaxRenderTargetCount> RTVs{};
		for (Uint32 rtIndex = 0; rtIndex < fb->m_RTVCount; ++rtIndex)
			RTVs[rtIndex] = this->m_DeviceResourcesRef.RenderTargetViewHeap.Get_CPUHandle(fb->m_RTVs[rtIndex]);

		D3D12_CPU_DESCRIPTOR_HANDLE DSV{};
		if (fb->m_Desc.DepthStencilAttachment.Is_Valid())
			DSV = this->m_DeviceResourcesRef.DepthStencilViewHeap.Get_CPUHandle(fb->DSV);

		m_ActiveCommandList->CommandList->OMSetRenderTargets(fb->m_RTVCount, RTVs.data(), false, fb->m_Desc.DepthStencilAttachment.Is_Valid() ? &DSV : nullptr);
	}

	//TODO :Use Span
	void CommandList::SetComputeBindings(const Array<BindingSet*, g_MaxBindingLayoutCount> bindings, Uint32 BindingSetCount, Uint32 bindingUpdateMask, Buffer* indirectParams, bool updateIndirectParams, const D3D12RootSignature* rootSignature) {
		if (bindingUpdateMask) {
			Array<VolatileConstantBufferBinding, g_MaxVolatileConstantBuffers> newVolatileCBs{};
			RemoveCV<decltype(g_MaxVolatileConstantBuffers)>::type newVolatileCBCount{ 0 };

			for (Uint32 bindingSetIndex = 0; bindingSetIndex < BindingSetCount; ++bindingSetIndex) {
				BindingSet* bindingSet{ bindings[bindingSetIndex] };

				if (nullptr == bindingSet)
					continue;

				const bool updateThisSet{ (bindingUpdateMask & (1 << bindingSetIndex)) != 0 };

				auto& [Layout, rootParameterOffset] { rootSignature->m_BindLayouts[bindingSetIndex] };
				auto bindinglayout{ Get<RefCountPtr<BindingLayout>>(Layout).Get() };

				//TODO : BingdSet and DescriptorTable
				ASSERT(bindinglayout == bindingSet->Get_Layout()); // validation layer handles this

				// Bind the volatile constant buffers
				for (Uint32 volatileCbIndex = 0; volatileCbIndex < bindingSet->m_VolatileCBCount; ++volatileCbIndex) {
					const auto& [ParameterIndex, buffer] { bindingSet->m_RootParametersVolatileCB[volatileCbIndex] };
					auto rootParameterIndex{ rootParameterOffset + ParameterIndex };

					if (nullptr != buffer) {
						if (buffer->m_Desc.IsVolatile) {
							const D3D12_GPU_VIRTUAL_ADDRESS volatileData{ this->m_VolatileConstantBufferAddresses[buffer] };

							if (0 == volatileData) {
								LOG_ERROR("Volatile buffer address is null");

								continue;
							}

							if (updateThisSet || volatileData != m_CurrentComputeVolatileCBs[newVolatileCBCount].Address)
								this->m_ActiveCommandList->CommandList->SetComputeRootConstantBufferView(rootParameterIndex, volatileData);

							newVolatileCBs[newVolatileCBCount++] = VolatileConstantBufferBinding{
								.BindingPoint { rootParameterIndex },
								.Buffer { buffer },
								.Address{ volatileData }
							};
						}
						else if (updateThisSet) {
							ASSERT(0 != buffer->m_GPUVirtualAddress);

							this->m_ActiveCommandList->CommandList->SetComputeRootConstantBufferView(rootParameterIndex, buffer->m_GPUVirtualAddress);
						}
					}
					else if (updateThisSet)
						this->m_ActiveCommandList->CommandList->SetComputeRootConstantBufferView(rootParameterIndex, 0);
					// This can only happen as a result of an improperly built binding set. Such binding set should fail to create.
				}

				if (updateThisSet) {
					if (bindingSet->m_DescriptorTableValidSamplers)
						this->m_ActiveCommandList->CommandList->SetComputeRootDescriptorTable(
							rootParameterOffset + bindingSet->m_RootParameterIndexSamplers,
							this->m_DeviceResourcesRef.SamplerHeap.Get_GPUHandle(bindingSet->m_DescriptorTableSamplers)
						);
					if (bindingSet->m_DescriptorTableValidSRVetc)
						this->m_ActiveCommandList->CommandList->SetComputeRootDescriptorTable(
							rootParameterOffset + bindingSet->m_RootParameterIndexSRVetc,
							this->m_DeviceResourcesRef.ShaderResourceViewHeap.Get_GPUHandle(bindingSet->m_DescriptorTableSRVetc)
						);

					if (bindingSet->m_Desc.TrackLiveness)//TODO :Remove
						this->m_Instance->ReferencedResources.push_back(bindingSet);
				}

				if (this->m_EnableAutomaticBarriers && (updateThisSet || bindingSet->m_HasUAVBindings)) // UAV bindings may place UAV barriers on the same binding set
					this->SetResourceStatesForBindingSet(bindingSet);
			}

			this->m_CurrentComputeVolatileCBCount = 0;
			for (Uint32 Index = 0; Index < newVolatileCBCount; ++Index)
				this->m_CurrentComputeVolatileCBs[this->m_CurrentComputeVolatileCBCount++] = newVolatileCBs[Index];
		}

		if (nullptr != indirectParams && updateIndirectParams) {
			if (this->m_EnableAutomaticBarriers)
				this->m_StateTracker.RequireBufferState(&indirectParams->m_StateExtension, RHIResourceState::IndirectArgument);
			this->m_Instance->ReferencedResources.push_back(indirectParams);
		}

		Uint32 bindingMask{ (1u << BindingSetCount) - 1u };
		if ((bindingUpdateMask & bindingMask) == bindingMask)
			this->m_AnyVolatileBufferWrites = false;
		// Only reset this flag when this function has gone over all the binging sets
	}

	//TODO :Use Span
	void CommandList::SetGraphicsBindings(const Array<BindingSet*, g_MaxBindingLayoutCount> bindings, Uint32 BindingSetCount, Uint32 bindingUpdateMask, Buffer* indirectParams, bool updateIndirectParams, const D3D12RootSignature* rootSignature) {
		if (bindingUpdateMask) {
			Array<VolatileConstantBufferBinding, g_MaxVolatileConstantBuffers> newVolatileCBs{};
			RemoveCV<decltype(g_MaxVolatileConstantBuffers)>::type newVolatileCBCount{ 0 };

			for (Uint32 bindingSetIndex = 0; bindingSetIndex < BindingSetCount; ++bindingSetIndex) {
				BindingSet* bindingSet{ bindings[bindingSetIndex] };

				if (nullptr == bindingSet)
					continue;

				const bool updateThisSet{ (bindingUpdateMask & (1 << bindingSetIndex)) != 0 };

				auto& [Layout, rootParameterOffset] = rootSignature->m_BindLayouts[bindingSetIndex];
				auto bindinglayout{ Get<RefCountPtr<BindingLayout>>(Layout).Get() };

				ASSERT(bindinglayout == bindingSet->Get_Layout()); // validation layer handles this

				// Bind the volatile constant buffers
				for (Uint32 volatileCbIndex = 0; volatileCbIndex < bindingSet->m_VolatileCBCount; ++volatileCbIndex) {
					const auto& [ParameterIndex, buffer] { bindingSet->m_RootParametersVolatileCB[volatileCbIndex] };
					auto rootParameterIndex = rootParameterOffset + ParameterIndex;

					if (nullptr != buffer) {
						if (buffer->m_Desc.IsVolatile) {
							const D3D12_GPU_VIRTUAL_ADDRESS volatileData{ this->m_VolatileConstantBufferAddresses[buffer] };

							if (0 == volatileData) {
								LOG_ERROR("Volatile buffer address is null");

								continue;
							}

							if (updateThisSet || volatileData != this->m_CurrentGraphicsVolatileCBs[newVolatileCBCount].Address)
								this->m_ActiveCommandList->CommandList->SetGraphicsRootConstantBufferView(rootParameterIndex, volatileData);

							newVolatileCBs[newVolatileCBCount++] = VolatileConstantBufferBinding{
								.BindingPoint { rootParameterIndex },
								.Buffer { buffer },
								.Address{ volatileData }
							};
						}
						else if (updateThisSet) {
							ASSERT(0 != buffer->m_GPUVirtualAddress);

							this->m_ActiveCommandList->CommandList->SetGraphicsRootConstantBufferView(rootParameterIndex, buffer->m_GPUVirtualAddress);
						}
					}
					else if (updateThisSet)
						this->m_ActiveCommandList->CommandList->SetGraphicsRootConstantBufferView(rootParameterIndex, 0);
					// This can only happen as a result of an improperly built binding set.Such binding set should fail to create.
				}

				if (updateThisSet) {
					if (bindingSet->m_DescriptorTableValidSamplers)
						this->m_ActiveCommandList->CommandList->SetGraphicsRootDescriptorTable(
							rootParameterOffset + bindingSet->m_RootParameterIndexSamplers,
							this->m_DeviceResourcesRef.SamplerHeap.Get_GPUHandle(bindingSet->m_DescriptorTableSamplers)
						);
					if (bindingSet->m_DescriptorTableValidSRVetc)
						this->m_ActiveCommandList->CommandList->SetGraphicsRootDescriptorTable(
							rootParameterOffset + bindingSet->m_RootParameterIndexSRVetc,
							this->m_DeviceResourcesRef.ShaderResourceViewHeap.Get_GPUHandle(bindingSet->m_DescriptorTableSRVetc)
						);

					if (bindingSet->m_Desc.TrackLiveness)
						this->m_Instance->ReferencedResources.push_back(bindingSet);
				}

				if (this->m_EnableAutomaticBarriers && (updateThisSet || bindingSet->m_HasUAVBindings)) // UAV bindings may place UAV barriers on the same binding set
					this->SetResourceStatesForBindingSet(bindingSet);
			}

			this->m_CurrentGraphicsVolatileCBCount = 0;
			for (Uint32 Index = 0; Index < newVolatileCBCount; ++Index)
				this->m_CurrentGraphicsVolatileCBs[this->m_CurrentGraphicsVolatileCBCount++] = newVolatileCBs[Index];
		}

		if (indirectParams && updateIndirectParams) {
			if (this->m_EnableAutomaticBarriers)
				this->m_StateTracker.RequireBufferState(&indirectParams->m_StateExtension, RHIResourceState::IndirectArgument);
			this->m_Instance->ReferencedResources.push_back(indirectParams);
		}

		Uint32 bindingMask{ (1u << BindingSetCount) - 1u };
		if ((bindingUpdateMask & bindingMask) == bindingMask)
			this->m_AnyVolatileBufferWrites = false;
		// Only reset this flag when this function has gone over all the binging sets
	}

	inline void CommandList::UnbindShadingRateState(void) {
		if (this->m_CurrentGraphicsStateValid && this->m_CurrentGraphicsState.ShadingRateState.Enabled) {
			this->m_ActiveCommandList->CommandList6->RSSetShadingRateImage(nullptr);
			this->m_ActiveCommandList->CommandList6->RSSetShadingRate(D3D12_SHADING_RATE_1X1, nullptr);
			this->m_CurrentGraphicsState.ShadingRateState.Enabled = false;
			this->m_CurrentGraphicsState.FrameBuffer = nullptr;
		}
	}

	inline void CommandList::UpdateGraphicsVolatileBuffers(void) {
		// If there are some volatile buffers bound, and they have been written into since the last draw or setGraphicsState, patch their views
		if (!this->m_AnyVolatileBufferWrites)
			return;

		for (Uint32 Index = 0; Index < this->m_CurrentGraphicsVolatileCBCount; ++Index) {
			auto& parameter{ this->m_CurrentGraphicsVolatileCBs[Index] };
			auto currentGpuVA{ this->m_VolatileConstantBufferAddresses[parameter.Buffer] };

			if (currentGpuVA != parameter.Address) {
				this->m_ActiveCommandList->CommandList->SetGraphicsRootConstantBufferView(parameter.BindingPoint, currentGpuVA);

				parameter.Address = currentGpuVA;
			}
		}

		this->m_AnyVolatileBufferWrites = false;
	}

	inline void CommandList::UpdateComputeVolatileBuffers(void) {
		// If there are some volatile buffers bound, and they have been written into since the last dispatch or setComputeState, patch their views
		if (!this->m_AnyVolatileBufferWrites)
			return;

		for (Uint32 Index = 0; Index < this->m_CurrentComputeVolatileCBCount; ++Index) {
			auto& parameter{ this->m_CurrentComputeVolatileCBs[Index] };
			auto currentGpuVA{ this->m_VolatileConstantBufferAddresses[parameter.Buffer] };

			if (currentGpuVA != parameter.Address) {
				this->m_ActiveCommandList->CommandList->SetComputeRootConstantBufferView(parameter.BindingPoint, currentGpuVA);

				parameter.Address = currentGpuVA;
			}
		}

		this->m_AnyVolatileBufferWrites = false;
	}

	//Imp

	inline void CommandList::Imp_Open(void) {
		Uint64 completedInstance{ m_Queue->UpdateLastCompletedInstance() };

		SharedPtr<InternalCommandList> chunk{ nullptr };

		if (!this->m_CommandListPool.empty()) {
			chunk = this->m_CommandListPool.front();

			if (chunk->LastSubmittedInstance <= completedInstance) {
				chunk->Allocator->Reset();
				chunk->CommandList->Reset(chunk->Allocator, nullptr);
				this->m_CommandListPool.pop_front();
			}
			else
				chunk = nullptr;
		}

		if (nullptr == chunk)
			chunk = this->CreateInternalCommandList();

		this->m_ActiveCommandList = chunk;

		this->m_Instance = MakeShared<CommandListInstance>();
		this->m_Instance->CommandQueue = this->m_Desc.QueueType;
		this->m_Instance->CommandAllocator = this->m_ActiveCommandList->Allocator;
		this->m_Instance->CommandList = this->m_ActiveCommandList->CommandList;


		this->m_RecordingVersion = MakeVersion(m_Queue->m_RecordingInstance++, m_Desc.QueueType, false);
	}

	inline void CommandList::Imp_Close(void) {
		this->m_StateTracker.KeepBufferInitialStates();
		this->m_StateTracker.KeepTextureInitialStates();

		this->CommitBarriers();

		this->m_ActiveCommandList->CommandList->Close();

		this->ClearStateCache();

		this->m_CurrentUploadBuffer = nullptr;
		this->m_VolatileConstantBufferAddresses.clear();
	}

	inline void CommandList::Imp_ClearTextureFloat(Texture* texture, RHITextureSubresourceSet subresources, const Color& color) {
		subresources = subresources.Resolve(texture->m_Desc, false);

		this->m_Instance->ReferencedResources.push_back(texture);

		if (texture->m_Desc.IsRenderTarget) {
			if (this->m_EnableAutomaticBarriers)
				this->m_StateTracker.RequireTextureState(&texture->m_StateExtension, subresources, RHIResourceState::RenderTarget);
			this->CommitBarriers();

			for (Uint32 mipLevel = subresources.BaseMipLevel; mipLevel < subresources.BaseMipLevel + subresources.MipLevelCount; ++mipLevel) {
				D3D12_CPU_DESCRIPTOR_HANDLE RTV{ .ptr { texture->GetNativeView(RHIObjectType::D3D12_RenderTargetViewDescriptor, RHIFormat::UNKNOWN, subresources, RHITextureDimension::Unknown).Integer } };

				m_ActiveCommandList->CommandList->ClearRenderTargetView(
					RTV,
					&color.R,
					0, nullptr
				);
			}
		}
		else {
			if (this->m_EnableAutomaticBarriers)
				this->m_StateTracker.RequireTextureState(&texture->m_StateExtension, subresources, RHIResourceState::UnorderedAccess);
			this->CommitBarriers();

			this->CommitDescriptorHeaps();

			for (Uint32 mipLevel = subresources.BaseMipLevel; mipLevel < subresources.BaseMipLevel + subresources.MipLevelCount; ++mipLevel) {
				D3D12DescriptorIndex index = texture->Get_ClearMipLevelUAV(mipLevel);

				ASSERT(index != g_InvalidDescriptorIndex);

				this->m_ActiveCommandList->CommandList->ClearUnorderedAccessViewFloat(
					this->m_DeviceResourcesRef.ShaderResourceViewHeap.Get_GPUHandle(index),
					this->m_DeviceResourcesRef.ShaderResourceViewHeap.Get_CPUHandle(index),
					texture->m_Resource.Get(),
					&color.R,
					0, nullptr
				);
			}
		}
	}

	inline void CommandList::Imp_ClearDepthStencilTexture(Texture* texture, RHITextureSubresourceSet subresources, Optional<float> depth, Optional<Uint8> stencil) {
		ASSERT(!(NullOpt == depth && NullOpt == stencil));

		subresources = subresources.Resolve(texture->m_Desc, false);

		this->m_Instance->ReferencedResources.push_back(texture);

		if (this->m_EnableAutomaticBarriers)
			this->m_StateTracker.RequireTextureState(&texture->m_StateExtension, subresources, RHIResourceState::DepthWrite);
		this->CommitBarriers();

		D3D12_CLEAR_FLAGS clearFlags{ D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL };
		if (NullOpt == depth)
			clearFlags = D3D12_CLEAR_FLAG_STENCIL;
		else if (NullOpt == stencil)
			clearFlags = D3D12_CLEAR_FLAG_DEPTH;

		for (Uint32 mipLevel = subresources.BaseMipLevel; mipLevel < subresources.BaseMipLevel + subresources.MipLevelCount; ++mipLevel) {
			D3D12_CPU_DESCRIPTOR_HANDLE DSV{ .ptr{ texture->GetNativeView(RHIObjectType::D3D12_DepthStencilViewDescriptor, RHIFormat::UNKNOWN, subresources, RHITextureDimension::Unknown).Integer } };

			this->m_ActiveCommandList->CommandList->ClearDepthStencilView(
				DSV,
				clearFlags,
				depth.has_value() ? depth.value() : 0.0f,
				stencil.has_value() ? stencil.value() : 0,
				0, nullptr
			);
		}
	}

	inline void CommandList::Imp_ClearTextureUInt(Texture* texture, RHITextureSubresourceSet subresources, Uint32 clearColor) {//TODO :const&
		subresources = subresources.Resolve(texture->m_Desc, false);

		this->m_Instance->ReferencedResources.push_back(texture);

		if (texture->m_Desc.IsUAV) {
			if (this->m_EnableAutomaticBarriers)
				this->m_StateTracker.RequireTextureState(&texture->m_StateExtension, subresources, RHIResourceState::UnorderedAccess);
			this->CommitBarriers();

			this->CommitDescriptorHeaps();

			for (Uint32 mipLevel = subresources.BaseMipLevel; mipLevel < subresources.BaseMipLevel + subresources.MipLevelCount; ++mipLevel) {
				D3D12DescriptorIndex index{ texture->Get_ClearMipLevelUAV(mipLevel) };

				ASSERT(index != g_InvalidDescriptorIndex);

				Uint32 clearValues[4]{ clearColor, clearColor, clearColor, clearColor };

				this->m_ActiveCommandList->CommandList->ClearUnorderedAccessViewUint(
					this->m_DeviceResourcesRef.ShaderResourceViewHeap.Get_GPUHandle(index),
					this->m_DeviceResourcesRef.ShaderResourceViewHeap.Get_CPUHandle(index),
					texture->m_Resource,
					clearValues,
					0, nullptr
				);
			}
		}
		else if (texture->m_Desc.IsRenderTarget) {
			if (this->m_EnableAutomaticBarriers)
				this->m_StateTracker.RequireTextureState(&texture->m_StateExtension, subresources, RHIResourceState::RenderTarget);
			this->CommitBarriers();

			for (Uint32 mipLevel = subresources.BaseMipLevel; mipLevel < subresources.BaseMipLevel + subresources.MipLevelCount; ++mipLevel) {
				D3D12_CPU_DESCRIPTOR_HANDLE RTV{ .ptr{ texture->GetNativeView(RHIObjectType::D3D12_RenderTargetViewDescriptor, RHIFormat::UNKNOWN, subresources, RHITextureDimension::Unknown).Integer} };

				float floatColor[4]{
					static_cast<float>(clearColor),
					static_cast<float>(clearColor),
					static_cast<float>(clearColor),
					static_cast<float>(clearColor)
				};

				this->m_ActiveCommandList->CommandList->ClearRenderTargetView(RTV, floatColor, 0, nullptr);
			}
		}
	}

	inline void CommandList::Imp_CopyTexture(Texture* dst, RHITextureSlice desSlice, Texture* src, RHITextureSlice srcSlice) {
		auto resolvedDstSlice{ desSlice.Resolve(dst->m_Desc) };
		auto resolvedSrcSlice{ srcSlice.Resolve(src->m_Desc) };

		Uint32 dstSubresource{ CalcSubresource(resolvedDstSlice.MipLevel, resolvedDstSlice.ArraySlice, 0, dst->m_Desc.MipLevelCount, dst->m_Desc.ArrayCount) };
		Uint32 srcSubresource{ CalcSubresource(resolvedSrcSlice.MipLevel, resolvedSrcSlice.ArraySlice, 0, src->m_Desc.MipLevelCount, src->m_Desc.ArrayCount) };

		D3D12_TEXTURE_COPY_LOCATION dstLocation{
			.pResource { dst->m_Resource },
			.Type { D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX },
			.SubresourceIndex { dstSubresource }
		};

		D3D12_TEXTURE_COPY_LOCATION srcLocation{
			.pResource { src->m_Resource },
			.Type { D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX },
			.SubresourceIndex { srcSubresource }
		};

		D3D12_BOX srcBox{
			.left { resolvedSrcSlice.Offset.X },
			.top { resolvedSrcSlice.Offset.Y },
			.front { resolvedSrcSlice.Offset.Z },
			.right { resolvedSrcSlice.Offset.X + resolvedSrcSlice.Extent.Width },
			.bottom { resolvedSrcSlice.Offset.Y + resolvedSrcSlice.Extent.Height },
			.back { resolvedSrcSlice.Offset.Z + resolvedSrcSlice.Extent.Depth }
		};

		if (m_EnableAutomaticBarriers) {
			this->m_StateTracker.RequireTextureState(
				&dst->m_StateExtension,
				RHITextureSubresourceSet{ .BaseMipLevel{ resolvedDstSlice.MipLevel }, .MipLevelCount{ 1 }, .BaseArraySlice{ resolvedDstSlice.ArraySlice }, .ArraySliceCount { 1 } },
				RHIResourceState::CopyDest
			);
			this->m_StateTracker.RequireTextureState(
				&src->m_StateExtension,
				RHITextureSubresourceSet{ .BaseMipLevel{ resolvedSrcSlice.MipLevel }, .MipLevelCount{ 1 }, .BaseArraySlice{ resolvedSrcSlice.ArraySlice }, .ArraySliceCount { 1 } },
				RHIResourceState::CopySource
			);
		}
		this->CommitBarriers();

		this->m_Instance->ReferencedResources.push_back(dst);
		this->m_Instance->ReferencedResources.push_back(src);

		this->m_ActiveCommandList->CommandList->CopyTextureRegion(
			&dstLocation,
			resolvedDstSlice.Offset.X, resolvedDstSlice.Offset.Y, resolvedDstSlice.Offset.Z,
			&srcLocation, &srcBox
		);
	}

	inline void CommandList::Imp_CopyTexture(Texture* dst, RHITextureSlice dstSlice, StagingTexture* src, RHITextureSlice srcSlice) {
		auto resolvedDstSlice{ dstSlice.Resolve(dst->m_Desc) };
		auto resolvedSrcSlice{ srcSlice.Resolve(src->m_Desc) };

		Uint32 dstSubresource{ CalcSubresource(resolvedDstSlice.MipLevel, resolvedDstSlice.ArraySlice, 0, dst->m_Desc.MipLevelCount, dst->m_Desc.ArrayCount) };

		if (this->m_EnableAutomaticBarriers) {
			this->m_StateTracker.RequireTextureState(
				&dst->m_StateExtension,
				RHITextureSubresourceSet{ .BaseMipLevel{ resolvedDstSlice.MipLevel }, .MipLevelCount{ 1 }, .BaseArraySlice{ resolvedDstSlice.ArraySlice }, .ArraySliceCount { 1 } },
				RHIResourceState::CopyDest
			);
			this->m_StateTracker.RequireBufferState(&src->m_Buffer->m_StateExtension, RHIResourceState::CopySource);
		}
		this->CommitBarriers();

		this->m_Instance->ReferencedResources.push_back(dst);
		this->m_Instance->ReferencedStagingTextures.push_back(src);

		auto srcRegion{ src->Get_SliceRegion(m_Context.Device, resolvedSrcSlice) };

		D3D12_TEXTURE_COPY_LOCATION dstLocation{
			.pResource { dst->m_Resource },
			.Type { D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX },
			.SubresourceIndex { dstSubresource }
		};

		D3D12_TEXTURE_COPY_LOCATION srcLocation{
			.pResource { src->m_Buffer->m_Resource },
			.Type { D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT },
			.PlacedFootprint { srcRegion.Footprint }
		};

		D3D12_BOX srcBox{
			.left { resolvedSrcSlice.Offset.X },
			.top { resolvedSrcSlice.Offset.Y },
			.front { resolvedSrcSlice.Offset.Z },
			.right { resolvedSrcSlice.Offset.X + resolvedSrcSlice.Extent.Width },
			.bottom { resolvedSrcSlice.Offset.Y + resolvedSrcSlice.Extent.Height },
			.back { resolvedSrcSlice.Offset.Z + resolvedSrcSlice.Extent.Depth }
		};

		this->m_ActiveCommandList->CommandList->CopyTextureRegion(
			&dstLocation,
			resolvedDstSlice.Offset.X, resolvedDstSlice.Offset.Y, resolvedDstSlice.Offset.Z,
			&srcLocation, &srcBox
		);
	}

	inline void CommandList::Imp_CopyTexture(StagingTexture* dst, RHITextureSlice dstSlice, Texture* src, RHITextureSlice srcSlice) {
		auto resolvedDstSlice{ dstSlice.Resolve(dst->m_Desc) };
		auto resolvedSrcSlice{ srcSlice.Resolve(src->m_Desc) };

		Uint32 srcSubresource{ CalcSubresource(resolvedSrcSlice.MipLevel, resolvedSrcSlice.ArraySlice, 0, src->m_Desc.MipLevelCount, src->m_Desc.ArrayCount) };

		if (this->m_EnableAutomaticBarriers) {
			this->m_StateTracker.RequireBufferState(&dst->m_Buffer->m_StateExtension, RHIResourceState::CopyDest);
			this->m_StateTracker.RequireTextureState(
				&src->m_StateExtension,
				RHITextureSubresourceSet{ .BaseMipLevel{ resolvedSrcSlice.MipLevel }, .MipLevelCount{ 1 }, .BaseArraySlice{ resolvedSrcSlice.ArraySlice }, .ArraySliceCount { 1 } },
				RHIResourceState::CopySource
			);
		}
		this->CommitBarriers();

		this->m_Instance->ReferencedResources.push_back(src);
		this->m_Instance->ReferencedStagingTextures.push_back(dst);

		auto dstRegion{ dst->Get_SliceRegion(m_Context.Device, resolvedDstSlice) };

		D3D12_TEXTURE_COPY_LOCATION dstLocation{
			.pResource { dst->m_Buffer->m_Resource },
			.Type { D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT },
			.PlacedFootprint { dstLocation.PlacedFootprint }
		};

		D3D12_TEXTURE_COPY_LOCATION srcLocation{
			.pResource { src->m_Resource },
			.Type { D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX },
			.SubresourceIndex { srcSubresource }
		};

		D3D12_BOX srcBox{
			.left { resolvedSrcSlice.Offset.X },
			.top { resolvedSrcSlice.Offset.Y },
			.front { resolvedSrcSlice.Offset.Z },
			.right { resolvedSrcSlice.Offset.X + resolvedSrcSlice.Extent.Width },
			.bottom { resolvedSrcSlice.Offset.Y + resolvedSrcSlice.Extent.Height },
			.back { resolvedSrcSlice.Offset.Z + resolvedSrcSlice.Extent.Depth }
		};

		this->m_ActiveCommandList->CommandList->CopyTextureRegion(
			&dstLocation,
			resolvedDstSlice.Offset.X, resolvedDstSlice.Offset.Y, resolvedDstSlice.Offset.Z,
			&srcLocation, &srcBox
		);
	}

	inline void CommandList::Imp_WriteTexture(Texture* dest, Uint32 arraySlice, Uint32 mipLevel, const void* data, Uint64 RowPitch, Uint64 DepthPitch) {
		if (this->m_EnableAutomaticBarriers)
			this->m_StateTracker.RequireTextureState(
				&dest->m_StateExtension,
				RHITextureSubresourceSet{ .BaseMipLevel{ mipLevel }, .BaseArraySlice{ arraySlice } },
				RHIResourceState::CopyDest
			);

		this->CommitBarriers();

		Uint32 subresource{ CalcSubresource(mipLevel, arraySlice, 0, dest->m_Desc.MipLevelCount, dest->m_Desc.ArrayCount) };

		const auto& resourceDesc{ dest->m_Resource->GetDesc() };
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
		Uint32 numRows;
		Uint64 rowSizeInBytes;
		Uint64 totalBytes;
		this->m_Context.Device->GetCopyableFootprints(&resourceDesc, subresource, 1, 0, &footprint, &numRows, &rowSizeInBytes, &totalBytes);

		void* cpuVA;
		ID3D12Resource* uploadBuffer;
		Uint64 offsetInUploadBuffer;
		if (!this->m_UploadManager.SuballocateBuffer(totalBytes, nullptr, &uploadBuffer, &offsetInUploadBuffer, &cpuVA, nullptr, this->m_RecordingVersion, g_D3D12TextureDataPlacementAlignment)) {
			LOG_ERROR("Failed to allocate upload buffer for texture copy");

			return;
		}
		footprint.Offset = static_cast<Uint64>(offsetInUploadBuffer);

		ASSERT(numRows <= footprint.Footprint.Height);

		for (Uint32 depthSlice = 0; depthSlice < footprint.Footprint.Depth; ++depthSlice)
			for (Uint32 row = 0; row < numRows; ++row) {
				void* destAddress{ reinterpret_cast<char*>(cpuVA) + static_cast<Uint64>(footprint.Footprint.RowPitch) * static_cast<Uint64>(row + depthSlice * numRows) };//GPU has row algnment so not use DepthPitch
				const void* srcAddress{ static_cast<const char*>(data) + RowPitch * row + DepthPitch * depthSlice };//CPU hash not row alignment,use odd row to less pading data copy
				memcpy(destAddress, srcAddress, Math::Min(RowPitch, rowSizeInBytes));//TODO : RowPitch maybe is zero....
			}

		D3D12_TEXTURE_COPY_LOCATION destCopyLocation{
			.pResource { dest->m_Resource },
			.Type { D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX },
			.SubresourceIndex { subresource }
		};

		D3D12_TEXTURE_COPY_LOCATION srcCopyLocation{
			.pResource { uploadBuffer },
			.Type { D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT },
			.PlacedFootprint { footprint }
		};

		this->m_Instance->ReferencedResources.push_back(dest);

		if (uploadBuffer != this->m_CurrentUploadBuffer) {
			this->m_Instance->ReferencedNativeResources.push_back(uploadBuffer);
			this->m_CurrentUploadBuffer = uploadBuffer;
		}

		this->m_ActiveCommandList->CommandList->CopyTextureRegion(&destCopyLocation, 0, 0, 0, &srcCopyLocation, nullptr);
	}

	inline void CommandList::Imp_ResolveTexture(Texture* dest, const RHITextureSubresourceSet& dstSubresources, Texture* src, const RHITextureSubresourceSet& srcSubresources) {
		auto dstSR{ dstSubresources.Resolve(dest->m_Desc, false) };
		auto srcSR{ srcSubresources.Resolve(src->m_Desc, false) };

		// let the validation layer handle the messages
		if (dstSR.ArraySliceCount != srcSR.ArraySliceCount || dstSR.MipLevelCount != srcSR.MipLevelCount)
			return;

		if (this->m_EnableAutomaticBarriers) {
			this->m_StateTracker.RequireTextureState(&dest->m_StateExtension, dstSubresources, RHIResourceState::ResolveDest);
			this->m_StateTracker.RequireTextureState(&src->m_StateExtension, srcSubresources, RHIResourceState::ResolveSource);
		}
		this->CommitBarriers();

		const auto& formatMapping{ Get_DXGIFormatMapping(dest->m_Desc.Format) };

		for (int plane = 0; plane < dest->m_PlaneCount; plane++)
			for (Uint32 arrayIndex = 0; arrayIndex < dstSR.ArraySliceCount; ++arrayIndex)
				for (Uint32 mipLevel = 0; mipLevel < dstSR.MipLevelCount; ++mipLevel) {
					uint32_t dstSubresource = CalcSubresource(mipLevel + dstSR.BaseMipLevel, arrayIndex + dstSR.BaseArraySlice, plane, dest->m_Desc.MipLevelCount, dest->m_Desc.ArrayCount);
					uint32_t srcSubresource = CalcSubresource(mipLevel + srcSR.BaseMipLevel, arrayIndex + srcSR.BaseArraySlice, plane, src->m_Desc.MipLevelCount, src->m_Desc.ArrayCount);
					this->m_ActiveCommandList->CommandList->ResolveSubresource(dest->m_Resource, dstSubresource, src->m_Resource, srcSubresource, formatMapping.RTVFormat);
				}
	}

	inline void CommandList::Imp_WriteBuffer(Buffer* buffer, const void* data, Uint64 dataSize, Uint64 destOffsetBytes) {
		void* cpuVA;
		D3D12_GPU_VIRTUAL_ADDRESS gpuVA;
		ID3D12Resource* uploadBuffer;
		Uint64 offsetInUploadBuffer;
		if (!this->m_UploadManager.SuballocateBuffer(dataSize, nullptr, &uploadBuffer, &offsetInUploadBuffer, &cpuVA, &gpuVA, this->m_RecordingVersion, g_D3D12ConstantBufferDataPlacementAlignment)) {
			LOG_ERROR("Failed to allocate upload buffer for buffer copy");

			return;
		}

		if (uploadBuffer != this->m_CurrentUploadBuffer) {
			this->m_Instance->ReferencedNativeResources.push_back(uploadBuffer);
			this->m_CurrentUploadBuffer = uploadBuffer;
		}

		memcpy(cpuVA, data, dataSize);

		if (buffer->m_Desc.IsVolatile) {
			this->m_VolatileConstantBufferAddresses[buffer] = gpuVA;
			this->m_AnyVolatileBufferWrites = true;
		}
		else {
			if (this->m_EnableAutomaticBarriers)
				this->m_StateTracker.RequireBufferState(&buffer->m_StateExtension, RHIResourceState::CopyDest);
			this->CommitBarriers();

			this->m_Instance->ReferencedResources.push_back(buffer);

			this->m_ActiveCommandList->CommandList->CopyBufferRegion(buffer->m_Resource, destOffsetBytes, uploadBuffer, offsetInUploadBuffer, dataSize);
		}
	}

	inline void CommandList::Imp_ClearBufferUInt(Buffer* buffer, Uint32 clearvalue) {
		if (!buffer->m_Desc.CanHaveUAVs) {
			LOG_ERROR("Buffer does not support UAVs");

			return;
		}

		if (this->m_EnableAutomaticBarriers)
			this->m_StateTracker.RequireBufferState(&buffer->m_StateExtension, RHIResourceState::UnorderedAccess);
		this->CommitBarriers();

		this->CommitDescriptorHeaps();

		auto clearUAV{ buffer->Get_ClearUAV() };
		ASSERT(clearUAV != g_InvalidDescriptorIndex);

		this->m_Instance->ReferencedResources.push_back(buffer);

		const Uint32 values[4] = { clearvalue, clearvalue, clearvalue, clearvalue };
		this->m_ActiveCommandList->CommandList->ClearUnorderedAccessViewUint(
			this->m_DeviceResourcesRef.ShaderResourceViewHeap.Get_GPUHandle(clearUAV),
			this->m_DeviceResourcesRef.ShaderResourceViewHeap.Get_CPUHandle(clearUAV),
			buffer->m_Resource,
			values,
			0, nullptr
		);
	}

	inline void CommandList::Imp_CopyBuffer(Buffer* dest, Uint64 destOffsetBytes, Buffer* src, Uint64 srcOffsetBytes, Uint64 dataSizeBytes) {
		if (this->m_EnableAutomaticBarriers) {
			this->m_StateTracker.RequireBufferState(&dest->m_StateExtension, RHIResourceState::CopyDest);
			this->m_StateTracker.RequireBufferState(&src->m_StateExtension, RHIResourceState::CopySource);
		}
		this->CommitBarriers();

		if (RHICPUAccessMode::None != src->m_Desc.CPUAccess)
			this->m_Instance->ReferencedStagingBuffers.push_back(src);
		else
			this->m_Instance->ReferencedResources.push_back(src);

		if (RHICPUAccessMode::None != dest->m_Desc.CPUAccess)
			this->m_Instance->ReferencedStagingBuffers.push_back(dest);
		else
			this->m_Instance->ReferencedResources.push_back(dest);

		this->m_ActiveCommandList->CommandList->CopyBufferRegion(dest->m_Resource, destOffsetBytes, src->m_Resource, srcOffsetBytes, dataSizeBytes);
	}

	inline void CommandList::Imp_ClearSamplerFeedbackTexture(SamplerFeedbackTexture* texture) {
		D3D12DescriptorIndex& descriptorIndex{ texture->m_ClearDescriptorIndex };
		if (g_InvalidDescriptorIndex == descriptorIndex) {
			descriptorIndex = this->m_DeviceResourcesRef.ShaderResourceViewHeap.AllocateDescriptor();
			texture->CreateUAV(this->m_DeviceResourcesRef.ShaderResourceViewHeap.Get_CPUHandle(descriptorIndex));
			this->m_DeviceResourcesRef.ShaderResourceViewHeap.CopyToShaderVisibleHeap(descriptorIndex);
		}

		this->CommitDescriptorHeaps();

		const Uint32 clearValue[4]{ 0xFF, 0xFF, 0xFF, 0xFF };
		this->m_ActiveCommandList->CommandList->ClearUnorderedAccessViewUint(
			this->m_DeviceResourcesRef.ShaderResourceViewHeap.Get_GPUHandle(descriptorIndex),
			this->m_DeviceResourcesRef.ShaderResourceViewHeap.Get_CPUHandle(descriptorIndex),
			texture->m_Resource,
			clearValue,
			0, nullptr
		);
	}

	inline void CommandList::Imp_DecodeSamplerFeedbackTexture(Buffer* buffer, SamplerFeedbackTexture* texture, RHIFormat format) {
		if (this->m_EnableAutomaticBarriers) {
			this->m_StateTracker.RequireBufferState(&buffer->m_StateExtension, RHIResourceState::ResolveDest);
			this->m_StateTracker.RequireTextureState(&texture->m_TextureStateExtension, g_AllSubResourceSet, RHIResourceState::ResolveSource);
		}
		this->CommitBarriers();

		const auto& formatMapping{ Get_DXGIFormatMapping(format) };

		this->m_ActiveCommandList->CommandList4->ResolveSubresourceRegion(
			buffer->m_Resource,
			0, 0, 0,
			texture->m_Resource, 0, nullptr,
			formatMapping.SRVFormat,
			D3D12_RESOLVE_MODE_DECODE_SAMPLER_FEEDBACK
		);
	}

	inline void CommandList::Imp_SetSamplerFeedbackTextureState(SamplerFeedbackTexture* texture, RHIResourceState state) {
		this->m_StateTracker.RequireTextureState(&texture->m_TextureStateExtension, g_AllSubResourceSet, RHIResourceState::ResolveSource);
	}

	inline void CommandList::Imp_SetPushConstants(const void* data, Uint32 ByteSize) {
		const D3D12RootSignature* rootsig{ nullptr };
		bool isGraphics{ false };

		//TODO
		if (this->m_CurrentGraphicsStateValid && nullptr != this->m_CurrentGraphicsState.Pipeline) {
			GraphicsPipeline* pso = this->m_CurrentGraphicsState.Pipeline;
			rootsig = pso->m_RootSignature;
			isGraphics = true;
		}
		else if (this->m_CurrentComputeStateValid && nullptr != this->m_CurrentComputeState.Pipeline) {
			ComputePipeline* pso = this->m_CurrentComputeState.Pipeline;
			rootsig = pso->m_RootSignature;
			isGraphics = false;
		}
		else if (this->m_CurrentMeshletStateValid && nullptr != this->m_CurrentMeshletState.Pipeline) {
			MeshletPipeline* pso = this->m_CurrentMeshletState.Pipeline;
			rootsig = pso->m_RootSignature;
			isGraphics = true;
		}

		if (nullptr == rootsig || 0 == rootsig->m_PushConstantByteSize)
			return;

		ASSERT(ByteSize == rootsig->m_PushConstantByteSize); // the validation error handles the error message

		if (isGraphics)
			this->m_ActiveCommandList->CommandList->SetGraphicsRoot32BitConstants(rootsig->m_RootParameterPushConstants, static_cast<Uint32>(ByteSize / 4), data, 0);
		else
			this->m_ActiveCommandList->CommandList->SetComputeRoot32BitConstants(rootsig->m_RootParameterPushConstants, static_cast<Uint32>(ByteSize / 4), data, 0);
	}

	void CommandList::Imp_SetGraphicsState(const RHIGraphicsState<D3D12Tag>& State) {
		auto pso{ State.Pipeline };
		auto framebuffer{ State.FrameBuffer };

		const bool updateFramebuffer{
			!this->m_CurrentGraphicsStateValid ||
			this->m_CurrentGraphicsState.FrameBuffer != framebuffer
		};
		const bool updateRootSignature{
			!this->m_CurrentGraphicsStateValid ||
			nullptr == this->m_CurrentGraphicsState.Pipeline ||
			this->m_CurrentGraphicsState.Pipeline->m_RootSignature != pso->m_RootSignature
		};

		const bool updatePipeline{
			!this->m_CurrentGraphicsStateValid ||
			this->m_CurrentGraphicsState.Pipeline != pso
		};

		const bool updateIndirectParams{
			!this->m_CurrentGraphicsStateValid ||
			this->m_CurrentGraphicsState.IndirectParams != State.IndirectParams
		};

		const bool updateViewports{
			!this->m_CurrentGraphicsStateValid ||
			this->m_CurrentGraphicsState.Viewport != State.Viewport
		};

		const bool updateBlendFactor{
			!this->m_CurrentGraphicsStateValid ||
			this->m_CurrentGraphicsState.BlendConstantColor != State.BlendConstantColor
		};

		const Uint8 effectiveStencilRefValue{
			pso->m_Desc.RenderState.DepthStencilState.DynamicStencilRef
			? State.DynamicStencilRefValue
			: pso->m_Desc.RenderState.DepthStencilState.StencilRefValue
		};

		const bool updateStencilRef{
			!this->m_CurrentGraphicsStateValid ||
			this->m_CurrentGraphicsState.DynamicStencilRefValue != effectiveStencilRefValue
		};

		const bool updateIndexBuffer{
			!this->m_CurrentGraphicsStateValid ||
			this->m_CurrentGraphicsState.IndexBuffer != State.IndexBuffer
		};

		const bool updateVertexBuffers{
			!this->m_CurrentGraphicsStateValid ||
			ArrayEqual(
				this->m_CurrentGraphicsState.VertexBuffers, this->m_CurrentGraphicsState.VertexBufferCount ,
				State.VertexBuffers,State.VertexBufferCount
			)
		};

		const bool updateShadingRate{
			!this->m_CurrentGraphicsStateValid ||
			this->m_CurrentGraphicsState.ShadingRateState != State.ShadingRateState
		};

		Uint32 bindingUpdateMask{ 0 };
		if (!this->m_CurrentGraphicsStateValid || updateRootSignature)
			bindingUpdateMask = ~0u;

		if (this->CommitDescriptorHeaps())
			bindingUpdateMask = ~0u;

		if (0 == bindingUpdateMask)
			bindingUpdateMask = Array32DifferenceMask(this->m_CurrentGraphicsState.BindingSets, State.BindingSets);

		if (updatePipeline) {
			this->BindGraphicsPipeline(pso, updateRootSignature);
			this->m_Instance->ReferencedResources.push_back(pso);
		}

		if (pso->m_Desc.RenderState.DepthStencilState.StencilEnable && (updatePipeline || updateStencilRef))
			m_ActiveCommandList->CommandList->OMSetStencilRef(effectiveStencilRefValue);

		if (pso->m_RequiresBlendFactor && updateBlendFactor)
			m_ActiveCommandList->CommandList->OMSetBlendFactor(&State.BlendConstantColor.R);

		if (updateFramebuffer)
			this->BindFramebuffer(framebuffer);
		this->m_Instance->ReferencedResources.push_back(framebuffer);

		this->SetGraphicsBindings(State.BindingSets, State.BindingSetCount, bindingUpdateMask, State.IndirectParams, updateIndirectParams, pso->m_RootSignature);

		if (updateIndexBuffer) {
			D3D12_INDEX_BUFFER_VIEW IBV{};

			if (nullptr != State.IndexBuffer.Buffer) {
				Buffer* buffer = State.IndexBuffer.Buffer;

				if (this->m_EnableAutomaticBarriers)
					this->m_StateTracker.RequireBufferState(&buffer->m_StateExtension, RHIResourceState::IndexBuffer);

				IBV.Format = Get_DXGIFormatMapping(State.IndexBuffer.Format).SRVFormat;
				IBV.SizeInBytes = static_cast<Uint32>(buffer->m_Desc.ByteSize - State.IndexBuffer.Offset);
				IBV.BufferLocation = buffer->m_GPUVirtualAddress + State.IndexBuffer.Offset;

				m_Instance->ReferencedResources.push_back(State.IndexBuffer.Buffer);
			}

			this->m_ActiveCommandList->CommandList->IASetIndexBuffer(&IBV);
		}

		if (updateVertexBuffers)
		{
			D3D12_VERTEX_BUFFER_VIEW VBVs[g_MaxVertexAttributeCount]{};
			Uint32 maxVbIndex{ 0 };
			InputLayout* inputLayout{ pso->m_Desc.InputLayout.Get() };

			for (Uint32 Index = 0; Index < State.VertexBufferCount; ++Index) {
				const auto& binding{ State.VertexBuffers[Index] };

				Buffer* buffer{ binding.Buffer };

				if (this->m_EnableAutomaticBarriers)
					this->m_StateTracker.RequireBufferState(&buffer->m_StateExtension, RHIResourceState::VertexBuffer);

				ASSERT(binding.Slot < g_MaxVertexAttributeCount);

				VBVs[binding.Slot].StrideInBytes = inputLayout->m_ElementStrides[binding.Slot];
				VBVs[binding.Slot].SizeInBytes = static_cast<Uint32>(Math::Min(buffer->m_Desc.ByteSize - binding.Offset, static_cast<Uint64>(Max_Uint32)));
				VBVs[binding.Slot].BufferLocation = buffer->m_GPUVirtualAddress + binding.Offset;
				maxVbIndex = Math::Max(maxVbIndex, binding.Slot);

				m_Instance->ReferencedResources.push_back(buffer);
			}

			if (this->m_CurrentGraphicsStateValid)
				for (Uint32 Index = 0; Index < this->m_CurrentGraphicsState.VertexBufferCount; ++Index) {
					const auto& binding = this->m_CurrentGraphicsState.VertexBuffers[Index];

					if (binding.Slot < g_MaxVertexAttributeCount)
						maxVbIndex = Math::Max(maxVbIndex, binding.Slot);
				}
			this->m_ActiveCommandList->CommandList->IASetVertexBuffers(0, maxVbIndex + 1, VBVs);
		}

		if (updateShadingRate || updateFramebuffer) {
			const auto& framebufferDesc{ framebuffer->Get_Desc() };
			bool shouldEnableVariableRateShading{ framebufferDesc.ShadingRateAttachment.Is_Valid() && State.ShadingRateState.Enabled };
			bool variableRateShadingCurrentlyEnabled{
				this->m_CurrentGraphicsStateValid &&
				this->m_CurrentGraphicsState.FrameBuffer->Get_Desc().ShadingRateAttachment.Is_Valid() &&
				this->m_CurrentGraphicsState.ShadingRateState.Enabled
			};

			if (shouldEnableVariableRateShading) {
				this->m_StateTracker.RequireTextureState(
					&framebufferDesc.ShadingRateAttachment.Texture->m_StateExtension,
					RHITextureSubresourceSet{ .BaseMipLevel{ 0 }, .MipLevelCount{ 1 }, .BaseArraySlice{ 0 }, .ArraySliceCount { 1 } },
					RHIResourceState::ShadingRateSurface
				);
				if (nullptr != this->m_Instance)
					this->m_Instance->ReferencedResources.push_back(framebufferDesc.ShadingRateAttachment.Texture);

				m_ActiveCommandList->CommandList6->RSSetShadingRateImage(framebufferDesc.ShadingRateAttachment.Texture->m_Resource);
			}
			else if (variableRateShadingCurrentlyEnabled)
				this->m_ActiveCommandList->CommandList6->RSSetShadingRateImage(nullptr);
			// shading rate attachment is not enabled in framebuffer, or VRS is turned off, so unbind VRS image
		}

		if (updateShadingRate) {
			if (State.ShadingRateState.Enabled) {
				D3D12_SHADING_RATE_COMBINER combiners[g_D3D12ReSetShadingRateCombinerCount]{};
				combiners[0] = ConvertShadingRateCombiner(State.ShadingRateState.PipelinePrimitiveCombiner);
				combiners[1] = ConvertShadingRateCombiner(State.ShadingRateState.ImageCombiner);
				this->m_ActiveCommandList->CommandList6->RSSetShadingRate(ConvertPixelShadingRate(State.ShadingRateState.ShadingRate), combiners);
			}
			else if (this->m_CurrentGraphicsStateValid && this->m_CurrentGraphicsState.ShadingRateState.Enabled)
				this->m_ActiveCommandList->CommandList6->RSSetShadingRate(D3D12_SHADING_RATE_1X1, nullptr);
			// only call if the old state had VRS enabled and we need to disable it
		}

		this->CommitBarriers();

		if (updateViewports) {
			ViewportState vpState{ ViewportState::Convert(pso->m_Desc.RenderState.RasterState, framebuffer->m_Info, State.Viewport) };

			if (0 != vpState.ViewportCount)
				this->m_ActiveCommandList->CommandList->RSSetViewports(vpState.ViewportCount, vpState.Viewports.data());

			if (0 != vpState.ScissorCount)
				this->m_ActiveCommandList->CommandList->RSSetScissorRects(vpState.ScissorCount, vpState.Scissors.data());
		}


		this->m_CurrentGraphicsStateValid = true;
		this->m_CurrentComputeStateValid = false;
		this->m_CurrentMeshletStateValid = false;
		this->m_CurrentGraphicsState = State;
		this->m_CurrentGraphicsState.DynamicStencilRefValue = effectiveStencilRefValue;
	}

	inline void CommandList::Imp_Draw(const RHIDrawArguments& args) {
		this->UpdateGraphicsVolatileBuffers();

		this->m_ActiveCommandList->CommandList->DrawInstanced(args.VertexCount, args.InstanceCount, args.StartVertexLocation, args.StartInstanceLocation);
	}

	inline void CommandList::Imp_DrawIndexed(const RHIDrawArguments& args) {
		this->UpdateGraphicsVolatileBuffers();

		this->m_ActiveCommandList->CommandList->DrawIndexedInstanced(args.VertexCount, args.InstanceCount, args.StartIndexLocation, args.StartVertexLocation, args.StartInstanceLocation);
	}

	inline void CommandList::Imp_DrawIndirect(Uint32 OffsetBytes, Uint32 Count) {
		Buffer* indirectParams{ this->m_CurrentGraphicsState.IndirectParams };
		ASSERT(nullptr != indirectParams); // validation layer handles this

		this->UpdateGraphicsVolatileBuffers();

		this->m_ActiveCommandList->CommandList->ExecuteIndirect(this->m_Context.DrawIndirectSignature, Count, indirectParams->m_Resource, OffsetBytes, nullptr, 0);
	}

	inline void CommandList::Imp_DrawIndexedIndirect(Uint32 OffsetBytes, Uint32 Count) {
		Buffer* indirectParams{ this->m_CurrentGraphicsState.IndirectParams };
		ASSERT(nullptr != indirectParams); // validation layer handles this

		this->UpdateGraphicsVolatileBuffers();

		this->m_ActiveCommandList->CommandList->ExecuteIndirect(this->m_Context.DrawIndexedIndirectSignature, Count, indirectParams->m_Resource, OffsetBytes, nullptr, 0);
	}

	void CommandList::Imp_SetComputeState(const RHIComputeState<D3D12Tag>& State) {
		ComputePipeline* pso{ State.Pipeline };

		const bool updateRootSignature{
			!this->m_CurrentComputeStateValid ||
			this->m_CurrentComputeState.Pipeline == nullptr ||
			this->m_CurrentComputeState.Pipeline->m_RootSignature != pso->m_RootSignature
		};

		bool updatePipeline{
			!this->m_CurrentComputeStateValid ||
			this->m_CurrentComputeState.Pipeline != pso
		};

		bool updateIndirectParams{
			!this->m_CurrentComputeStateValid ||
			this->m_CurrentComputeState.IndirectParams != State.IndirectParams
		};

		Uint32 bindingUpdateMask{ 0 };
		if (!this->m_CurrentComputeStateValid || updateRootSignature)
			bindingUpdateMask = ~0u;

		if (this->CommitDescriptorHeaps())
			bindingUpdateMask = ~0u;

		if (bindingUpdateMask == 0)
			bindingUpdateMask = Array32DifferenceMask(this->m_CurrentComputeState.BindingSets, State.BindingSets);

		if (updateRootSignature)
			this->m_ActiveCommandList->CommandList->SetComputeRootSignature(pso->m_RootSignature->m_RootSignature);

		if (updatePipeline) {
			this->m_ActiveCommandList->CommandList->SetPipelineState(pso->m_PipelineState);

			this->m_Instance->ReferencedResources.push_back(pso);
		}

		this->SetComputeBindings(State.BindingSets, State.BindingSetCount, bindingUpdateMask, State.IndirectParams, updateIndirectParams, pso->m_RootSignature);

		this->UnbindShadingRateState();

		this->m_CurrentGraphicsStateValid = false;
		this->m_CurrentComputeStateValid = true;
		this->m_CurrentMeshletStateValid = false;
		this->m_CurrentComputeState = State;

		this->CommitBarriers();
	}

	inline void CommandList::Imp_Dispatch(Uint32 GroupsX, Uint32 GroupsY, Uint32 GroupsZ) {
		this->UpdateComputeVolatileBuffers();

		this->m_ActiveCommandList->CommandList->Dispatch(GroupsX, GroupsY, GroupsZ);
	}

	inline void CommandList::Imp_DispatchIndirect(Uint32 OffsetBytes) {
		this->UpdateComputeVolatileBuffers();

		this->m_ActiveCommandList->CommandList->ExecuteIndirect(this->m_Context.DispatchIndirectSignature, 1, this->m_CurrentComputeState.IndirectParams->m_Resource.Get(), OffsetBytes, nullptr, 0);
	}

	void CommandList::Imp_SetMeshletState(const RHIMeshletState<D3D12Tag>& State) {
		auto pso{ State.Pipeline };
		auto framebuffer{ State.FrameBuffer };

		this->UnbindShadingRateState();

		const bool updateFramebuffer{
			!this->m_CurrentMeshletStateValid ||
			this->m_CurrentMeshletState.FrameBuffer != framebuffer
		};
		const bool updateRootSignature{
			!this->m_CurrentMeshletStateValid ||
			nullptr == this->m_CurrentMeshletState.Pipeline ||
			this->m_CurrentMeshletState.Pipeline->m_RootSignature != pso->m_RootSignature
		};

		const bool updatePipeline{
			!this->m_CurrentMeshletStateValid ||
			this->m_CurrentMeshletState.Pipeline != pso
		};
		const bool updateIndirectParams{
			!this->m_CurrentMeshletStateValid ||
			this->m_CurrentMeshletState.IndirectParams != State.IndirectParams
		};

		const bool updateViewports{
			!this->m_CurrentMeshletStateValid ||
			this->m_CurrentMeshletState.Viewport != State.Viewport
		};

		const bool updateBlendFactor{
			!this->m_CurrentMeshletStateValid ||
			this->m_CurrentMeshletState.BlendConstantColor != State.BlendConstantColor
		};

		const Uint8 effectiveStencilRefValue{
			pso->m_Desc.RenderState.DepthStencilState.DynamicStencilRef
			? State.DynamicStencilRefValue
			: pso->m_Desc.RenderState.DepthStencilState.StencilRefValue
		};

		const bool updateStencilRef{
			!this->m_CurrentMeshletStateValid ||
			this->m_CurrentMeshletState.DynamicStencilRefValue != effectiveStencilRefValue
		};

		Uint32 bindingUpdateMask{ 0 };
		if (!this->m_CurrentMeshletStateValid || updateRootSignature)
			bindingUpdateMask = ~0u;

		if (this->CommitDescriptorHeaps())
			bindingUpdateMask = ~0u;

		if (0 == bindingUpdateMask)
			bindingUpdateMask = Array32DifferenceMask(this->m_CurrentMeshletState.BindingSets, State.BindingSets);

		if (updatePipeline) {
			this->BindMeshletPipeline(pso, updateRootSignature);
			this->m_Instance->ReferencedResources.push_back(pso);
		}

		if (pso->m_Desc.RenderState.DepthStencilState.StencilEnable && (updatePipeline || updateStencilRef)) {
			this->m_ActiveCommandList->CommandList->OMSetStencilRef(effectiveStencilRefValue);

			if (pso->m_RequiresBlendFactor && updateBlendFactor)
				this->m_ActiveCommandList->CommandList->OMSetBlendFactor(&State.BlendConstantColor.R);

			if (updateFramebuffer) {
				this->BindFramebuffer(framebuffer);
				this->m_Instance->ReferencedResources.push_back(framebuffer);
			}

			this->SetGraphicsBindings(State.BindingSets, State.BindingSetCount, bindingUpdateMask, State.IndirectParams, updateIndirectParams, pso->m_RootSignature);

			this->CommitBarriers();

			if (updateViewports) {
				ViewportState vpState{ ViewportState::Convert(pso->m_Desc.RenderState.RasterState, framebuffer->m_Info, State.Viewport) };

				if (0 != vpState.ViewportCount)
					this->m_ActiveCommandList->CommandList->RSSetViewports(vpState.ViewportCount, vpState.Viewports.data());

				if (0 != vpState.ScissorCount)
					this->m_ActiveCommandList->CommandList->RSSetScissorRects(vpState.ScissorCount, vpState.Scissors.data());
			}

			this->m_CurrentGraphicsStateValid = false;
			this->m_CurrentComputeStateValid = false;
			this->m_CurrentMeshletStateValid = true;
			this->m_CurrentMeshletState = State;
			this->m_CurrentMeshletState.DynamicStencilRefValue = effectiveStencilRefValue;
		}
	}

	inline void CommandList::Imp_DispatchMesh(Uint32 GroupsX, Uint32 GroupsY, Uint32 GroupsZ) {
		this->UpdateGraphicsVolatileBuffers();

		this->m_ActiveCommandList->CommandList6->DispatchMesh(GroupsX, GroupsY, GroupsZ);
	}

	inline void CommandList::Imp_BeginTimerQuery(TimerQuery* query) {
		this->m_Instance->ReferencedTimerQueries.push_back(query);

		this->m_ActiveCommandList->CommandList->EndQuery(m_Context.TimerQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, query->m_BeginQueryIndex);

		// two timestamps within the same command list are always reliably comparable, so we avoid kicking off here
		// (note: we don't call SetStablePowerState anymore)
	}

	inline void CommandList::Imp_EndTimerQuery(TimerQuery* query) {
		this->m_Instance->ReferencedTimerQueries.push_back(query);

		this->m_ActiveCommandList->CommandList->EndQuery(m_Context.TimerQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, query->m_EndQueryIndex);

		this->m_ActiveCommandList->CommandList->ResolveQueryData(
			this->m_Context.TimerQueryHeap,
			D3D12_QUERY_TYPE_TIMESTAMP,
			query->m_BeginQueryIndex,
			2,
			this->m_Context.TimerQueryResolveBuffer->m_Resource.Get(),
			query->m_BeginQueryIndex * 8
		);
	}

	inline void CommandList::Imp_BeginMarker(const char* Name) {
		PIXBeginEvent(this->m_ActiveCommandList->CommandList.Get(), 0, Name);
	}

	inline void CommandList::Imp_EndMarker(void) {
		PIXEndEvent(this->m_ActiveCommandList->CommandList.Get());
	}

	inline void CommandList::Imp_SetEnableAutomaticBarriers(bool enable) {
		this->m_EnableAutomaticBarriers = enable;
	}

	void CommandList::Imp_SetResourceStatesForBindingSet(BindingSet* bindingSet) {
		for (auto bindingIndex : bindingSet->m_BindingsThatNeedTransitions)
		{
			const auto& binding = bindingSet->m_Desc.Bindings[bindingIndex];

			switch (binding.Type) {
				using enum RHIResourceType;
			case Texture_SRV:
				if (nullptr != GetIf<RefCountPtr<Texture>>(&binding.ResourcePtr))
					this->m_StateTracker.RequireTextureState(&Get<RefCountPtr<Texture>>(binding.ResourcePtr)->m_StateExtension, binding.Subresources, RHIResourceState::ShaderResource);
				else
					this->m_StateTracker.RequireTextureState(&Get<RefCountPtr<SamplerFeedbackTexture>>(binding.ResourcePtr)->m_PairdTexture->m_StateExtension, binding.Subresources, RHIResourceState::ShaderResource);//TODO :
				break;

			case Texture_UAV:
				this->m_StateTracker.RequireTextureState(&Get<RefCountPtr<Texture>>(binding.ResourcePtr)->m_StateExtension, binding.Subresources, RHIResourceState::UnorderedAccess);
				break;

			case TypedBuffer_SRV:
			case StructuredBuffer_SRV:
			case RawBuffer_SRV:
				this->m_StateTracker.RequireBufferState(&Get<RefCountPtr<Buffer>>(binding.ResourcePtr)->m_StateExtension, RHIResourceState::ShaderResource);
				break;

			case TypedBuffer_UAV:
			case StructuredBuffer_UAV:
			case RawBuffer_UAV:
				this->m_StateTracker.RequireBufferState(&Get<RefCountPtr<Buffer>>(binding.ResourcePtr)->m_StateExtension, RHIResourceState::UnorderedAccess);
				break;

			case ConstantBuffer:
				this->m_StateTracker.RequireBufferState(&Get<RefCountPtr<Buffer>>(binding.ResourcePtr)->m_StateExtension, RHIResourceState::ConstantBuffer);
				break;

			default:
				// do nothing
				break;
			}
		}
	}

	inline void CommandList::Imp_SetResourceStatesForFramebuffer(FrameBuffer* framebuffer) {
		const auto& desc = framebuffer->Get_Desc();

		for (Uint32 Index = 0; Index < desc.ColorAttachmentCount; ++Index) {
			const auto& attachment{ desc.ColorAttachments[Index] };

			this->SetTextureState(attachment.Texture, attachment.Subresources, RHIResourceState::RenderTarget);
		}

		if (desc.DepthStencilAttachment.Is_Valid())
			this->SetTextureState(
				desc.DepthStencilAttachment.Texture,
				desc.DepthStencilAttachment.Subresources,
				desc.DepthStencilAttachment.IsReadOnly ? RHIResourceState::DepthRead : RHIResourceState::DepthWrite
			);
	}

	inline void CommandList::Imp_SetEnableUAVBarriersForTexture(Texture* texture, bool enable) {
		this->m_StateTracker.SetEnableUavBarriersForTexture(&texture->m_StateExtension, enable);
	}

	inline void CommandList::Imp_SetEnableUAVBarriersForBuffer(Buffer* buffer, bool enable) {
		this->m_StateTracker.SetEnableUavBarriersForBuffer(&buffer->m_StateExtension, enable);
	}

	inline void CommandList::Imp_BeginTrackingTextureState(Texture* texture, RHITextureSubresourceSet subresources, RHIResourceState state) {
		this->m_StateTracker.BeginTrackingTextureState(&texture->m_StateExtension, subresources, state);
	}

	inline void CommandList::Imp_BeginTrackingBufferState(Buffer* buffer, RHIResourceState state) {
		this->m_StateTracker.BeginTrackingBufferState(&buffer->m_StateExtension, state);
	}

	inline void CommandList::Imp_SetTextureState(Texture* texture, RHITextureSubresourceSet subresources, RHIResourceState state) {
		this->m_StateTracker.RequireTextureState(&texture->m_StateExtension, subresources, state);

		if (nullptr != this->m_Instance)
			this->m_Instance->ReferencedResources.push_back(texture);
	}

	inline void CommandList::Imp_SetBufferState(Buffer* buffer, RHIResourceState state) {
		this->m_StateTracker.RequireBufferState(&buffer->m_StateExtension, state);

		if (nullptr != this->m_Instance)
			this->m_Instance->ReferencedResources.push_back(buffer);
	}

	inline void CommandList::Imp_SetPermanentTextureState(Texture* texture, RHIResourceState state) {
		this->m_StateTracker.SetPermanentTextureState(&texture->m_StateExtension, g_AllSubResourceSet, state);

		if (nullptr != this->m_Instance)
			this->m_Instance->ReferencedResources.push_back(texture);
	}

	inline void CommandList::Imp_SetPermanentBufferState(Buffer* buffer, RHIResourceState state) {
		this->m_StateTracker.SetPermanentBufferState(&buffer->m_StateExtension, state);

		if (nullptr != this->m_Instance)
			this->m_Instance->ReferencedResources.push_back(buffer);
	}

	void CommandList::Imp_CommitBarriers(void) {
		const auto& textureBarriers{ this->m_StateTracker.Get_TextureBarriers() };
		const auto& bufferBarriers{ this->m_StateTracker.Get_BufferBarriers() };
		const Uint64 barrierCount{ textureBarriers.size() + bufferBarriers.size() };
		if (0 == barrierCount)
			return;

		// Allocate vector space for the barriers assuming 1:1 translation.
		//maybe more tanh 1...
		this->m_D3DBarriers.clear();
		this->m_D3DBarriers.reserve(barrierCount);

		// Convert the texture barriers into D3D equivalents
		for (const auto& barrier : textureBarriers) {
			const Texture* ParentTexture{ nullptr };
			ID3D12Resource* resource{ nullptr };

			if (barrier.Texture->IsSamplerFeedback)//TODO :Remove bool use getif
				resource = Get<SamplerFeedbackTexture*>(barrier.Texture->ParentTextureRef)->m_Resource.Get();
			else {
				ParentTexture = Get<Texture*>(barrier.Texture->ParentTextureRef);
				resource = ParentTexture->m_Resource.Get();
			}

			D3D12_RESOURCE_BARRIER d3dbarrier{};
			const D3D12_RESOURCE_STATES stateBefore{ ConvertResourceStates(barrier.StateBefore) };
			const D3D12_RESOURCE_STATES stateAfter{ ConvertResourceStates(barrier.StateAfter) };
			if (stateBefore != stateAfter)
			{
				d3dbarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				d3dbarrier.Transition.StateBefore = stateBefore;
				d3dbarrier.Transition.StateAfter = stateAfter;
				d3dbarrier.Transition.pResource = resource;
				if (barrier.EntireTexture) {
					d3dbarrier.Transition.Subresource = g_D3D12ResourceBarrierAllSubresource;
					this->m_D3DBarriers.push_back(d3dbarrier);
				}
				else
					for (Uint8 plane = 0; plane < ParentTexture->m_PlaneCount; ++plane) {
						d3dbarrier.Transition.Subresource = CalcSubresource(barrier.MipLevel, barrier.ArraySlice, plane, ParentTexture->m_Desc.MipLevelCount, ParentTexture->m_Desc.ArrayCount);
						m_D3DBarriers.push_back(d3dbarrier);
					}
			}
			else if (stateAfter & D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
			{
				d3dbarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
				d3dbarrier.UAV.pResource = resource;
				m_D3DBarriers.push_back(d3dbarrier);
			}
		}

		// Convert the buffer barriers into D3D equivalents
		for (const auto& barrier : bufferBarriers) {
			const Buffer* buffer = barrier.Buffer->ParentBuffer;

			D3D12_RESOURCE_BARRIER d3dbarrier{};
			const D3D12_RESOURCE_STATES stateBefore{ ConvertResourceStates(barrier.StateBefore) };
			const D3D12_RESOURCE_STATES stateAfter{ ConvertResourceStates(barrier.StateAfter) };

			//TODO : using enum RHIResourceState; if can do ....
			if (stateBefore != stateAfter &&
				(stateBefore & D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE) == 0 &&
				(stateAfter & D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE) == 0) {
				d3dbarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				d3dbarrier.Transition.StateBefore = stateBefore;
				d3dbarrier.Transition.StateAfter = stateAfter;
				d3dbarrier.Transition.pResource = buffer->m_Resource;
				d3dbarrier.Transition.Subresource = g_D3D12ResourceBarrierAllSubresource;
				m_D3DBarriers.push_back(d3dbarrier);
			}
			else if ((RHIResourceState::AccelStructWrite == barrier.StateBefore && (barrier.StateAfter & (RHIResourceState::AccelStructRead | RHIResourceState::AccelStructBuildBlas)) != RHIResourceState::Unknown) ||
				(RHIResourceState::AccelStructWrite == barrier.StateAfter && (barrier.StateBefore & (RHIResourceState::AccelStructRead | RHIResourceState::AccelStructBuildBlas)) != RHIResourceState::Unknown) ||
				(RHIResourceState::OpacityMicromapWrite == barrier.StateBefore && (barrier.StateAfter & (RHIResourceState::AccelStructBuildInput)) != RHIResourceState::Unknown) ||
				(stateAfter & D3D12_RESOURCE_STATE_UNORDERED_ACCESS) != D3D12_RESOURCE_STATE_COMMON) {
				d3dbarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
				d3dbarrier.UAV.pResource = buffer->m_Resource;
				m_D3DBarriers.push_back(d3dbarrier);
			}
		}//also has a bug ... TODO

		if (this->m_D3DBarriers.size() > 0)
			this->m_ActiveCommandList->CommandList->ResourceBarrier(static_cast<Uint32>(this->m_D3DBarriers.size()), this->m_D3DBarriers.data());

		this->m_StateTracker.ClearBarriers();
	}

	inline RHIResourceState CommandList::Imp_Get_TextureSubresourceState(Texture* texture, Uint32 arraySlice, Uint32 mipLevel) {
		return this->m_StateTracker.Get_TextureSubresourceState(&texture->m_StateExtension, arraySlice, mipLevel);
	}

	inline RHIResourceState CommandList::Imp_Get_BufferState(Buffer* buffer) {
		return this->m_StateTracker.Get_BufferState(&buffer->m_StateExtension);
	}

	inline Device* CommandList::Imp_Get_Device(void) {
		return this->m_Device;
	}

	inline const RHICommandListParameters& CommandList::Imp_Get_Desc(void) {
		return this->m_Desc;
	}

}
