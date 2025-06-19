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
#include "D3D12RHI/Module/DirectX12Wrapper.h"

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Concurrent/Module/Concurrent.h"
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
#include "D3D12RHI/Module/D3D12RHI-CommandList.h"

#endif // PARTING_MODULE_BUILD

namespace RHI::D3D12 {

	class Device final :public RHIDevice<Device, D3D12Tag> {
		friend class RHIResource<Device>;
		friend class RHIDevice<Device, D3D12Tag>;


		friend class CommandList;
		friend class D3D12RootSignature;
		friend class D3D12StaticDescriptorHeap;

	public:
		explicit Device(const D3D12DviceDesc& deviceDesc);

		~Device(void) {
			if (false == this->WaitForIdle())
				LOG_ERROR("Device is not idle");

			if (nullptr != this->m_FenceEvent) {
				::CloseHandle(this->m_FenceEvent);
				this->m_FenceEvent = nullptr;
			}
		}

	public:
		D3D12Queue* Get_Queue(RHICommandQueue type) { return this->m_Queues[Tounderlying(type)].get(); }

		RefCountPtr<D3D12RootSignature> BuildRootSignature(const Span<const RefCountPtr<BindingLayout>>& pipelineLayouts, bool allowInputLayout, bool isLocal, const D3D12_ROOT_PARAMETER1* pCustomParameters = nullptr, Uint32 numCustomParameters = 0);

		RefCountPtr<GraphicsPipeline> CreateHandleForNativeGraphicsPipeline(D3D12RootSignature* rootSignature, ID3D12PipelineState* pipelineState, const RHIGraphicsPipelineDesc<D3D12Tag>& desc, const RHIFrameBufferInfo<D3D12Tag>& framebufferInfo);

		RefCountPtr<MeshletPipeline> CreateHandleForNativeMeshletPipeline(D3D12RootSignature* rootSignature, ID3D12PipelineState* pipelineState, const RHIMeshletPipelineDesc<D3D12Tag>& desc, const RHIFrameBufferInfo<D3D12Tag>& framebufferInfo) {
			return nullptr;
		}
		D3D12StaticDescriptorHeap* Get_DescriptorHeap(DescriptorHeapType heapType) {
			return nullptr;
		}


	private:
		RefCountPtr<D3D12RootSignature> Get_RootSignature(const Span<const RefCountPtr<BindingLayout>>& pipelineLayouts, bool allowInputLayout);


		RefCountPtr<ID3D12PipelineState> CreatePipelineState(const RHIComputePipelineDesc<D3D12Tag>& desc, D3D12RootSignature* pRS) const;
		RefCountPtr<ID3D12PipelineState> CreatePipelineState(const RHIGraphicsPipelineDesc<D3D12Tag>& desc, D3D12RootSignature* pRS, const RHIFrameBufferInfo<D3D12Tag>& fbinfo) const;
		RefCountPtr<ID3D12PipelineState> CreatePipelineState(const RHIMeshletPipelineDesc<D3D12Tag>& desc, D3D12RootSignature* pRS, const RHIFrameBufferInfo<D3D12Tag>& fbinfo) const {
			return nullptr;
		}

	private:
		Context m_Context{};
		D3D12DeviceResources m_Resources;

		Array<UniquePtr<D3D12Queue>, Tounderlying(RHICommandQueue::Count)> m_Queues;
		HANDLE m_FenceEvent;

		Mutex m_Mutex;

		Vector<ID3D12CommandList*> m_CommandListsToExecute; // used locally in executeCommandLists, member to avoid re-allocations

		D3D12_FEATURE_DATA_D3D12_OPTIONS  m_Options{};
		D3D12_FEATURE_DATA_D3D12_OPTIONS7 m_Options7{};

		bool m_MeshletsSupported{ false };

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
		void Imp_UpdateTextureTileMappings(Texture* texture, const RHITextureTilesMapping<D3D12Tag>* TileMappings, Uint32 TileMappingCount, RHICommandQueue executionQueue);
		RefCountPtr<Buffer> Imp_CreateBuffer(const RHIBufferDesc& desc);
		void* Imp_MapBuffer(Buffer* buffer, RHICPUAccessMode CPUaccess);
		void Imp_UnmapBuffer(Buffer* buffer);
		RHIMemoryRequirements Imp_Get_BufferMemoryRequirements(Buffer* buffer);
		bool Imp_BindBufferMemory(Buffer* buffer, Heap* heap, Uint64 offset);
		RefCountPtr<Shader> Imp_CreateShader(const RHIShaderDesc& desc, const void* binary, Uint64 binarySize);
		RefCountPtr<Shader> Imp_CreateShaderSpecialization(Shader* baseshader, const RHIShaderSpecialization* Constants, Uint32 ConstantCount) { LOG_ERROR("Imp But Empty"); return nullptr; }
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
		RefCountPtr<FrameBuffer> Imp_CreateFrameBuffer(const RHIFrameBufferDesc<D3D12Tag>& desc);
		RefCountPtr<GraphicsPipeline> Imp_CreateGraphicsPipeline(const RHIGraphicsPipelineDesc<D3D12Tag>& desc, FrameBuffer* framebuffer);
		RefCountPtr<ComputePipeline> Imp_CreateComputePipeline(const RHIComputePipelineDesc<D3D12Tag>& desc);


		RefCountPtr<BindingLayout> Imp_CreateBindingLayout(const RHIBindingLayoutDesc& desc);
		RefCountPtr<BindingSet> Imp_CreateBindingSet(const RHIBindingSetDesc<D3D12Tag>& desc, BindingLayout* layout);


		RefCountPtr<CommandList> Imp_CreateCommandList(const RHICommandListParameters& desc);
		Uint64 Imp_ExecuteCommandLists(CommandList* const* commandLists, Uint32 commandListCount, RHICommandQueue queue);

		bool Imp_WaitForIdle(void);
		void Imp_RunGarbageCollection(void);
		bool Imp_QueryFeatureSupport(RHIFeature feature, void* outData, Uint64 outDataSize);
		RHIFormatSupport Imp_QueryFormatSupport(RHIFormat format);
	};

	Device::Device(const D3D12DviceDesc& desc) :
		RHIDevice<Device, D3D12Tag>{},
		m_Resources{
			.Context{ this->m_Context },
			.RenderTargetViewHeap{ this->m_Context },
			.DepthStencilViewHeap{ this->m_Context },
			.ShaderResourceViewHeap{ this->m_Context },
			.SamplerHeap {this->m_Context },
			.TimerQueries { desc.MaxTimerQueries , true }
		}{

		this->m_Context.Device = desc.Device;

		if (nullptr != desc.GraphicsQueue)
			this->m_Queues[Tounderlying(RHICommandQueue::Graphics)] = MakeUnique<decltype(this->m_Queues)::value_type::element_type>(m_Context, desc.GraphicsQueue);
		if (nullptr != desc.ComputeQueue)
			this->m_Queues[Tounderlying(RHICommandQueue::Compute)] = MakeUnique<decltype(this->m_Queues)::value_type::element_type>(m_Context, desc.ComputeQueue);
		if (desc.CopyQueue)
			this->m_Queues[Tounderlying(RHICommandQueue::Copy)] = MakeUnique<decltype(this->m_Queues)::value_type::element_type>(m_Context, desc.CopyQueue);

		this->m_Resources.RenderTargetViewHeap.D3D12AllocateResources(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, desc.RenderTargetViewHeapSize, false);
		this->m_Resources.DepthStencilViewHeap.D3D12AllocateResources(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, desc.DepthStencilViewHeapSize, false);
		this->m_Resources.ShaderResourceViewHeap.D3D12AllocateResources(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, desc.ShaderResourceViewHeapSize, true);
		this->m_Resources.SamplerHeap.D3D12AllocateResources(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, desc.SamplerHeapSize, true);

		this->m_Context.Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &this->m_Options, sizeof(this->m_Options));
		bool hasOptions7{ HRusltSucccess == m_Context.Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &this->m_Options7, sizeof(decltype(this->m_Options7))) };

		if (hasOptions7 && HRusltSucccess == this->m_Context.Device->QueryInterface(&this->m_Context.Device2))
			this->m_MeshletsSupported = this->m_Options7.MeshShaderTier >= D3D12_MESH_SHADER_TIER_1;

		{
			D3D12_INDIRECT_ARGUMENT_DESC argDesc{};
			D3D12_COMMAND_SIGNATURE_DESC csDesc{ .NumArgumentDescs{ 1 },.pArgumentDescs{ &argDesc } };

			csDesc.ByteStride = 16;
			argDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;
			D3D12_CHECK(this->m_Context.Device->CreateCommandSignature(&csDesc, nullptr, PARTING_IID_PPV_ARGS(&this->m_Context.DrawIndirectSignature)));

			csDesc.ByteStride = 20;
			argDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
			D3D12_CHECK(this->m_Context.Device->CreateCommandSignature(&csDesc, nullptr, PARTING_IID_PPV_ARGS(&this->m_Context.DrawIndexedIndirectSignature)));

			csDesc.ByteStride = 12;
			argDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
			D3D12_CHECK(this->m_Context.Device->CreateCommandSignature(&csDesc, nullptr, PARTING_IID_PPV_ARGS(&this->m_Context.DispatchIndirectSignature)));
		}

		this->m_FenceEvent = CreateEventW(nullptr, false, false, nullptr);

		this->m_CommandListsToExecute.reserve(64);
	}


	//Src

	RefCountPtr<D3D12RootSignature> Device::BuildRootSignature(const Span<const RefCountPtr<BindingLayout>>& pipelineLayouts, bool allowInputLayout, bool isLocal, const D3D12_ROOT_PARAMETER1* pCustomParameters, Uint32 numCustomParameters) {
		D3D12RootSignature* rootsig{ new D3D12RootSignature{ this->m_Resources } };

		// Assemble the root parameter table from the pipeline binding layouts
		// Also attach the root parameter offsets to the pipeline layouts

		Vector<D3D12_ROOT_PARAMETER1> rootParameters;

		// Add custom parameters in the beginning of the RS
		for (Uint32 index = 0; index < numCustomParameters; ++index)
			rootParameters.push_back(pCustomParameters[index]);

		for (const auto& layout : pipelineLayouts) {
			auto rootParameterOffset{ static_cast<D3D12RootSignature::D3D12RootParameterIndex>(rootParameters.size()) };

			rootsig->m_BindLayouts[rootsig->m_BindLayoutCount++] = MakePair(layout, rootParameterOffset);

			rootParameters.insert(rootParameters.end(), layout->m_RootParameters.begin(), layout->m_RootParameters.begin() + layout->m_RootParameterCount);

			if (layout->m_PushConstantByteSize > 0) {
				rootsig->m_PushConstantByteSize = layout->m_PushConstantByteSize;
				rootsig->m_RootParameterPushConstants = layout->m_RootParameterPushConstants + rootParameterOffset;
			}
		}

		// Build the description structure
		D3D12_VERSIONED_ROOT_SIGNATURE_DESC rsDesc{};
		rsDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;//TODO : Support

		if (allowInputLayout)
			rsDesc.Desc_1_1.Flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		if (isLocal)
			rsDesc.Desc_1_1.Flags |= D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;

		if (!rootParameters.empty()) {
			rsDesc.Desc_1_1.pParameters = rootParameters.data();
			rsDesc.Desc_1_1.NumParameters = static_cast<Uint32>(rootParameters.size());
		}

		// Serialize the root signature

		RefCountPtr<ID3DBlob> rsBlob;
		D3D12_CHECK(D3D12SerializeVersionedRootSignature(&rsDesc, &rsBlob, nullptr));

		D3D12_CHECK(this->m_Context.Device->CreateRootSignature(0, rsBlob->GetBufferPointer(), rsBlob->GetBufferSize(), PARTING_IID_PPV_ARGS(&rootsig->m_RootSignature)));

		return RefCountPtr<D3D12RootSignature>::Create(rootsig);
	}

	RefCountPtr<D3D12RootSignature> Device::Get_RootSignature(const Span<const RefCountPtr<BindingLayout>>& pipelineLayouts, bool allowInputLayout) {
		Uint64 hash{ 0 };

		for (const auto& layout : pipelineLayouts)
			hash = HashCombine(hash, reinterpret_cast<Uint64>(layout.Get()));

		hash = HashCombine(hash, allowInputLayout ? static_cast<Uint64>(1) : static_cast<Uint64>(0));

		RefCountPtr<D3D12RootSignature> RootSignature{ this->m_Resources.RootSignatureCahe[hash] };//NOTE :must use temp Ref to write to map
		if (nullptr == RootSignature) {
			RootSignature = this->BuildRootSignature(pipelineLayouts, allowInputLayout, false);

			ASSERT(nullptr != RootSignature);
			RootSignature->m_Hash = hash;

			this->m_Resources.RootSignatureCahe[hash] = RootSignature;
		}

		return RootSignature;
	}

	RefCountPtr<ID3D12PipelineState> Device::CreatePipelineState(const RHIComputePipelineDesc<D3D12Tag>& desc, D3D12RootSignature* pRS) const {
		RefCountPtr<ID3D12PipelineState> Re;

		D3D12_COMPUTE_PIPELINE_STATE_DESC d3d12desc{
			.pRootSignature{ pRS->m_RootSignature },
			.CS{.pShaderBytecode{ desc.CS->m_Bytecode.data() },	.BytecodeLength{ desc.CS->m_Bytecode.size() } }
		};

		D3D12_CHECK(this->m_Context.Device->CreateComputePipelineState(&d3d12desc, PARTING_IID_PPV_ARGS(&Re)));
		return Re;
	}

	RefCountPtr<ID3D12PipelineState> Device::CreatePipelineState(const RHIGraphicsPipelineDesc<D3D12Tag>& state, D3D12RootSignature* pRS, const RHIFrameBufferInfo<D3D12Tag>& fbinfo) const {
		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{
			.pRootSignature{ pRS->m_RootSignature },
			.BlendState{ TranslateBlendState(state.RenderState.BlendState) },
			.SampleMask{ ~0u },
			.RasterizerState{ TranslateRasterizerState(state.RenderState.RasterState) },
			.DepthStencilState{ TranslateDepthStencilState(state.RenderState.DepthStencilState) },
			.PrimitiveTopologyType { ConvertPrimitiveType(state.PrimType) },
			.NumRenderTargets{ static_cast<Uint32>(fbinfo.ColorFormatCount) },
			.DSVFormat { Get_DXGIFormatMapping(fbinfo.DepthFormat).RTVFormat },
			.SampleDesc{.Count{ fbinfo.SampleCount }, .Quality{ fbinfo.SampleQuality } },
		};

		ASSERT(((desc.DepthStencilState.DepthEnable || desc.DepthStencilState.StencilEnable) && fbinfo.DepthFormat != RHIFormat::UNKNOWN) || (!desc.DepthStencilState.DepthEnable && !desc.DepthStencilState.StencilEnable));

		if (nullptr != state.VS.Get())
			desc.VS = { state.VS->m_Bytecode.data(), state.VS->m_Bytecode.size() };
		if (nullptr != state.PS.Get())
			desc.PS = { state.PS->m_Bytecode.data(), state.PS->m_Bytecode.size() };
		if (nullptr != state.DS.Get())
			desc.DS = { state.DS->m_Bytecode.data(), state.DS->m_Bytecode.size() };
		if (nullptr != state.HS.Get())
			desc.HS = { state.HS->m_Bytecode.data(), state.HS->m_Bytecode.size() };
		if (nullptr != state.GS.Get())
			desc.GS = { state.GS->m_Bytecode.data(), state.GS->m_Bytecode.size() };

		for (Uint32 Index = 0; Index < fbinfo.ColorFormatCount; ++Index)
			desc.RTVFormats[Index] = Get_DXGIFormatMapping(fbinfo.ColorFormats[Index]).RTVFormat;

		if (auto inputLayout = state.InputLayout.Get(); nullptr != inputLayout && !inputLayout->m_InputElements.empty()) {
			desc.InputLayout.NumElements = static_cast<Uint32>(inputLayout->m_InputElements.size());
			desc.InputLayout.pInputElementDescs = inputLayout->m_InputElements.data();
		}

		RefCountPtr<ID3D12PipelineState> pipelineState;
		D3D12_CHECK(this->m_Context.Device->CreateGraphicsPipelineState(&desc, PARTING_IID_PPV_ARGS(&pipelineState)));

		static Uint32 Index{ 0 };
		pipelineState->SetName((WString{ _W("Pipeline") } + std::to_wstring(Index)).c_str());

		return pipelineState;
	}

	RefCountPtr<GraphicsPipeline> Device::CreateHandleForNativeGraphicsPipeline(D3D12RootSignature* rootSignature, ID3D12PipelineState* pipelineState, const RHIGraphicsPipelineDesc<D3D12Tag>& desc, const RHIFrameBufferInfo<D3D12Tag>& framebufferInfo) {
		if (nullptr == rootSignature)
			return nullptr;
		if (nullptr == pipelineState)
			return nullptr;

		GraphicsPipeline* pso = new GraphicsPipeline();
		pso->m_Desc = desc;
		pso->m_FrameBufferInfo = framebufferInfo;
		pso->m_RootSignature = rootSignature;
		pso->m_PipelineState = pipelineState;
		pso->m_RequiresBlendFactor = desc.RenderState.BlendState.Is_UsesConstantColor(pso->m_FrameBufferInfo.ColorFormatCount);

		return RefCountPtr<GraphicsPipeline>::Create(pso);
	}

	//Imp


	RHIObject Device::Imp_GetNativeObject(RHIObjectType type)const noexcept {
		switch (type) {
			using enum RHIObjectType;
		case D3D12_Device: return RHIObject{ .Pointer{ this->m_Context.Device} };
		case D3D12_CommandQueue: return RHIObject{ .Pointer{ this->m_Queues[Tounderlying(RHICommandQueue::Graphics)]->m_Queue } };
		default:LOG_ERROR("Imp But Empty"); return RHIObject{};
		}
	}

	RefCountPtr<Heap> Device::Imp_CreateHeap(const RHIHeapDesc& desc) {
		D3D12_HEAP_DESC heapDesc{
			.SizeInBytes { desc.Size },
			.Properties {
				.Type { D3D12_HEAP_TYPE_DEFAULT },
				.CPUPageProperty { D3D12_CPU_PAGE_PROPERTY_UNKNOWN },
				.MemoryPoolPreference { D3D12_MEMORY_POOL_UNKNOWN },
				.CreationNodeMask { 1 },
				.VisibleNodeMask { 1 }
			},
			.Alignment{ g_D3D12DefaultMassResourcePlacmentAlignment },
			.Flags { D3D12_RESOURCE_HEAP_TIER_1 == this->m_Options.ResourceHeapTier ? D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES : D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES }
		};

		switch (desc.Type) {
			using enum RHIHeapType;
		case DeviceLocal:heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT; break;
		case Upload:heapDesc.Properties.Type = D3D12_HEAP_TYPE_UPLOAD; break;
		case Readback:heapDesc.Properties.Type = D3D12_HEAP_TYPE_READBACK; break;
		default:ASSERT(false); return nullptr;
		}

		RefCountPtr<ID3D12Heap> d3dHeap;
		this->m_Context.Device->CreateHeap(&heapDesc, PARTING_IID_PPV_ARGS(&d3dHeap));

		Heap* heap = new Heap{};
		heap->m_Desc = desc;
		heap->m_Heap = d3dHeap;
		return RefCountPtr<Heap> ::Create(heap);
	}

	RefCountPtr<Texture> Device::Imp_CreateTexture(const RHITextureDesc& d) {
		auto rd{ Texture::ConvertTextureDesc(d) };
		D3D12_HEAP_PROPERTIES heapProps{};
		D3D12_HEAP_FLAGS heapFlags{ D3D12_HEAP_FLAG_NONE };

		if (d.IsTiled)//TODO : ?
			rd.Layout = D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE;

		Texture* texture{ new Texture(this->m_Context, this->m_Resources, d, rd) };

		auto clearValue{ d.ClearValue.has_value() ? Texture::ConvertTextureClearValue(d) : D3D12_CLEAR_VALUE{} };

		if (d.IsVirtual)
			return RefCountPtr<Texture>::Create(texture);// The resource is created in BindTextureMemory

		if (d.IsTiled) {
			D3D12_CHECK(this->m_Context.Device->CreateReservedResource(
				&texture->m_ResourceDesc,
				ConvertResourceStates(d.InitialState),
				d.ClearValue.has_value() ? &clearValue : nullptr,
				PARTING_IID_PPV_ARGS(&texture->m_Resource)
			));
		}
		else {
			heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

			D3D12_CHECK(this->m_Context.Device->CreateCommittedResource(
				&heapProps,
				heapFlags,
				&texture->m_ResourceDesc,
				ConvertResourceStates(d.InitialState),
				d.ClearValue.has_value() ? &clearValue : nullptr,
				PARTING_IID_PPV_ARGS(&texture->m_Resource)
			));
		}

		texture->PostCreate();

		return RefCountPtr<Texture>::Create(texture);
	}

	RHIMemoryRequirements Device::Imp_Get_TextureMemoryRequirements(Texture* texture) {
		D3D12_RESOURCE_ALLOCATION_INFO allocInfo{ m_Context.Device->GetResourceAllocationInfo(1, 1, &texture->m_ResourceDesc) };

		return RHIMemoryRequirements{
			.Size{ allocInfo.SizeInBytes },
			.Alignment{ allocInfo.Alignment },
		};
	}

	bool Device::Imp_BindTextureMemory(Texture* texture, Heap* heap, Uint64 offset) {
		if (nullptr != texture->m_Resource)
			return false; // already bound

		if (!texture->m_Desc.IsVirtual)
			return false; // not supported


		auto clearValue{ texture->m_Desc.ClearValue.has_value() ? Texture::ConvertTextureClearValue(texture->m_Desc) : D3D12_CLEAR_VALUE{} };

		this->m_Context.Device->CreatePlacedResource(
			heap->m_Heap.Get(),
			offset,
			&texture->m_ResourceDesc,
			ConvertResourceStates(texture->m_Desc.InitialState),
			texture->m_Desc.ClearValue.has_value() ? &clearValue : nullptr,
			PARTING_IID_PPV_ARGS(&texture->m_Resource)
		);

		texture->m_Heap = heap;
		texture->PostCreate();

		return true;
	}

	RefCountPtr<Texture> Device::Imp_CreateHandleForNativeTexture(RHIObjectType type, RHIObject object, const RHITextureDesc& desc) {
		ASSERT(nullptr != object.Pointer);
		ASSERT(RHIObjectType::D3D12_Resource == type);

		auto pResource{ static_cast<ID3D12Resource*>(object.Pointer) };

		Texture* texture{ new Texture(this->m_Context, this->m_Resources, desc, pResource->GetDesc()) };
		ASSERT(nullptr != texture);

		texture->m_Resource = ::MoveTemp(pResource);
		texture->PostCreate();

		return RefCountPtr<Texture>::Create(texture);
	}

	RefCountPtr<StagingTexture> Device::Imp_CreateStagingTexture(const RHITextureDesc& d, RHICPUAccessMode CPUaccess) {
		ASSERT(RHICPUAccessMode::None != CPUaccess);

		StagingTexture* ret = new StagingTexture();
		ret->m_Desc = d;
		ret->m_ResourceDesc = Texture::ConvertTextureDesc(d);
		ret->ComputeSubresourceOffsets(this->m_Context.Device);

		RHIBufferDesc bufferDesc{
			.ByteSize { ret->Get_SizeInBytes(this->m_Context.Device) },
			.StructStride { 0 },
		};
		bufferDesc.CPUAccess = CPUaccess;

		RefCountPtr<Buffer> buffer = CreateBuffer(bufferDesc);
		ret->m_Buffer = buffer;
		ret->m_CPUAccessMode = CPUaccess;
		return RefCountPtr<StagingTexture>::Create(ret);
	}

	void* Device::Imp_MapStagingTexture(StagingTexture* tex, const RHITextureSlice& slice, RHICPUAccessMode CPUAccess, Uint64* OutRowPitch) {
		ASSERT(0 == slice.Extent.Width);
		ASSERT(0 == slice.Extent.Height);
		ASSERT(RHICPUAccessMode::None != CPUAccess);
		ASSERT(0 == tex->m_MappedRegion.Size);
		ASSERT(RHICPUAccessMode::None == tex->m_MappedRegionAccessMode);

		auto resolvedSlice{ slice.Resolve(tex->m_Desc) };
		auto region{ tex->Get_SliceRegion(m_Context.Device, resolvedSlice) };

		if (tex->m_LastUseFence) {
			D3D12WaitForFence(tex->m_LastUseFence.Get(), tex->m_LastUseFenceValue, this->m_FenceEvent);
			tex->m_LastUseFence = nullptr;
		}

		D3D12_RANGE range{};
		if (RHICPUAccessMode::Read == CPUAccess)
			range = D3D12_RANGE{ .Begin { static_cast<Uint64>(region.Offset) }, .End { region.Offset + region.Size } };
		else
			range = D3D12_RANGE{ .Begin { 0 }, .End { 0 } };

		Uint8* ret{};
		tex->m_Buffer->m_Resource->Map(0, &range, reinterpret_cast<void**>(&ret));

		tex->m_MappedRegion = region;
		tex->m_MappedRegionAccessMode = CPUAccess;

		*OutRowPitch = region.Footprint.Footprint.RowPitch;
		return ret + tex->m_MappedRegion.Offset;
	}

	void Device::Imp_UnmapStagingTexture(StagingTexture* tex) {

		ASSERT(0 != tex->m_MappedRegion.Size);
		ASSERT(RHICPUAccessMode::None != tex->m_MappedRegionAccessMode);

		D3D12_RANGE range{};
		if (RHICPUAccessMode::Write == tex->m_CPUAccessMode)
			range = D3D12_RANGE{ .Begin { static_cast<Uint64>(tex->m_MappedRegion.Offset) }, .End { tex->m_MappedRegion.Offset + tex->m_MappedRegion.Size } };
		else
			range = D3D12_RANGE{ .Begin { 0 }, .End { 0 } };

		tex->m_Buffer->m_Resource->Unmap(0, &range);

		tex->m_MappedRegion.Size = 0;
		tex->m_MappedRegionAccessMode = RHICPUAccessMode::None;
	}

	void Device::Imp_Get_TextureTiling(Texture* texture, Uint32* numTiles, RHIPackedMipDesc* desc, RHITileShape* tileShape, Uint32* subresourceTilingsNum, RHISubresourceTiling* _subresourceTilings) {
		ID3D12Resource* resource{ texture->m_Resource };
		D3D12_RESOURCE_DESC resourceDesc{ resource->GetDesc() };

		D3D12_PACKED_MIP_INFO packedMipDesc{};
		D3D12_TILE_SHAPE standardTileShapeForNonPackedMips{};
		D3D12_SUBRESOURCE_TILING subresourceTilings[16]{};

		this->m_Context.Device->GetResourceTiling(resource, numTiles, desc ? &packedMipDesc : nullptr, tileShape ? &standardTileShapeForNonPackedMips : nullptr, subresourceTilingsNum, 0, subresourceTilings);

		if (desc) {
			desc->StandardMipCount = packedMipDesc.NumStandardMips;
			desc->PackedMipCount = packedMipDesc.NumPackedMips;
			desc->StartTileIndexInOverallResource = packedMipDesc.StartTileIndexInOverallResource;
			desc->TilesForPackedMipCount = packedMipDesc.NumTilesForPackedMips;
		}

		if (tileShape) {
			tileShape->WidthInTexels = standardTileShapeForNonPackedMips.WidthInTexels;
			tileShape->HeightInTexels = standardTileShapeForNonPackedMips.HeightInTexels;
			tileShape->DepthInTexels = standardTileShapeForNonPackedMips.DepthInTexels;
		}

		for (Uint32 Index = 0; Index < *subresourceTilingsNum; ++Index) {
			_subresourceTilings[Index].WidthInTiles = subresourceTilings[Index].WidthInTiles;
			_subresourceTilings[Index].HeightInTiles = subresourceTilings[Index].HeightInTiles;
			_subresourceTilings[Index].DepthInTiles = subresourceTilings[Index].DepthInTiles;
			_subresourceTilings[Index].StartTileIndexInOverallResource = subresourceTilings[Index].StartTileIndexInOverallResource;
		}
	}

	void Device::Imp_UpdateTextureTileMappings(Texture* texture, const RHITextureTilesMapping<D3D12Tag>* tileMappings, Uint32 numTileMappings, RHICommandQueue executionQueue) {
		D3D12Queue* queue = Get_Queue(executionQueue);

		D3D12_TILE_SHAPE tileShape;
		D3D12_SUBRESOURCE_TILING subresourceTiling;
		m_Context.Device->GetResourceTiling(texture->m_Resource, nullptr, nullptr, &tileShape, nullptr, 0, &subresourceTiling);

		for (Uint32 i = 0; i < numTileMappings; i++) {
			ID3D12Heap* heap{ tileMappings[i].Heap->m_Heap };

			auto numRegions{ tileMappings[i].TextureRegionCount };
			Vector<D3D12_TILED_RESOURCE_COORDINATE> resourceCoordinates(numRegions);
			Vector<D3D12_TILE_REGION_SIZE> regionSizes(numRegions);
			Vector<D3D12_TILE_RANGE_FLAGS> rangeFlags{ numRegions, heap ? D3D12_TILE_RANGE_FLAG_NONE : D3D12_TILE_RANGE_FLAG_NULL };
			Vector<Uint32> heapStartOffsets(numRegions);
			Vector<Uint32> rangeTileCounts(numRegions);

			for (Uint32 j = 0; j < numRegions; ++j) {
				const auto& tiledTextureCoordinate = tileMappings[i].TiledTextureCoordinates[j];
				const auto& tiledTextureRegion = tileMappings[i].TiledTextureRegions[j];

				resourceCoordinates[j].Subresource = tiledTextureCoordinate.MipLevel * texture->m_Desc.ArrayCount + tiledTextureCoordinate.ArrayLevel;
				resourceCoordinates[j].X = tiledTextureCoordinate.TileCoordinate.Width;
				resourceCoordinates[j].Y = tiledTextureCoordinate.TileCoordinate.Height;
				resourceCoordinates[j].Z = tiledTextureCoordinate.TileCoordinate.Depth;

				if (tiledTextureRegion.TilesCount) {
					regionSizes[j].NumTiles = tiledTextureRegion.TilesCount;
					regionSizes[j].UseBox = false;
				}
				else {
					Uint32 tilesX = (tiledTextureRegion.TileSize.Width + (tileShape.WidthInTexels - 1)) / tileShape.WidthInTexels;
					Uint32 tilesY = (tiledTextureRegion.TileSize.Height + (tileShape.HeightInTexels - 1)) / tileShape.HeightInTexels;
					Uint32 tilesZ = (tiledTextureRegion.TileSize.Depth + (tileShape.DepthInTexels - 1)) / tileShape.DepthInTexels;

					regionSizes[j].Width = tilesX;
					regionSizes[j].Height = static_cast<Uint16>(tilesY);
					regionSizes[j].Depth = static_cast<Uint16>(tilesZ);

					regionSizes[j].NumTiles = tilesX * tilesY * tilesZ;
					regionSizes[j].UseBox = true;
				}

				// Offset in tiles
				if (heap)
					heapStartOffsets[j] = static_cast<Uint32>(tileMappings[i].ByteOffsets[j] / g_D3D12TildResourceTileSizeBytes);

				rangeTileCounts[j] = regionSizes[j].NumTiles;
			}

			queue->m_Queue->UpdateTileMappings(
				texture->m_Resource,
				tileMappings[i].TextureRegionCount, resourceCoordinates.data(), regionSizes.data(),
				heap,
				numRegions, rangeFlags.data(),
				heap ? heapStartOffsets.data() : nullptr,
				rangeTileCounts.data(),
				D3D12_TILE_MAPPING_FLAG_NONE
			);
		}
	}

	RefCountPtr<Buffer> Device::Imp_CreateBuffer(const RHIBufferDesc& d) {
		auto desc{ d };
		if (desc.IsConstantBuffer)
			desc.ByteSize = Math::Align(d.ByteSize, static_cast<Uint64>(g_D3D12ConstantBufferDataPlacementAlignment));

		Buffer* buffer{ new Buffer(this->m_Context, this->m_Resources, desc) };

		if (d.IsVolatile)
			return RefCountPtr<Buffer>::Create(buffer);//if vo , create a empty buffer

		D3D12_RESOURCE_DESC& resourceDesc{ buffer->m_ResourceDesc };
		resourceDesc.Width = buffer->m_Desc.ByteSize;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		/*resourceDesc.SampleDesc.Quality = 0;*/
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		if (buffer->m_Desc.CanHaveUAVs)
			resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		if (d.IsVirtual)
			return RefCountPtr<Buffer>::Create(buffer);

		D3D12_HEAP_PROPERTIES heapProps{};
		D3D12_HEAP_FLAGS heapFlags{ D3D12_HEAP_FLAG_NONE };
		D3D12_RESOURCE_STATES initialState{ D3D12_RESOURCE_STATE_COMMON };

		bool isShared{ false };
		if (RHISharedResourceFlag::None != (d.sharedResourceFlags & RHISharedResourceFlag::Shared)) {
			heapFlags |= D3D12_HEAP_FLAG_SHARED;
			isShared = true;
		}
		if (RHISharedResourceFlag::None != (d.sharedResourceFlags & RHISharedResourceFlag::Shared_CrossAdapter)) {
			resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER;
			heapFlags |= D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER;
			isShared = true;
		}

		switch (buffer->m_Desc.CPUAccess) {
			using enum RHICPUAccessMode;
		case None:
			heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
			initialState = D3D12_RESOURCE_STATE_COMMON;
			break;

		case Read:
			heapProps.Type = D3D12_HEAP_TYPE_READBACK;
			initialState = D3D12_RESOURCE_STATE_COPY_DEST;
			break;

		case Write:
			heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
			initialState = D3D12_RESOURCE_STATE_GENERIC_READ;
			break;
		}

		// Allow readback buffers to be used as resolve destination targets
		if ((buffer->m_Desc.CPUAccess == RHICPUAccessMode::Read) && (d.InitialState == RHIResourceState::ResolveDest)) {
			heapProps.Type = D3D12_HEAP_TYPE_CUSTOM;
			heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
			heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
			initialState = D3D12_RESOURCE_STATE_COMMON;
		}

		D3D12_CHECK(this->m_Context.Device->CreateCommittedResource(
			&heapProps,
			heapFlags,
			&resourceDesc,
			initialState,
			nullptr,
			PARTING_IID_PPV_ARGS(&buffer->m_Resource)
		));

		if (isShared)
			D3D12_CHECK(this->m_Context.Device->CreateSharedHandle(
				buffer->m_Resource,
				nullptr,
				0x10000000L /*GENERIC_ALL*/,
				nullptr,
				&buffer->m_SharedHandle
			));

		buffer->PostCreate();

		return RefCountPtr<Buffer>::Create(buffer);
	}

	void* Device::Imp_MapBuffer(Buffer* buffer, RHICPUAccessMode CPUaccess) {
		if (buffer->m_LastUseFence) {
			D3D12WaitForFence(buffer->m_LastUseFence, buffer->m_LastUseFenceValue, this->m_FenceEvent);
			buffer->m_LastUseFence = nullptr;
		}

		D3D12_RANGE range;

		if (RHICPUAccessMode::Read == CPUaccess)
			range = D3D12_RANGE{ .Begin { 0 }, .End {buffer->m_Desc.ByteSize } };
		else
			range = D3D12_RANGE{ .Begin { 0 }, .End { 0 } };

		void* mappedBuffer;
		D3D12_CHECK(buffer->m_Resource->Map(0, &range, &mappedBuffer));

		return mappedBuffer;
	}

	void Device::Imp_UnmapBuffer(Buffer* buffer) {
		buffer->m_Resource->Unmap(0, nullptr);
	}

	RHIMemoryRequirements Device::Imp_Get_BufferMemoryRequirements(Buffer* buffer) {
		D3D12_RESOURCE_ALLOCATION_INFO allocInfo{ m_Context.Device->GetResourceAllocationInfo(1, 1, &buffer->m_ResourceDesc) };
		return RHIMemoryRequirements{
			.Size{ allocInfo.SizeInBytes },
			.Alignment{ allocInfo.Alignment },
		};
	}

	bool Device::Imp_BindBufferMemory(Buffer* buffer, Heap* heap, Uint64 offset) {
		if (nullptr != buffer->m_Resource)
			return false; // already bound

		if (!buffer->m_Desc.IsVirtual)
			return false; // not supported

		auto initialState{ ConvertResourceStates(buffer->m_Desc.InitialState) };
		if (D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE != initialState)
			initialState = D3D12_RESOURCE_STATE_COMMON;

		this->m_Context.Device->CreatePlacedResource(
			heap->m_Heap, offset,
			&buffer->m_ResourceDesc,
			initialState,
			nullptr,
			PARTING_IID_PPV_ARGS(&buffer->m_Resource));

		buffer->m_Heap = heap;
		buffer->PostCreate();

		return true;
	}

	RefCountPtr<Shader> Device::Imp_CreateShader(const RHIShaderDesc& desc, const void* binary, Uint64 binarySize) {
		ASSERT(0 != binarySize);
		ASSERT(nullptr != binary);

		Shader* shader{ new Shader{ desc } };
		shader->m_Bytecode.resize(binarySize);
		shader->m_Desc = desc;
		memcpy(shader->m_Bytecode.data(), binary, binarySize);

		return RefCountPtr<Shader>::Create(shader);
	}

	RefCountPtr<Sampler> Device::Imp_CreateSampler(const RHISamplerDesc& desc) {
		return RefCountPtr<Sampler>::Create(new Sampler{ this->m_Context, desc });
	}

	RefCountPtr<InputLayout> Device::Imp_CreateInputLayout(const RHIVertexAttributeDesc* attributes, Uint32 attributeCount) {
		auto layout{ new InputLayout{} };
		layout->m_Attributes.resize(attributeCount);

		for (Uint32 index = 0; index < attributeCount; ++index) {//TODO :Span....
			auto& attr{ layout->m_Attributes[index] };

			// Copy the description to get a stable name pointer in desc
			attr = attributes[index];

			ASSERT(attr.ArrayCount > 0);

			const auto& formatMapping{ Get_DXGIFormatMapping(attr.Format) };
			const auto& formatInfo{ Get_RHIFormatInfo(attr.Format) };

			for (Uint32 semanticIndex = 0; semanticIndex < attr.ArrayCount; ++semanticIndex) {
				D3D12_INPUT_ELEMENT_DESC desc{};

				desc.SemanticName = attr.Name.c_str();
				desc.AlignedByteOffset = attr.Offset + semanticIndex * formatInfo.BytesPerBlock;
				desc.Format = formatMapping.SRVFormat;
				desc.InputSlot = attr.BufferIndex;
				desc.SemanticIndex = semanticIndex;

				if (attr.IsInstanced) {
					desc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
					desc.InstanceDataStepRate = 1;
				}
				else {
					desc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
					desc.InstanceDataStepRate = 0;
				}

				layout->m_InputElements.push_back(desc);
			}

			if (layout->m_ElementStrides.find(attr.BufferIndex) == layout->m_ElementStrides.end())
				layout->m_ElementStrides[attr.BufferIndex] = attr.ElementStride;
			else
				ASSERT(layout->m_ElementStrides[attr.BufferIndex] == attr.ElementStride);
		}

		return RefCountPtr<InputLayout>::Create(layout);
	}

	RefCountPtr<EventQuery> Device::Imp_CreateEventQuery(void) {
		return RefCountPtr<EventQuery>::Create(new EventQuery{});
	}

	void Device::Imp_SetEventQuery(EventQuery* query, RHICommandQueue queue) {
		D3D12Queue* pQueue{ this->Get_Queue(queue) };

		query->m_Started = true;
		query->m_Fence = pQueue->m_Fence;
		query->m_FenceValue = pQueue->m_LastSubmittedInstance;
		query->m_Resolved = false;
	}

	bool Device::Imp_PollEventQuery(EventQuery* query) {
		if (!query->m_Started)
			return false;

		if (query->m_Resolved)
			return true;

		ASSERT(nullptr != query->m_Fence);

		if (query->m_Fence->GetCompletedValue() >= query->m_FenceValue) {
			query->m_Resolved = true;
			query->m_Fence = nullptr;
		}

		return query->m_Resolved;
	}

	void Device::Imp_WaitEventQuery(EventQuery* query) {
		if (!query->m_Started || query->m_Resolved)
			return;

		ASSERT(nullptr != query->m_Fence);

		D3D12WaitForFence(query->m_Fence, query->m_FenceValue, this->m_FenceEvent);
	}

	void Device::Imp_ResetEventQuery(EventQuery* query) {
		query->m_Started = false;
		query->m_Resolved = false;
		query->m_Fence = nullptr;
	}

	RefCountPtr<TimerQuery> Device::Imp_CreateTimerQuery(void) {
		if (nullptr == this->m_Context.TimerQueryHeap) {
			LockGuard lockGuard(this->m_Mutex);

			if (nullptr == m_Context.TimerQueryHeap) {
				D3D12_QUERY_HEAP_DESC queryHeapDesc{
					.Type { D3D12_QUERY_HEAP_TYPE_TIMESTAMP },
					.Count { this->m_Resources.TimerQueries.Get_Capacity() * 2  }, // Use 2 D3D12 queries per 1 TimerQuery
					.NodeMask { 1 }
				};
				m_Context.Device->CreateQueryHeap(&queryHeapDesc, PARTING_IID_PPV_ARGS(&this->m_Context.TimerQueryHeap));

				RHIBufferDesc qbDesc;
				qbDesc.ByteSize = static_cast<Uint64>(queryHeapDesc.Count) * 8;
				qbDesc.CPUAccess = RHICPUAccessMode::Read;

				auto timerQueryBuffer{ this->CreateBuffer(qbDesc) };
				this->m_Context.TimerQueryResolveBuffer = timerQueryBuffer.Get();
			}
		}

		Uint32 queryIndex{ m_Resources.TimerQueries.Allocate() };

		ASSERT(Max_Uint32 != queryIndex);

		auto query{ new TimerQuery{ this->m_Resources } };
		query->m_BeginQueryIndex = static_cast<Uint32>(queryIndex) * 2;
		query->m_EndQueryIndex = query->m_BeginQueryIndex + 1;
		query->m_Resolved = false;
		query->m_Time = 0.f;

		return RefCountPtr<TimerQuery>::Create(query);
	}

	bool Device::Imp_PollTimerQuery(TimerQuery* query) {
		if (!query->m_Started)
			return false;

		if (nullptr == query->m_Fence)
			return true;

		if (query->m_Fence->GetCompletedValue() >= query->m_FenceCounter) {
			query->m_Fence = nullptr;
			return true;
		}

		return false;
	}

	float Device::Imp_Get_TimerQueryTime(TimerQuery* query) {
		if (!query->m_Resolved) {
			if (query->m_Fence) {
				D3D12WaitForFence(query->m_Fence, query->m_FenceCounter, this->m_FenceEvent);
				query->m_Fence = nullptr;
			}

			Uint64 frequency;
			this->Get_Queue(RHICommandQueue::Graphics)->m_Queue->GetTimestampFrequency(&frequency);

			D3D12_RANGE bufferReadRange{
				.Begin {query->m_BeginQueryIndex * sizeof(Uint64) },
				.End { (query->m_BeginQueryIndex + 2) * sizeof(Uint64) }
			};
			Uint64* data{};
			this->m_Context.TimerQueryResolveBuffer->m_Resource->Map(0, &bufferReadRange, reinterpret_cast<void**>(&data));

			query->m_Resolved = true;
			query->m_Time = static_cast<float>(static_cast<double>(data[query->m_EndQueryIndex] - data[query->m_BeginQueryIndex]) / static_cast<double>(frequency));

			m_Context.TimerQueryResolveBuffer->m_Resource->Unmap(0, nullptr);
		}

		return query->m_Time;
	}

	void Device::Imp_ResetTimerQuery(TimerQuery* query) {
		query->m_Started = false;
		query->m_Resolved = false;
		query->m_Fence = nullptr;
		query->m_FenceCounter = 0;
		query->m_Time = 0.f;
	}

	RefCountPtr<FrameBuffer> Device::Imp_CreateFrameBuffer(const RHIFrameBufferDesc<D3D12Tag>& desc) {
		using FrameBufferDesc = RHIFrameBufferDesc<D3D12Tag>;
		using FrameBufferInfo = RHIFrameBufferInfo<D3D12Tag>;

		auto fb{ new FrameBuffer{ this->m_Resources } };
		fb->m_Desc = desc;
		fb->m_Info = FrameBufferInfo::Build(desc);

		Texture* texture{ nullptr };
		if (0 != desc.ColorAttachmentCount)
			texture = desc.ColorAttachments[0].Texture;
		else if (desc.DepthStencilAttachment.Is_Valid())
			texture = desc.DepthStencilAttachment.Texture;
		ASSERT(nullptr != texture);

		fb->m_RenderTargetWidth = texture->m_Desc.Extent.Width;
		fb->m_RenderTargetHeight = texture->m_Desc.Extent.Height;

		for (const auto& attachment : Span<const RHIFrameBufferAttachment<D3D12Tag>>{ desc.ColorAttachments.data(), desc.ColorAttachmentCount }) {
			Texture* texture{ attachment.Texture };
			ASSERT(texture->m_Desc.Extent.Width == fb->m_RenderTargetWidth);
			ASSERT(texture->m_Desc.Extent.Height == fb->m_RenderTargetHeight);

			const auto index{ this->m_Resources.RenderTargetViewHeap.AllocateDescriptor() };

			const auto descriptorHandle{ this->m_Resources.RenderTargetViewHeap.Get_CPUHandle(index) };
			texture->CreateRTV(descriptorHandle, attachment.Format, attachment.Subresources);

			fb->m_RTVs[fb->m_RTVCount++] = index;
			fb->m_Texture[fb->m_TextureCount++] = texture;
		}

		if (desc.DepthStencilAttachment.Is_Valid()) {
			Texture* texture{ desc.DepthStencilAttachment.Texture };

			ASSERT(texture->m_Desc.Extent.Width == fb->m_RenderTargetWidth);
			ASSERT(texture->m_Desc.Extent.Height == fb->m_RenderTargetHeight);

			const auto index{ this->m_Resources.DepthStencilViewHeap.AllocateDescriptor() };

			const auto descriptorHandle{ this->m_Resources.DepthStencilViewHeap.Get_CPUHandle(index) };
			texture->CreateDSV(descriptorHandle, desc.DepthStencilAttachment.Subresources, desc.DepthStencilAttachment.IsReadOnly);

			fb->DSV = index;
			fb->m_Texture[fb->m_TextureCount++] = texture;//Ignore err
		}

		return RefCountPtr<FrameBuffer>::Create(fb);
	}

	RefCountPtr<GraphicsPipeline> Device::Imp_CreateGraphicsPipeline(const RHIGraphicsPipelineDesc<D3D12Tag>& desc, FrameBuffer* framebuffer) {
		RefCountPtr<D3D12RootSignature> pRS{ this->Get_RootSignature(Span<const RefCountPtr<BindingLayout>>{ desc.BindingLayouts.data(), desc.BindingLayoutCount }, (nullptr != desc.InputLayout)) };
		RefCountPtr<ID3D12PipelineState> pPSO{ this->CreatePipelineState(desc, pRS, framebuffer->Get_Info()) };

		return this->CreateHandleForNativeGraphicsPipeline(pRS, pPSO, desc, framebuffer->Get_Info());
	}

	RefCountPtr<ComputePipeline> Device::Imp_CreateComputePipeline(const RHIComputePipelineDesc<D3D12Tag>& desc) {
		RefCountPtr<D3D12RootSignature> pRS{ this->Get_RootSignature(Span<const RefCountPtr<BindingLayout>>{ desc.BindingLayouts.data(), desc.BindingLayoutCount }, false) };
		RefCountPtr<ID3D12PipelineState> pPSO{ this->CreatePipelineState(desc, pRS) };

		ASSERT(nullptr != pPSO);

		ComputePipeline* pso{ new ComputePipeline{} };

		pso->m_Desc = desc;
		pso->m_RootSignature = pRS;
		pso->m_PipelineState = pPSO;

		return RefCountPtr<ComputePipeline>::Create(pso);
	}


	RefCountPtr<BindingLayout> Device::Imp_CreateBindingLayout(const RHIBindingLayoutDesc& desc) {
		return RefCountPtr<BindingLayout>::Create(new BindingLayout{ desc });//TODO :  if new throw a std::bad_alloc exception ,but i do not using this exception in the project
	}

	RefCountPtr<BindingSet> Device::Imp_CreateBindingSet(const RHIBindingSetDesc<D3D12Tag>& desc, BindingLayout* layout) {
		BindingSet* Re{ new BindingSet{ this->m_Context, this->m_Resources } };

		Re->m_Desc = desc;
		Re->m_Layout = layout;
		Re->CreateDescriptors();

		return RefCountPtr<BindingSet>::Create(Re);
	}



	RefCountPtr<CommandList> Device::Imp_CreateCommandList(const RHICommandListParameters& desc) {
		return  RefCountPtr<CommandList>::Create(new CommandList{ this, this->Get_Queue(desc.QueueType), this->m_Context, this->m_Resources, desc });
	}

	Uint64 Device::Imp_ExecuteCommandLists(CommandList* const* commandLists, Uint32 numCommandLists, RHICommandQueue executionQueue) {
		this->m_CommandListsToExecute.resize(numCommandLists);
		for (Uint64 Index = 0; Index < numCommandLists; ++Index)
			this->m_CommandListsToExecute[Index] = commandLists[Index]->Get_D3D12CommandList();

		D3D12Queue* pQueue{ this->Get_Queue(executionQueue) };

		pQueue->m_Queue->ExecuteCommandLists(static_cast<Uint32>(this->m_CommandListsToExecute.size()), this->m_CommandListsToExecute.data());
		++pQueue->m_LastSubmittedInstance;
		D3D12_CHECK(pQueue->m_Queue->Signal(pQueue->m_Fence, pQueue->m_LastSubmittedInstance));

		for (Uint64 Index = 0; Index < numCommandLists; ++Index) {
			auto instance{ commandLists[Index]->Executed(pQueue) };
			pQueue->m_CommandListsInFlights.push_front(instance);
		}

		if (!D3D12_SUCCESS(m_Context.Device->GetDeviceRemovedReason()))
			LOG_ERROR("Device removed reason:");

		return pQueue->m_LastSubmittedInstance;
	}

	bool Device::Imp_WaitForIdle(void) {
		// Wait for every queue to reach its last submitted instance
		for (const auto& pQueue : this->m_Queues) {
			if (nullptr == pQueue)
				continue;

			if (pQueue->UpdateLastCompletedInstance() < pQueue->m_LastSubmittedInstance)
				D3D12WaitForFence(pQueue->m_Fence, pQueue->m_LastSubmittedInstance, this->m_FenceEvent);//TODO :
		}

		return true;
	}

	void Device::Imp_RunGarbageCollection(void) {
		for (const auto& pQueue : this->m_Queues) {
			if (nullptr == pQueue)
				continue;

			pQueue->UpdateLastCompletedInstance();

			// Starting from the back of the queue, i.e. oldest submitted command lists,
			// see if those command lists have finished executing.
			while (!pQueue->m_CommandListsInFlights.empty()) {
				if (pQueue->m_LastCompletedInstance >= pQueue->m_CommandListsInFlights.back()->SubmittedInstance)
					pQueue->m_CommandListsInFlights.pop_back();
				else
					break;
			}
		}
	}

	bool Device::Imp_QueryFeatureSupport(RHIFeature feature, void* pInfo, Uint64 infoSize) {
		switch (feature) {
			using enum RHIFeature;
		case Meshlets:return this->m_MeshletsSupported;
		case ComputeQueue:return (nullptr != this->Get_Queue(RHICommandQueue::Compute));
		case CopyQueue:return (nullptr != this->Get_Queue(RHICommandQueue::Copy));
		case ConservativeRasterization:return true;
		default:return false;
		}
	}

	RHIFormatSupport Device::Imp_QueryFormatSupport(RHIFormat format) {
		const DXGIFormatMapping& formatMapping{ Get_DXGIFormatMapping(format) };

		/*using enum RHIFormatSupport;*///NOTE :do not use this ,beacse enum value hash a name is Buffer

		RHIFormatSupport result{ RHIFormatSupport::None };

		D3D12_FEATURE_DATA_FORMAT_SUPPORT featureData{ .Format { formatMapping.RTVFormat} };

		this->m_Context.Device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &featureData, sizeof(featureData));

		if (featureData.Support1 & D3D12_FORMAT_SUPPORT1_BUFFER)
			result = result | RHIFormatSupport::Buffer;
		if (featureData.Support1 & (D3D12_FORMAT_SUPPORT1_TEXTURE1D | D3D12_FORMAT_SUPPORT1_TEXTURE2D | D3D12_FORMAT_SUPPORT1_TEXTURE3D | D3D12_FORMAT_SUPPORT1_TEXTURECUBE))
			result = result | RHIFormatSupport::Texture;
		if (featureData.Support1 & D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL)
			result = result | RHIFormatSupport::DepthStencil;
		if (featureData.Support1 & D3D12_FORMAT_SUPPORT1_RENDER_TARGET)
			result = result | RHIFormatSupport::RenderTarget;
		if (featureData.Support1 & D3D12_FORMAT_SUPPORT1_BLENDABLE)
			result = result | RHIFormatSupport::Blendable;

		if (formatMapping.SRVFormat != featureData.Format) {
			featureData.Format = formatMapping.SRVFormat;
			featureData.Support1 = D3D12_FORMAT_SUPPORT1_NONE;
			featureData.Support2 = D3D12_FORMAT_SUPPORT2_NONE;
			this->m_Context.Device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &featureData, sizeof(featureData));
		}

		if (featureData.Support1 & D3D12_FORMAT_SUPPORT1_IA_INDEX_BUFFER)
			result = result | RHIFormatSupport::IndexBuffer;
		if (featureData.Support1 & D3D12_FORMAT_SUPPORT1_IA_VERTEX_BUFFER)
			result = result | RHIFormatSupport::VertexBuffer;
		if (featureData.Support1 & D3D12_FORMAT_SUPPORT1_SHADER_LOAD)
			result = result | RHIFormatSupport::ShaderLoad;
		if (featureData.Support1 & D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE)
			result = result | RHIFormatSupport::ShaderSample;
		if (featureData.Support2 & D3D12_FORMAT_SUPPORT2_UAV_ATOMIC_ADD)
			result = result | RHIFormatSupport::ShaderAtomic;
		if (featureData.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD)
			result = result | RHIFormatSupport::ShaderUavLoad;
		if (featureData.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_STORE)
			result = result | RHIFormatSupport::ShaderUavStore;

		return result;
	}


}