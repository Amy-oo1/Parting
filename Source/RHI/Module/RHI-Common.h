#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"


PARTING_SUBMODULE(RHI, Common)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Container;
PARTING_IMPORT Logger;

PARTING_SUBMODE_IMPORT(Traits)

#else
#pragma once

#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"
//Global

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Container/Module/Container.h"
#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI-Traits.h"

#endif // PARTING_MODULE_BUILD

//NOTE : this FIle(Sub Module) not named Misc it is in not only other Modle (logic)

namespace RHI {

	PARTING_EXPORT using GPUVirtualAddress = Uint64;

	PARTING_EXPORT HEADER_INLINE constexpr Uint32 g_MaxRenderTargetCount{ 8 };
	PARTING_EXPORT HEADER_INLINE constexpr Uint16 g_MaxViewportCount{ 16 };
	PARTING_EXPORT HEADER_INLINE constexpr Uint16 g_MaxVertexAttributeCount{ 16 };
	PARTING_EXPORT HEADER_INLINE constexpr Uint32 g_MaxBindingLayoutCount{ 5 };
	PARTING_EXPORT HEADER_INLINE constexpr Uint32 g_MaxBindingsPerLayout{ 128 };
	PARTING_EXPORT HEADER_INLINE constexpr Uint32 g_MaxVolatileConstantBuffers{ 32 };
	PARTING_EXPORT HEADER_INLINE constexpr Uint32 g_MaxVolatileConstantBufferCountPerLayout{ 6 };
	PARTING_EXPORT HEADER_INLINE constexpr Uint32 g_MaxBindlessLayoutCount{ 16 };
	PARTING_EXPORT HEADER_INLINE constexpr Uint32 g_ConstantBufferOffsetSizeAlignment{ 256 };


	PARTING_EXPORT struct RHIOffset2D final {
		Uint32 X{ 0 };
		Uint32 Y{ 0 };

		STDNODISCARD constexpr bool operator==(const RHIOffset2D&)const noexcept = default;
		STDNODISCARD constexpr bool operator!=(const RHIOffset2D&)const noexcept = default;
	};

	PARTING_EXPORT struct RHIOffset3D final {
		Uint32 X{ 0 };
		Uint32 Y{ 0 };
		Uint32 Z{ 0 };

		STDNODISCARD constexpr bool operator==(const RHIOffset3D&)const noexcept = default;
		STDNODISCARD constexpr bool operator!=(const RHIOffset3D&)const noexcept = default;
	};

	PARTING_EXPORT struct RHIExtent2D final {
		Uint32 Width{ 0 };
		Uint32 Height{ 0 };

		STDNODISCARD constexpr bool operator==(const RHIExtent2D&)const noexcept = default;
		STDNODISCARD constexpr bool operator!=(const RHIExtent2D&)const noexcept = default;
	};

	PARTING_EXPORT struct RHIExtent3D final {
		Uint32 Width{ 0 };
		Uint32 Height{ 0 };
		Uint32 Depth{ 0 };

		STDNODISCARD constexpr bool operator==(const RHIExtent3D&)const noexcept = default;
		STDNODISCARD constexpr bool operator!=(const RHIExtent3D&)const noexcept = default;
	};

	PARTING_EXPORT struct RHIRect2D final {
		RHIOffset2D Offset;
		RHIExtent2D Extent;

		static constexpr RHIRect2D Build(Uint32 minX, Uint32 minY, Uint32 maxX, Uint32 maxY) {
			return RHIRect2D{
				RHIOffset2D{.X{ minX }, .Y{ minY } },
				RHIExtent2D{.Width{ maxX - minX },.Height{ maxY - minY } }
			};
		}

		//TODO :not use this func
		static RHIRect2D Merge(const RHIRect2D& a, const RHIRect2D& b) {
			return RHIRect2D{
				RHIOffset2D{.X{ Math::Min(a.Offset.X, b.Offset.X) }, .Y{ Math::Min(a.Offset.Y, b.Offset.Y) } },
				RHIExtent2D{
					.Width{ Math::Max(a.Offset.X + a.Extent.Width, b.Offset.X + b.Extent.Width) - Math::Min(a.Offset.X, b.Offset.X) },
					.Height{ Math::Max(a.Offset.Y + a.Extent.Height, b.Offset.Y + b.Extent.Height) - Math::Min(a.Offset.Y, b.Offset.Y) } }
			};
		}

		STDNODISCARD constexpr bool operator==(const RHIRect2D&)const noexcept = default;
		STDNODISCARD constexpr bool operator!=(const RHIRect2D&)const noexcept = default;
	};

	PARTING_EXPORT enum class RHICPUAccessMode :Uint8 {
		None,
		Read,
		Write
	};

	PARTING_EXPORT enum class RHIResourceState :Uint32 {
		Unknown = 0,
		Common = 0x00000001,
		ConstantBuffer = 0x00000002,
		VertexBuffer = 0x00000004,
		IndexBuffer = 0x00000008,
		IndirectArgument = 0x00000010,
		ShaderResource = 0x00000020,
		UnorderedAccess = 0x00000040,
		RenderTarget = 0x00000080,
		DepthWrite = 0x00000100,
		DepthRead = 0x00000200,
		StreamOut = 0x00000400,
		CopyDest = 0x00000800,
		CopySource = 0x00001000,
		ResolveDest = 0x00002000,
		ResolveSource = 0x00004000,
		Present = 0x00008000,//TODO
		AccelStructRead = 0x00010000,
		AccelStructWrite = 0x00020000,
		AccelStructBuildInput = 0x00040000,
		AccelStructBuildBlas = 0x00080000,
		ShadingRateSurface = 0x00100000,
		OpacityMicromapWrite = 0x00200000,
		OpacityMicromapBuildInput = 0x00400000,
	};
	EXPORT_ENUM_CLASS_OPERATORS(RHIResourceState);

	PARTING_EXPORT enum class RHISharedResourceFlag : Uint32 {
		None = 0,

		// D3D11: adds D3D11_RESOURCE_MISC_SHARED
		// D3D12: adds D3D12_HEAP_FLAG_SHARED
		// Vulkan: adds vk::ExternalMemoryImageCreateInfo and vk::ExportMemoryAllocateInfo/vk::ExternalMemoryBufferCreateInfo
		Shared = 0x01,

		// D3D11: adds (D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX | D3D11_RESOURCE_MISC_SHARED_NTHANDLE)
		// D3D12, Vulkan: ignored
		Shared_NTHandle = 0x02,

		// D3D12: adds D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER and D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER
		// D3D11, Vulkan: ignored
		Shared_CrossAdapter = 0x04,
	};
	EXPORT_ENUM_CLASS_OPERATORS(RHISharedResourceFlag);

	PARTING_EXPORT enum class RHIFeature : Uint8 {
		DeferredCommandLists,
		SinglePassStereo,
		RayTracingAccelStruct,
		RayTracingPipeline,
		RayTracingOpacityMicromap,
		RayTracingClusters,
		RayQuery,
		ShaderExecutionReordering,
		Spheres,
		LinearSweptSpheres,
		FastGeometryShader,
		Meshlets,
		ConservativeRasterization,
		VariableRateShading,
		ShaderSpecializations,
		VirtualResources,
		ComputeQueue,
		CopyQueue,
		ConstantBufferRanges
	};

	PARTING_EXPORT struct RHIVariableRateShadingFeatureInfo {
		Uint32 ShadingRateImageTileSize;
	};

	PARTING_EXPORT enum class RHIMessageSeverity : Uint8 {
		Info,
		Warning,
		Error,
		Fatal
	};

	PARTING_EXPORT template<typename Derived>
		class RHIMessageCallback :public NonCopyAndMoveAble {
		protected:
			RHIMessageCallback(void) = default;
			PARTING_VIRTUAL ~RHIMessageCallback(void) = default;

		public:
			void Message(RHIMessageSeverity severity, const char* message) { this->Get_Derived()->Imp_Message(severity, message); }
		private:
			STDNODISCARD constexpr Derived* Get_Derived(void)const noexcept { return static_cast<Derived*>(this); }
		private:
			void Imp_Message(RHIMessageSeverity severity, const char* message) { LOG_ERROR("No Imp"); }
	};

	PARTING_EXPORT template<typename Derived>
		class RHIEventQuery :public RHIResource<Derived> {
		friend class RHIResource<Derived>;
		protected:
			RHIEventQuery(void) = default;
			PARTING_VIRTUAL ~RHIEventQuery(void) = default;
	};

	PARTING_EXPORT template<typename Derived>
		class RHITimerQuery :public RHIResource<Derived> {
		friend class RHIResource<Derived>;
		protected:
			RHITimerQuery(void) = default;
			PARTING_VIRTUAL ~RHITimerQuery(void) = default;
	};

	PARTING_EXPORT template<APITagConcept APITag>
		using RHIShaderBindingResources = Variant<
		RefCountPtr<typename ShaderBindingResourceType<APITag>::Imp_Texture>,
		RefCountPtr<typename ShaderBindingResourceType<APITag>::Imp_SamplerFeedbackTexture>,
		RefCountPtr<typename ShaderBindingResourceType<APITag>::Imp_Buffer>,
		RefCountPtr<typename ShaderBindingResourceType<APITag>::Imp_Sampler>,
		Nullptr_T
		>;


}
