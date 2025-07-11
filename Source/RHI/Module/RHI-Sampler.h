#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"


PARTING_SUBMODULE(RHI, Sampler)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Algorithm;
PARTING_IMPORT Container;
PARTING_IMPORT String;
PARTING_IMPORT Color;
PARTING_IMPORT Logger;

PARTING_SUBMODE_IMPORT(Resource)
PARTING_SUBMODE_IMPORT(Traits)
PARTING_SUBMODE_IMPORT(Common)

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
#include "Core/Color/Module/Color.h"
#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI-Resource.h"
#include "RHI/Module/RHI-Traits.h"
#include "RHI/Module/RHI-Common.h"

#endif // PARTING_MODULE_BUILD

namespace RHI {
	PARTING_EXPORT enum class RHISamplerAddressMode :Uint8 {
		Clamp,
		Wrap,
		Border,
		Mirror,
		MirrorOnce,

		COUNT,

		// Vulkan names
		ClampToEdge = Clamp,
		Repeat = Wrap,
		ClampToBorder = Border,
		MirroredRepeat = Mirror,
		MirrorClampToEdge = MirrorOnce
	};

	PARTING_EXPORT enum class RHISamplerReductionType : Uint8{
		Standard,
		Comparison,
		Minimum,
		Maximum,

	    COUNT
	};

	PARTING_EXPORT struct RHISamplerDesc final {
		Color BorderColor{ 1.0f };
		float MaxAnisotropy{ 1.0f };
		float MipBias{ 0.0f };

		bool MinFilter{ true };
		bool MagFilter{ true };
		bool MipFilter{ true };

		RHISamplerAddressMode AddressModeU{ RHISamplerAddressMode::Clamp };
		RHISamplerAddressMode AddressModeV{ RHISamplerAddressMode::Clamp };
		RHISamplerAddressMode AddressModeW{ RHISamplerAddressMode::Clamp };

		RHISamplerReductionType ReductionType{ RHISamplerReductionType::Standard };
	};

	PARTING_EXPORT class RHISamplerDescBuilder final {
	public:
		constexpr RHISamplerDescBuilder& Reset(void) { this->m_Desc = RHISamplerDesc{}; return *this; }

		constexpr RHISamplerDescBuilder& Set_BorderColor(const Color& color) { this->m_Desc.BorderColor = color; return *this; }
		constexpr RHISamplerDescBuilder& Set_MaxAnisotropy(float maxAnisotropy) { this->m_Desc.MaxAnisotropy = maxAnisotropy; return *this; }
		constexpr RHISamplerDescBuilder& Set_MipBias(float miBias) { this->m_Desc.MipBias = miBias; return *this; }
		constexpr RHISamplerDescBuilder& Set_MinFilter(bool minFilter) { this->m_Desc.MinFilter = minFilter; return *this; }
		constexpr RHISamplerDescBuilder& Set_MagFilter(bool magFilter) { this->m_Desc.MagFilter = magFilter; return *this; }
		constexpr RHISamplerDescBuilder& Set_MipFilter(bool mipFilter) { this->m_Desc.MipFilter = mipFilter; return *this; }
		constexpr RHISamplerDescBuilder& Set_AddressModeU(RHISamplerAddressMode addressMode) { this->m_Desc.AddressModeU = addressMode; return *this; }
		constexpr RHISamplerDescBuilder& Set_AddressModeV(RHISamplerAddressMode addressMode) { this->m_Desc.AddressModeV = addressMode; return *this; }
		constexpr RHISamplerDescBuilder& Set_AddressModeW(RHISamplerAddressMode addressMode) { this->m_Desc.AddressModeW = addressMode; return *this; }
		constexpr RHISamplerDescBuilder& Set_ReductionType(RHISamplerReductionType reductionType) { this->m_Desc.ReductionType = reductionType; return *this; }

		constexpr RHISamplerDescBuilder& Set_Filter(bool minFilter, bool magFilter, bool mipFilter) {
			this->m_Desc.MinFilter = minFilter;
			this->m_Desc.MagFilter = magFilter;
			this->m_Desc.MipFilter = mipFilter;
			return *this;
		}

		constexpr RHISamplerDescBuilder& Set_AllFilter(bool Filter) {
			this->m_Desc.MinFilter = Filter;
			this->m_Desc.MagFilter = Filter;
			this->m_Desc.MipFilter = Filter;

			return *this;
		}

		constexpr RHISamplerDescBuilder& Set_AddressModeUVW(RHISamplerAddressMode addressMode) {
			this->m_Desc.AddressModeU = addressMode;
			this->m_Desc.AddressModeV = addressMode;
			this->m_Desc.AddressModeW = addressMode;

			return *this;
		}

		STDNODISCARD constexpr const RHISamplerDesc& Build(void) { return this->m_Desc; }

	private:
		RHISamplerDesc m_Desc{};

	};

	PARTING_EXPORT template<typename Derived>
	class RHISampler :public RHIResource<Derived> {
		friend class RHIResource<Derived>;
	protected:
		RHISampler(void) = default;
		PARTING_VIRTUAL ~RHISampler(void) = default;

	public:
		STDNODISCARD const RHISamplerDesc& Get_Desc(void)const { return this->Get_Derived()->Imp_Get_Desc(); }
	
	private:
		STDNODISCARD Derived* Get_Derived(void)noexcept { return static_cast<Derived*>(this); }
		STDNODISCARD const Derived* Get_Derived(void)const noexcept { return static_cast<const Derived*>(this); }
	private:
		const RHISamplerDesc& Imp_Get_Desc(void)const { LOG_ERROR("No Imp");  return RHISamplerDesc{}; }

	};

}