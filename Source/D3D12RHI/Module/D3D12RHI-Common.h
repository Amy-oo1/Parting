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

#include "Core/ModuleBuild.h"

#include "D3D12RHI/Include/DirectXMacros.h"

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

#include "D3D12RHI/Module/D3D12RHI-Traits.h"

#endif // PARTING_MODULE_BUILD

namespace RHI::D3D12 {

	//D3D12Begin 

	PARTING_EXPORT using D3D12DescriptorIndex = Uint32;
	PARTING_EXPORT HEADER_INLINE constexpr D3D12DescriptorIndex g_InvalidDescriptorIndex{ ~0u };

	PARTING_EXPORT HEADER_INLINE constexpr Uint32 g_D3D12TextureDataPlacementAlignment{ 512 };

	PARTING_EXPORT HEADER_INLINE constexpr Uint32 g_D3D12ConstantBufferDataPlacementAlignment{ 256 };

	PARTING_EXPORT HEADER_INLINE constexpr Uint32 g_D3D12DefaultMassResourcePlacmentAlignment{ 4194304 };

	PARTING_EXPORT HEADER_INLINE constexpr Uint32 g_D3D12TildResourceTileSizeBytes{ 65536 };

	PARTING_EXPORT HEADER_INLINE constexpr Uint32 g_D3D12MaxRootParameterWordCount{ 32 };

	PARTING_EXPORT HEADER_INLINE constexpr Uint32 g_D3D12ReSetShadingRateCombinerCount{ 2 };

	PARTING_EXPORT HEADER_INLINE constexpr Uint32 g_D3D12ResourceBarrierAllSubresource{ 0xffffffff };

	PARTING_EXPORT HEADER_INLINE constexpr Uint32 g_D3D12DefaultShader4ComponentMapping{ ((((0) & 0x7) | (((1) & 0x7) << 3) | (((2) & 0x7) << (3 * 2)) | (((3) & 0x7) << (3 * 3)) | (1 << (3 * 4)))) };

	PARTING_EXPORT template<typename Derived>
		class D3D12DescriptorHeap;

	PARTING_EXPORT class D3D12RootSignature;

	PARTING_EXPORT struct D3D12DviceDesc final {
		ID3D12Device* Device{ nullptr };
		ID3D12CommandQueue* GraphicsQueue{ nullptr };
		ID3D12CommandQueue* ComputeQueue{ nullptr };
		ID3D12CommandQueue* CopyQueue{ nullptr };

		Uint32 RenderTargetViewHeapSize{ 1024 };
		Uint32 DepthStencilViewHeapSize{ 1024 };
		Uint32 ShaderResourceViewHeapSize{ 16384 };
		Uint32 SamplerHeapSize{ 1024 };
		Uint32 MaxTimerQueries{ 256 };
	};

	PARTING_EXPORT template<typename Derived>
		class D3D12DescriptorHeap :public NonCopyAndMoveAble {
		protected:
			D3D12DescriptorHeap(void) = default;
			PARTING_VIRTUAL ~D3D12DescriptorHeap(void) = default;

		public:
			STDNODISCARD D3D12DescriptorIndex AllocateDescriptor(Uint32 Count = 1) { return this->Get_Derived()->Imp_AllocateDescriptor(Count); }
			void ReleaseDescriptor(D3D12DescriptorIndex Offset, Uint32 Count = 1) { this->Get_Derived()->Imp_ReleaseDescriptor(Offset, Count); }
			STDNODISCARD D3D12_CPU_DESCRIPTOR_HANDLE Get_CPUHandle(D3D12DescriptorIndex Index)const { return this->Get_Derived()->Imp_Get_CPUHandle(Index); }
			STDNODISCARD D3D12_GPU_DESCRIPTOR_HANDLE Get_GPUHandle(D3D12DescriptorIndex Index)const { return this->Get_Derived()->Imp_Get_GPUHandle(Index); }
			STDNODISCARD D3D12_CPU_DESCRIPTOR_HANDLE Get_CPUHandleShaderVisible(D3D12DescriptorIndex Index)const { return this->Get_Derived()->Imp_Get_CPUHandleShaderVisible(Index); }

			STDNODISCARD ID3D12DescriptorHeap* Get_Heap(void)const { return this->Get_Derived()->Imp_Get_Heap(); }
			STDNODISCARD ID3D12DescriptorHeap* Get_ShaderVisibleHeap(void)const { return this->Get_Derived()->Imp_Get_ShaderVisibleHeap(); }

		private:
			STDNODISCARD Derived* Get_Derived(void)noexcept { return static_cast<Derived*>(this); }
			STDNODISCARD const Derived* Get_Derived(void)const noexcept { return static_cast<const Derived*>(this); }
		private:
			D3D12DescriptorIndex Imp_AllocateDescriptor(Uint32 Count) { LOG_ERROR("No Imp"); return 0; }
			void Imp_ReleaseDescriptor(D3D12DescriptorIndex Offset, Uint32 Count) { LOG_ERROR("No Imp"); }
			D3D12_CPU_DESCRIPTOR_HANDLE Imp_Get_CPUHandle(D3D12DescriptorIndex Index)const { LOG_ERROR("No Imp"); return D3D12_CPU_DESCRIPTOR_HANDLE{}; }
			D3D12_GPU_DESCRIPTOR_HANDLE Imp_Get_GPUHandle(D3D12DescriptorIndex Index)const { LOG_ERROR("No Imp"); return D3D12_GPU_DESCRIPTOR_HANDLE{}; }
			D3D12_CPU_DESCRIPTOR_HANDLE Imp_Get_CPUHandleShaderVisible(D3D12DescriptorIndex Index)const { LOG_ERROR("No Imp"); return D3D12_CPU_DESCRIPTOR_HANDLE{}; }
			ID3D12DescriptorHeap* Imp_Get_Heap(void)const { LOG_ERROR("No Imp"); return nullptr; }
			ID3D12DescriptorHeap* Imp_Get_ShaderVisibleHeap(void)const { LOG_ERROR("No Imp"); return nullptr; }
	};

	PARTING_EXPORT enum class DescriptorHeapType :Uint8 {
		RenderTargetView,
		DepthStencilView,
		ShaderResourceView,
		Sampler
	};

	PARTING_EXPORT struct Context final {
		RefCountPtr<ID3D12Device> Device;
		RefCountPtr<ID3D12Device2> Device2;
		RefCountPtr<ID3D12Device5> Device5;
		RefCountPtr<ID3D12Device8> Device8;

		RefCountPtr<ID3D12CommandSignature> DrawIndirectSignature;
		RefCountPtr<ID3D12CommandSignature> DrawIndexedIndirectSignature;
		RefCountPtr<ID3D12CommandSignature> DispatchIndirectSignature;
		RefCountPtr<ID3D12QueryHeap> TimerQueryHeap;

		RefCountPtr<Buffer> TimerQueryResolveBuffer;
	};

	PARTING_EXPORT class D3D12StaticDescriptorHeap final :public D3D12DescriptorHeap<D3D12StaticDescriptorHeap> {
		friend class D3D12DescriptorHeap<D3D12StaticDescriptorHeap>;


		/*friend class Texture;*/
	public:
		D3D12StaticDescriptorHeap(const Context& context) :
			D3D12DescriptorHeap<D3D12StaticDescriptorHeap>{},
			m_Context{ context } {
		}
		~D3D12StaticDescriptorHeap(void) = default;

	public:
		void CopyToShaderVisibleHeap(D3D12DescriptorIndex Index, Uint32 Count = 1);

		void D3D12AllocateResources(D3D12_DESCRIPTOR_HEAP_TYPE heapType, Uint32 DescriptorCount, bool shaderVisible);

	private:
		const Context& m_Context;

		Mutex m_Mutex{};
		RefCountPtr<ID3D12DescriptorHeap> m_Heap;
		RefCountPtr<ID3D12DescriptorHeap> m_ShaderVisibleHeap;
		D3D12_DESCRIPTOR_HEAP_TYPE m_HeapType{};
		D3D12_CPU_DESCRIPTOR_HANDLE m_StartCPUHandle{ 0 };
		D3D12_CPU_DESCRIPTOR_HANDLE m_StartCPUHandleShaderVisible{ 0 };
		D3D12_GPU_DESCRIPTOR_HANDLE m_StartGPUHandleShaderVisible{ 0 };
		Uint32 m_Stride{ 0 };
		Uint32 m_DescriptorCount{ 0 };

		BitVector m_AllocatedDescriptors;
		Uint32 m_SearchStart{ 0 };
		Uint32 m_AllocatedCount{ 0 };
	private:
		void D3D12Grow(Uint32 minRequiredSize);

	private:
		D3D12DescriptorIndex Imp_AllocateDescriptor(Uint32 Count);
		void Imp_ReleaseDescriptor(D3D12DescriptorIndex Offset, Uint32 Count);
		D3D12_CPU_DESCRIPTOR_HANDLE Imp_Get_CPUHandle(D3D12DescriptorIndex Index)const;
		D3D12_GPU_DESCRIPTOR_HANDLE Imp_Get_GPUHandle(D3D12DescriptorIndex Index)const;
		D3D12_CPU_DESCRIPTOR_HANDLE Imp_Get_CPUHandleShaderVisible(D3D12DescriptorIndex Index)const;
		ID3D12DescriptorHeap* Imp_Get_Heap(void)const { return this->m_Heap; }
		ID3D12DescriptorHeap* Imp_Get_ShaderVisibleHeap(void)const { return this->m_ShaderVisibleHeap; }
	};

	PARTING_EXPORT struct D3D12DeviceResources final {
		const Context& Context;

		D3D12StaticDescriptorHeap RenderTargetViewHeap;
		D3D12StaticDescriptorHeap DepthStencilViewHeap;
		D3D12StaticDescriptorHeap ShaderResourceViewHeap;
		D3D12StaticDescriptorHeap SamplerHeap;
		BitSetAllocator TimerQueries;

		UnorderedMap<Uint64, D3D12RootSignature*> RootSignatureCahe;
		UnorderedMap<DXGI_FORMAT, Uint8> DXGIFormatPlaneCounts;

		Uint8 Get_FormatPlaneCount(DXGI_FORMAT format) {
			auto& PlaneCount{ this->DXGIFormatPlaneCounts[format] };
			if (0 == PlaneCount) {
				D3D12_FEATURE_DATA_FORMAT_INFO ForamtInfo{ .Format{ format }, .PlaneCount{ 1 } };

				if (!D3D12_SUCCESS(this->Context.Device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_INFO, &ForamtInfo, sizeof(decltype(ForamtInfo)))))
					PlaneCount = Max_Uint8;
				else
					PlaneCount = ForamtInfo.PlaneCount;
			}
			if (Max_Uint8 == PlaneCount)
				return 0;

			return PlaneCount;
		}
	};

	PARTING_EXPORT class D3D12RootSignature final :public RHIResource<D3D12RootSignature> {
		friend class RHIResource<D3D12RootSignature>;

		friend class CommandList;
		friend class Device;
	public:
		D3D12RootSignature(D3D12DeviceResources& Resource) :
			RHIResource<D3D12RootSignature>{},
			m_DeviceResourcesRef{ Resource } {
		}

		~D3D12RootSignature(void) {
			if (auto It = this->m_DeviceResourcesRef.RootSignatureCahe.find(this->m_Hash); It != this->m_DeviceResourcesRef.RootSignatureCahe.end())
				this->m_DeviceResourcesRef.RootSignatureCahe.erase(It);
		}

	public:
		using D3D12RootParameterIndex = Uint32;

	private:
		D3D12DeviceResources& m_DeviceResourcesRef;

		Uint64 m_Hash{ 0 };//TODO :

		Array<Pair<RefCountPtr<BindingLayout>, D3D12RootParameterIndex>, g_MaxBindingLayoutCount> m_BindLayouts;
		RemoveCV<decltype(g_MaxBindingLayoutCount)>::type m_BindLayoutCount{ 0 };

		RefCountPtr<ID3D12RootSignature> m_RootSignature;

		Uint32 m_PushConstantByteSize{ 0 };
		D3D12RootParameterIndex m_RootParameterPushConstants{ ~0u };

	private:
		RHIObject Imp_GetNativeObject(RHIObjectType objecttyoe)const noexcept {
			switch (objecttyoe) {
			case RHIObjectType::D3D12_RootSignature:
				return RHIObject{ .Pointer{ this->m_RootSignature.Get()} };
			default:
				return RHIObject{};
			}
		}

	};

	PARTING_EXPORT class D3D12Queue final {
		friend class UploadManager;

		friend class CommandList;
		friend class Device;
	public:
		explicit D3D12Queue(const Context& context, ID3D12CommandQueue* queue) :
			m_Context{ context },
			m_Queue{ queue } {

			D3D12_CHECK(this->m_Context.Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, PARTING_IID_PPV_ARGS(&this->m_Fence)));
		}

		~D3D12Queue(void) = default;

	public:
		Uint64 UpdateLastCompletedInstance(void) {
			if (this->m_LastCompletedInstance < this->m_LastSubmittedInstance)
				this->m_LastCompletedInstance = this->m_Fence->GetCompletedValue();

			return this->m_LastCompletedInstance;
		}

	private:
		const Context& m_Context;
		RefCountPtr<ID3D12CommandQueue> m_Queue;
		RefCountPtr<ID3D12Fence> m_Fence;
		Uint64 m_LastSubmittedInstance{ 0 };
		Uint64 m_LastCompletedInstance{ 0 };
		Atomic<Uint64> m_RecordingInstance{ 1 };
		Deque<SharedPtr<struct CommandListInstance>> m_CommandListsInFlights;
	};

	//TODO this class is a struct but has a Unp func
	class BufferChunk final {
	public:
		static constexpr Uint64 c_SizeAlignment{ 4096 }; // GPU page size

		RefCountPtr<ID3D12Resource> D3D12Buffer;
		Uint64 Version{ 0 };
		Uint64 BufferSize{ 0 };
		Uint64 WritePointer{ 0 };
		void* CPUVirtualAddress{ nullptr };
		D3D12_GPU_VIRTUAL_ADDRESS GPUVirtualAddress{ static_cast<D3D12_GPU_VIRTUAL_ADDRESS>(0) };

		~BufferChunk(void) {
			if (nullptr != this->D3D12Buffer && nullptr != this->CPUVirtualAddress) {
				this->D3D12Buffer->Unmap(0, nullptr);
				this->CPUVirtualAddress = nullptr;
			}
		}
	};

	class UploadManager final {
	public:
		UploadManager(const Context& Context, D3D12Queue* Queue, Uint64 DefaultChunkSize) :
			m_Context{ Context },
			m_Queue{ Queue },
			m_DefaultChunkSize{ DefaultChunkSize }
		{
		}

		~UploadManager(void) = default;

	public:
		bool SuballocateBuffer(Uint64 size, ID3D12Resource** pBuffer, Uint64* pOffset, void** pCpuVA, D3D12_GPU_VIRTUAL_ADDRESS* pGpuVA, Uint64 currentVersion, Uint32 alignment);

		void SubmitChunks(Uint64 currentVersion, Uint64 submittedVersion);

	private:
		SharedPtr<BufferChunk> CreateChunk(Uint64 ByteSize)const;

	private:
		const Context& m_Context;
		D3D12Queue* m_Queue{ nullptr };
		Uint64 m_DefaultChunkSize{ 0 };
		Uint64 m_AllocatedMemory{ 0 };

		List<SharedPtr<BufferChunk>> m_ChunkPool;
		SharedPtr<BufferChunk> m_CurrentChunk;

	};

	struct InternalCommandList final {
		RefCountPtr<ID3D12CommandAllocator> Allocator;
		RefCountPtr<ID3D12GraphicsCommandList> CommandList;
		RefCountPtr<ID3D12GraphicsCommandList4> CommandList4;
		RefCountPtr<ID3D12GraphicsCommandList6> CommandList6;
		Uint64 LastSubmittedInstance{ 0 };
	};

	struct CommandListInstance final {
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

			Nullptr_T
		>;

		Uint64 SubmittedInstance{ 0 };
		RHICommandQueue CommandQueue{ RHICommandQueue::Graphics };
		RefCountPtr<ID3D12CommandAllocator> CommandAllocator;
		RefCountPtr<ID3D12CommandList> CommandList;
		RefCountPtr<ID3D12Fence> Fence;

		Vector<CommandResource> ReferencedResources;
		Vector<RefCountPtr<IUnknown>> ReferencedNativeResources;
		Vector<RefCountPtr<StagingTexture>> ReferencedStagingTextures;
		Vector<RefCountPtr<Buffer>> ReferencedStagingBuffers;
		Vector<RefCountPtr<TimerQuery>> ReferencedTimerQueries;
	};

	//Convert TODO trans to a table
	STDNODISCARD constexpr D3D12_SHADER_VISIBILITY ConvertShaderStage(RHIShaderType s) {
		switch (s) {
			using enum RHIShaderType;
		case Vertex:return D3D12_SHADER_VISIBILITY_VERTEX;
		case Hull:return D3D12_SHADER_VISIBILITY_HULL;
		case Domain:return D3D12_SHADER_VISIBILITY_DOMAIN;
		case Geometry:return D3D12_SHADER_VISIBILITY_GEOMETRY;
		case Pixel:return D3D12_SHADER_VISIBILITY_PIXEL;
		case Amplification:return D3D12_SHADER_VISIBILITY_AMPLIFICATION;
		case Mesh:return D3D12_SHADER_VISIBILITY_MESH;
		default:return D3D12_SHADER_VISIBILITY_ALL;
		}
	}

	STDNODISCARD D3D12_RESOURCE_STATES ConvertResourceStates(RHIResourceState stateBits) {
		using enum RHIResourceState;
		if (RHIResourceState::Common == stateBits)//NOTE : msvc has a bug ,must add RHIResourceState to erase error
			return D3D12_RESOURCE_STATE_COMMON;

		D3D12_RESOURCE_STATES result{ D3D12_RESOURCE_STATE_COMMON }; // also 0

		if ((stateBits & ConstantBuffer) != Unknown) result |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		if ((stateBits & VertexBuffer) != Unknown) result |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		if ((stateBits & IndexBuffer) != Unknown) result |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
		if ((stateBits & IndirectArgument) != Unknown) result |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
		if ((stateBits & ShaderResource) != Unknown) result |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		if ((stateBits & UnorderedAccess) != Unknown) result |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		if ((stateBits & RenderTarget) != Unknown) result |= D3D12_RESOURCE_STATE_RENDER_TARGET;
		if ((stateBits & DepthWrite) != Unknown) result |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
		if ((stateBits & DepthRead) != Unknown) result |= D3D12_RESOURCE_STATE_DEPTH_READ;
		if ((stateBits & StreamOut) != Unknown) result |= D3D12_RESOURCE_STATE_STREAM_OUT;
		if ((stateBits & CopyDest) != Unknown) result |= D3D12_RESOURCE_STATE_COPY_DEST;
		if ((stateBits & CopySource) != Unknown) result |= D3D12_RESOURCE_STATE_COPY_SOURCE;
		if ((stateBits & ResolveDest) != Unknown) result |= D3D12_RESOURCE_STATE_RESOLVE_DEST;
		if ((stateBits & ResolveSource) != Unknown) result |= D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
		if ((stateBits & Present) != Unknown) result |= D3D12_RESOURCE_STATE_PRESENT;
		if ((stateBits & ShadingRateSurface) != Unknown) result |= D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE;

		return result;
	}

	STDNODISCARD D3D12_SHADING_RATE_COMBINER ConvertShadingRateCombiner(RHIShadingRateCombiner combiner) {
		switch (combiner) {
			using enum RHIShadingRateCombiner;
		case Override:return D3D12_SHADING_RATE_COMBINER_OVERRIDE;
		case Min:return D3D12_SHADING_RATE_COMBINER_MIN;
		case Max:return D3D12_SHADING_RATE_COMBINER_MAX;
		case ApplyRelative:return D3D12_SHADING_RATE_COMBINER_SUM;
		case Passthrough:default:return D3D12_SHADING_RATE_COMBINER_PASSTHROUGH;
		}
	}

	D3D12_SHADING_RATE ConvertPixelShadingRate(RHIVariableShadingRate shadingRate) {
		switch (shadingRate) {
			using enum RHIVariableShadingRate;
		case e1x2:return D3D12_SHADING_RATE_1X2;
		case e2x1:return D3D12_SHADING_RATE_2X1;
		case e2x2:return D3D12_SHADING_RATE_2X2;
		case e2x4:return D3D12_SHADING_RATE_2X4;
		case e4x2:return D3D12_SHADING_RATE_4X2;
		case e4x4:return D3D12_SHADING_RATE_4X4;
		case e1x1:default:return D3D12_SHADING_RATE_1X1;
		}
	}

	D3D12_COMPARISON_FUNC ConvertComparisonFunc(RHIComparisonFunc value) {
		switch (value) {
			using enum RHIComparisonFunc;
		case Never:return D3D12_COMPARISON_FUNC_NEVER;
		case Less:return D3D12_COMPARISON_FUNC_LESS;
		case Equal:return D3D12_COMPARISON_FUNC_EQUAL;
		case LessOrEqual:return D3D12_COMPARISON_FUNC_LESS_EQUAL;
		case Greater:return D3D12_COMPARISON_FUNC_GREATER;
		case NotEqual:return D3D12_COMPARISON_FUNC_NOT_EQUAL;
		case GreaterOrEqual:return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		case Always:return D3D12_COMPARISON_FUNC_ALWAYS;
		default:ASSERT(false); return D3D12_COMPARISON_FUNC_NEVER;
		}
	}

	void D3D12WaitForFence(ID3D12Fence* fence, uint64_t value, HANDLE event) {
		// Test if the fence has been reached
		if (fence->GetCompletedValue() < value) {
			// If it's not, wait for it to finish using an event
			ResetEvent(event);
			fence->SetEventOnCompletion(value, event);
			WaitForSingleObject(event, 0xFFFFFFFF);
		}
	}

	//D3D12End


	class EventQuery final :public RHIEventQuery<EventQuery> {
		friend class RHIResource<EventQuery>;
		friend class RHIEventQuery<EventQuery>;

		friend class CommandList;
		friend class Device;
	public:
		EventQuery(void) = default;
		~EventQuery(void) = default;

	public:

	private:


	private:
		RefCountPtr<ID3D12Fence> m_Fence{ nullptr };
		Uint64 m_FenceValue{ 0 };
		bool m_Started{ false };
		bool m_Resolved{ false };

	private:
		RHIObject Imp_GetNativeObject(RHIObjectType)const noexcept { LOG_ERROR("Imp But Empty");  return RHIObject{}; };
	};

	class TimerQuery final : public RHIEventQuery<TimerQuery> {
		friend class RHIResource<TimerQuery>;
		friend class RHIEventQuery<TimerQuery>;

		friend class CommandList;
		friend class Device;
	public:
		TimerQuery(D3D12DeviceResources& deviceResources) :
			RHIEventQuery<TimerQuery>{},
			m_DeviceResourcesRef{ deviceResources } {
		}

		~TimerQuery(void) {
			this->m_DeviceResourcesRef.TimerQueries.Release(this->m_BeginQueryIndex / 2);
		}

	public:

	private:

	private:
		D3D12DeviceResources& m_DeviceResourcesRef;

		Uint32 m_BeginQueryIndex{ 0 };
		Uint32 m_EndQueryIndex{ 0 };

		RefCountPtr<ID3D12Fence> m_Fence;
		uint64_t m_FenceCounter{ 0 };

		bool m_Started{ false };
		bool m_Resolved{ false };
		float m_Time{ 0.f };
	private:
		RHIObject Imp_GetNativeObject(RHIObjectType)const noexcept { LOG_ERROR("Imp But Empty");  return RHIObject{}; };
	};

	//NOTE :Src

	inline void D3D12StaticDescriptorHeap::D3D12AllocateResources(D3D12_DESCRIPTOR_HEAP_TYPE heapType, Uint32 DescriptorCount, bool shaderVisible) {
		this->m_Heap = nullptr;
		this->m_ShaderVisibleHeap = nullptr;

		this->m_DescriptorCount = DescriptorCount;
		this->m_HeapType = heapType;

		D3D12_DESCRIPTOR_HEAP_DESC HeapDesc{ .Type { heapType }, .NumDescriptors { DescriptorCount } };

		D3D12_CHECK(this->m_Context.Device->CreateDescriptorHeap(&HeapDesc, PARTING_IID_PPV_ARGS(&this->m_Heap)));

		if (shaderVisible) {
			HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

			D3D12_CHECK(this->m_Context.Device->CreateDescriptorHeap(&HeapDesc, PARTING_IID_PPV_ARGS(&this->m_ShaderVisibleHeap)));
			this->m_StartCPUHandleShaderVisible = this->m_ShaderVisibleHeap->GetCPUDescriptorHandleForHeapStart();
			this->m_StartGPUHandleShaderVisible = this->m_ShaderVisibleHeap->GetGPUDescriptorHandleForHeapStart();
		}

		this->m_StartCPUHandle = this->m_Heap->GetCPUDescriptorHandleForHeapStart();
		this->m_Stride = this->m_Context.Device->GetDescriptorHandleIncrementSize(heapType);

		this->m_AllocatedDescriptors.resize(DescriptorCount);
	}

	inline void D3D12StaticDescriptorHeap::D3D12Grow(Uint32 minRequiredSize) {
		Uint32 OldSize{ this->m_DescriptorCount };
		Uint32 NewSize{ Math::NextPowerOf2(minRequiredSize) };

		RefCountPtr<ID3D12DescriptorHeap> OldHeap{ this->m_Heap };
		this->D3D12AllocateResources(this->m_HeapType, NewSize, nullptr != this->m_ShaderVisibleHeap);

		this->m_Context.Device->CopyDescriptorsSimple(
			OldSize,
			this->m_StartCPUHandle,
			OldHeap->GetCPUDescriptorHandleForHeapStart(),
			this->m_HeapType
		);

		if (nullptr != this->m_ShaderVisibleHeap)
			this->m_Context.Device->CopyDescriptorsSimple(
				OldSize,
				this->m_StartCPUHandleShaderVisible,
				OldHeap->GetCPUDescriptorHandleForHeapStart(),
				this->m_HeapType
			);
	}

	inline void D3D12StaticDescriptorHeap::CopyToShaderVisibleHeap(D3D12DescriptorIndex Index, Uint32 Count) {
		this->m_Context.Device->CopyDescriptorsSimple(
			Count,
			this->Get_CPUHandleShaderVisible(Index),
			this->Get_CPUHandle(Index),
			this->m_HeapType
		);
	}


	//Imp

	inline D3D12DescriptorIndex D3D12StaticDescriptorHeap::Imp_AllocateDescriptor(Uint32 Count) {
		ASSERT(Count >= 1);

		Uint32 FoundIndex{ 0 };
		Uint32 FreeCount{ 0 };
		bool Found{ false };

		{//TODO Load def
			LockGuard lock{ this->m_Mutex };

			for (auto Index = this->m_SearchStart; Index < this->m_DescriptorCount; ++Index) {
				if (this->m_AllocatedDescriptors[Index])
					FreeCount = 0;
				else
					++FreeCount;
				if (FreeCount >= Count) {
					Found = true;
					FoundIndex = Index - Count + 1;
					break;
				}
			}

			if (!Found) {
				FoundIndex = this->m_DescriptorCount;

				this->D3D12Grow(this->m_DescriptorCount + Count);
			}

			for (auto Index = FoundIndex; Index < FoundIndex + Count; ++Index)
				this->m_AllocatedDescriptors[Index] = true;
		}

		this->m_AllocatedCount += Count;
		this->m_SearchStart = FoundIndex + Count;

		return FoundIndex;
	}

	inline void D3D12StaticDescriptorHeap::Imp_ReleaseDescriptor(D3D12DescriptorIndex Offset, Uint32 Count) {
		if (0 == Count)
			return;

		{
			LockGuard lock{ this->m_Mutex };

			for (auto Index = Offset; Index < Offset + Count; ++Index) {
				ASSERT(true == this->m_AllocatedDescriptors[Index]);

				this->m_AllocatedDescriptors[Index] = false;
			}
		}

		this->m_AllocatedCount -= Count;
		if (this->m_SearchStart > Offset)
			this->m_SearchStart = Offset;
	}

	inline D3D12_CPU_DESCRIPTOR_HANDLE D3D12StaticDescriptorHeap::Imp_Get_CPUHandle(D3D12DescriptorIndex Index)const {
		auto Handle{ this->m_StartCPUHandle };
		Handle.ptr += static_cast<Uint64>(Index * this->m_Stride);

		return Handle;
	}

	inline D3D12_GPU_DESCRIPTOR_HANDLE D3D12StaticDescriptorHeap::Imp_Get_GPUHandle(D3D12DescriptorIndex Index) const {
		auto Handle{ this->m_StartGPUHandleShaderVisible };
		Handle.ptr += static_cast<Uint64>(Index * this->m_Stride);

		return Handle;
	}

	inline D3D12_CPU_DESCRIPTOR_HANDLE D3D12StaticDescriptorHeap::Imp_Get_CPUHandleShaderVisible(D3D12DescriptorIndex Index) const {
		auto Handle{ this->m_StartCPUHandleShaderVisible };
		Handle.ptr += static_cast<Uint64>(Index * this->m_Stride);

		return Handle;
	}

	//Src
	SharedPtr<BufferChunk> UploadManager::CreateChunk(Uint64 ByteSize)const {
		auto Re{ MakeShared<BufferChunk>() };

		ByteSize = Math::Align(ByteSize, BufferChunk::c_SizeAlignment);

		constexpr D3D12_HEAP_PROPERTIES HeapDesc{ .Type{ D3D12_HEAP_TYPE_UPLOAD } };

		D3D12_RESOURCE_DESC BufferDesc{
			.Dimension{ D3D12_RESOURCE_DIMENSION_BUFFER },
			.Width{ ByteSize },
			.Height{ 1 },
			.DepthOrArraySize{ 1 },
			.MipLevels{ 1 },
			.SampleDesc{.Count{ 1 } },
			.Layout{ D3D12_TEXTURE_LAYOUT_ROW_MAJOR }
		};

		D3D12_CHECK(this->m_Context.Device->CreateCommittedResource(
			&HeapDesc,
			D3D12_HEAP_FLAG_NONE,
			&BufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			PARTING_IID_PPV_ARGS(&Re->D3D12Buffer)
		));

		D3D12_CHECK(Re->D3D12Buffer->Map(0, nullptr, &Re->CPUVirtualAddress));

		Re->BufferSize = ByteSize;
		Re->GPUVirtualAddress = Re->D3D12Buffer->GetGPUVirtualAddress();

		return Re;
	}

	bool UploadManager::SuballocateBuffer(Uint64 size, ID3D12Resource** pBuffer, Uint64* pOffset, void** pCpuVA, D3D12_GPU_VIRTUAL_ADDRESS* pGpuVA, Uint64 currentVersion, Uint32 alignment) {
		SharedPtr<BufferChunk> chunkToRetire{ nullptr };

		if (nullptr != this->m_CurrentChunk) {
			Uint64 alignedOffset{ Math::Align(this->m_CurrentChunk->WritePointer, static_cast<Uint64>(alignment)) };
			auto endOfDataInChunk{ alignedOffset + size };

			if (endOfDataInChunk <= this->m_CurrentChunk->BufferSize) {
				this->m_CurrentChunk->WritePointer = endOfDataInChunk;

				if (nullptr != pBuffer)
					*pBuffer = this->m_CurrentChunk->D3D12Buffer.Get();
				if (nullptr != pOffset)
					*pOffset = alignedOffset;
				if (nullptr != pCpuVA && nullptr != this->m_CurrentChunk->CPUVirtualAddress)
					*pCpuVA = static_cast<char*>(this->m_CurrentChunk->CPUVirtualAddress) + alignedOffset;
				if (nullptr != pGpuVA && 0 != this->m_CurrentChunk->GPUVirtualAddress)
					*pGpuVA = this->m_CurrentChunk->GPUVirtualAddress + alignedOffset;

				return true;
			}

			chunkToRetire = this->m_CurrentChunk;
			this->m_CurrentChunk.reset();
		}

		auto completedInstance{ this->m_Queue->m_LastCompletedInstance };

		for (auto It = this->m_ChunkPool.begin(); It != this->m_ChunkPool.end(); ++It) {
			auto chunk{ *It };
			if (VersionGetSubmitted(chunk->Version) && VersionGetInstance(chunk->Version) <= completedInstance)
				chunk->Version = 0;

			if (0 == chunk->Version && chunk->BufferSize >= size) {
				this->m_ChunkPool.erase(It);
				this->m_CurrentChunk = chunk;

				break;
			}
		}

		if (nullptr != chunkToRetire)
			this->m_ChunkPool.push_back(chunkToRetire);

		if (nullptr == this->m_CurrentChunk)
			this->m_CurrentChunk = this->CreateChunk(Math::Align(Math::Max(size, this->m_DefaultChunkSize), BufferChunk::c_SizeAlignment));

		this->m_CurrentChunk->Version = currentVersion;
		this->m_CurrentChunk->WritePointer = size;

		if (nullptr != pBuffer)
			*pBuffer = this->m_CurrentChunk->D3D12Buffer.Get();
		if (nullptr != pOffset)
			*pOffset = 0;
		if (nullptr != pCpuVA)
			*pCpuVA = this->m_CurrentChunk->CPUVirtualAddress;
		if (nullptr != pGpuVA)
			*pGpuVA = this->m_CurrentChunk->GPUVirtualAddress;

		return true;

	}

	inline void UploadManager::SubmitChunks(Uint64 currentVersion, Uint64 submittedVersion) {
		if (nullptr != this->m_CurrentChunk) {
			this->m_ChunkPool.push_back(this->m_CurrentChunk);
			this->m_CurrentChunk.reset();
		}

		for (const auto& chunk : this->m_ChunkPool)
			if (chunk->Version == currentVersion)
				chunk->Version = submittedVersion;
	}
}