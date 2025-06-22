#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"


PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"

PARTING_SUBMODULE(RHI, Texture)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT String;
PARTING_IMPORT Container;
PARTING_IMPORT Algorithm;
PARTING_IMPORT Color;
PARTING_IMPORT Logger;

PARTING_SUBMODE_IMPORT(Resource)
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

#endif // PARTING_MODULE_BUILD

namespace RHI {

	PARTING_EXPORT struct RHIBufferDesc final {
		Uint64 ByteSize{ 0 };
		Uint32 StructStride{ 0 }; // if non-zero it's structured
		Uint32 MaxVersions{ 0 }; // only valid and required to be nonzero for volatile buffers on Vulkan
		WString DebugName;
		RHIFormat Format{ RHIFormat::UNKNOWN }; // for typed buffer views
		bool CanHaveUAVs{ false };
		bool CanHaveTypedViews{ false };
		bool CanHaveRawViews{ false };
		bool IsVertexBuffer{ false };
		bool IsIndexBuffer{ false };
		bool IsConstantBuffer{ false };
		bool IsDrawIndirectArgs{ false };

		// A dynamic/upload buffer whose contents only live in the current command list
		bool IsVolatile{ false };

		// Indicates that the buffer is created with no backing memory,
		// and memory is bound to the buffer later using bindBufferMemory.
		// On DX12, the buffer resource is created at the time of memory binding.
		bool IsVirtual{ false };

		RHIResourceState InitialState{ RHIResourceState::Common };

		// see TextureDesc::keepInitialState
		bool KeepInitialState{ false };

		RHICPUAccessMode CPUAccess{ RHICPUAccessMode::None };
	};

	PARTING_EXPORT class RHIBufferDescBuilder final {
	public:
		constexpr RHIBufferDescBuilder& Reset(void) { this->m_Desc = RHIBufferDesc{}; return *this; }
		
		constexpr RHIBufferDescBuilder& Set_ByteSize(const Uint64 byteSize) { this->m_Desc.ByteSize = byteSize; return *this; }
		constexpr RHIBufferDescBuilder& Set_StructStride(const Uint32 structStride) { this->m_Desc.StructStride = structStride; return *this; }
		constexpr RHIBufferDescBuilder& Set_MaxVersions(const Uint32 maxVersions) { this->m_Desc.MaxVersions = maxVersions; return *this; }
		constexpr RHIBufferDescBuilder& Set_DebugName(const WString& debugName) { this->m_Desc.DebugName = debugName; return *this; }
		constexpr RHIBufferDescBuilder& Set_Format(const RHIFormat format) { this->m_Desc.Format = format; return *this; }
		constexpr RHIBufferDescBuilder& Set_CanHaveUAVs(const bool canHaveUAVs) { this->m_Desc.CanHaveUAVs = canHaveUAVs; return *this; }
		constexpr RHIBufferDescBuilder& Set_CanHaveTypedViews(const bool canHaveTypedViews) { this->m_Desc.CanHaveTypedViews = canHaveTypedViews; return *this; }
		constexpr RHIBufferDescBuilder& Set_CanHaveRawViews(const bool canHaveRawViews) { this->m_Desc.CanHaveRawViews = canHaveRawViews; return *this; }
		constexpr RHIBufferDescBuilder& Set_IsVertexBuffer(const bool isVertexBuffer) { this->m_Desc.IsVertexBuffer = isVertexBuffer; return *this; }
		constexpr RHIBufferDescBuilder& Set_IsIndexBuffer(const bool isIndexBuffer) { this->m_Desc.IsIndexBuffer = isIndexBuffer; return *this; }
		constexpr RHIBufferDescBuilder& Set_IsConstantBuffer(const bool isConstantBuffer) { this->m_Desc.IsConstantBuffer = isConstantBuffer; return *this; }
		constexpr RHIBufferDescBuilder& Set_IsDrawIndirectArgs(const bool isDrawIndirectArgs) { this->m_Desc.IsDrawIndirectArgs = isDrawIndirectArgs; return *this; }
		constexpr RHIBufferDescBuilder& Set_IsVolatile(const bool isVolatile) { this->m_Desc.IsVolatile = isVolatile; return *this; }
		constexpr RHIBufferDescBuilder& Set_IsVirtual(const bool isVirtual) { this->m_Desc.IsVirtual = isVirtual; return *this; }
		constexpr RHIBufferDescBuilder& Set_InitialState(const RHIResourceState initialState) { this->m_Desc.InitialState = initialState; return *this; }
		constexpr RHIBufferDescBuilder& Set_KeepInitialState(const bool keepInitialState) { this->m_Desc.KeepInitialState = keepInitialState; return *this; }
		constexpr RHIBufferDescBuilder& Set_CPUAccess(const RHICPUAccessMode cpuAccess) { this->m_Desc.CPUAccess = cpuAccess; return *this; }
	
		constexpr const RHIBufferDesc& Build(void) const { return this->m_Desc; }

		STDNODISCARD static RHIBufferDesc CreateVolatileConstantBufferDesc(Uint32 ByteSize, Uint32 MaxVersions) {
			return RHIBufferDesc{
				.ByteSize{ ByteSize },
				.MaxVersions{ MaxVersions },
				.IsConstantBuffer {true },
				.IsVolatile { true }
			};
		}

		//TODO Move

	private:
		RHIBufferDesc m_Desc{};
	};

	PARTING_EXPORT struct RHIBufferRange final {
		Uint64 Offset{ 0 };
		Uint64 ByteSize{ 0 };

		STDNODISCARD RHIBufferRange Resolve(const RHIBufferDesc& desc) const {
			RHIBufferRange Re{};
			Re.Offset = Math::Min(this->Offset,desc.ByteSize);

			if (0 == this->ByteSize)
				Re.ByteSize = desc.ByteSize - this->Offset;
			else 
				Re.ByteSize = Math::Min(this->ByteSize, desc.ByteSize - Re.Offset);

			return Re;
		}
		
		STDNODISCARD bool Is_EntireBuffer(const RHIBufferDesc& desc) const { return 0 == this->Offset && (Max_Uint64 == this->ByteSize || this->ByteSize == desc.ByteSize); }
		
		STDNODISCARD constexpr bool operator==(const RHIBufferRange&) const noexcept = default;
		STDNODISCARD constexpr bool operator!=(const RHIBufferRange&) const noexcept = default;
	};

	PARTING_EXPORT HEADER_INLINE constexpr RHIBufferRange g_EntireBuffer{ .Offset{ 0ull }, .ByteSize{ Max_Uint64 } };

	PARTING_EXPORT template<typename Derived>
	class RHIBuffer :public RHIResource<Derived> {
		friend class RHIResource<Derived>;
	protected:
		RHIBuffer(void) = default;
		PARTING_VIRTUAL ~RHIBuffer(void) = default;

	public:
		STDNODISCARD const RHIBufferDesc& Get_Desc(void) const { return this->Get_Derived()->Imp_Get_Desc(); }
		STDNODISCARD GPUVirtualAddress Get_GPUVirtualAddress(void) const { return this->Get_Derived()->Imp_Get_GPUVirtualAddress(); }
	private:
		STDNODISCARD Derived* Get_Derived(void)noexcept { return static_cast<Derived*>(this); }
		STDNODISCARD const Derived* Get_Derived(void)const noexcept { return static_cast<const Derived*>(this); }
	private:
		const RHIBufferDesc& Imp_Get_Desc(void) const { LOG_ERROR("No Imp"); return RHIBufferDesc{}; }
		GPUVirtualAddress Imp_Get_GPUVirtualAddress(void) const { LOG_ERROR("No Imp"); return 0; }
	
	};

}