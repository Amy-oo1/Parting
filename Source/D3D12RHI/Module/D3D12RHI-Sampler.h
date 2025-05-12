#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"

PARTING_SUBMODULE(D3D12RHI, Texture)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Algorithm;
PARTING_IMPORT Container;
PARTING_IMPORT String;
PARTING_IMPORT Color;
PARTING_IMPORT Logger;

PARTING_IMPORT RHI;

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

#include "RHI/Module/RHI.h"
#include "RHI/Module/StateTracking.h"

#include "D3D12RHI/Module/D3D12RHI-Traits.h"
#include "D3D12RHI/Module/D3D12RHI-Common.h"
#endif // PARTING_MODULE_BUILD

namespace RHI::D3D12 {

	Array<D3D12_FILTER_REDUCTION_TYPE, Tounderlying(RHISamplerReductionType::COUNT)> g_D3D12ReductionType{
		D3D12_FILTER_REDUCTION_TYPE::D3D12_FILTER_REDUCTION_TYPE_STANDARD,
		D3D12_FILTER_REDUCTION_TYPE::D3D12_FILTER_REDUCTION_TYPE_COMPARISON,
		D3D12_FILTER_REDUCTION_TYPE::D3D12_FILTER_REDUCTION_TYPE_MINIMUM,
		D3D12_FILTER_REDUCTION_TYPE::D3D12_FILTER_REDUCTION_TYPE_MAXIMUM
	};

	STDNODISCARD constexpr auto Get_D3D12ReductionType(RHISamplerReductionType ReductionType) noexcept {
		return g_D3D12ReductionType[Tounderlying(ReductionType)];
	}

	Array<D3D12_TEXTURE_ADDRESS_MODE, Tounderlying(RHISamplerAddressMode::COUNT)> g_D3D12AddressMode{
		D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_MIRROR,
		D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE
	};

	STDNODISCARD constexpr auto Get_D3D12AddressMode(RHISamplerAddressMode AddressMode) noexcept {
		return g_D3D12AddressMode[Tounderlying(AddressMode)];
	}




	PARTING_EXPORT class Sampler final :public RHISampler<Sampler> {
		friend class RHIResource<Sampler>;
		friend class RHISampler<Sampler>;
	public:
		Sampler(const Context& context, const RHISamplerDesc& desc);
		~Sampler(void) = default;

	public:
		void CreateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE handle) const { this->m_Context.Device->CreateSampler(&this->m_SamplerDesc, handle); }

	private:
		
	private:
		const Context& m_Context;
		const RHISamplerDesc m_Desc;
		D3D12_SAMPLER_DESC m_SamplerDesc{};

	private:
		RHIObject Imp_GetNativeObject(RHIObjectType)const noexcept { LOG_ERROR("Imp But Empty");  return RHIObject{}; };

		const RHISamplerDesc& Imp_Get_Desc(void)const { return this->m_Desc; };
	};



	inline Sampler::Sampler(const Context& context, const RHISamplerDesc& desc) :
		m_Context{ context },
		m_Desc{ desc } {

		const auto ReductionType{ Get_D3D12ReductionType(this->m_Desc.ReductionType) };

		if (this->m_Desc.MaxAnisotropy > 1.f)
			this->m_SamplerDesc.Filter = D3D12EncodeAnisotropicFilter(ReductionType);
		else
			this->m_SamplerDesc.Filter = D3D12EnocdeBasicFilter(
				this->m_Desc.MinFilter ? D3D12_FILTER_TYPE_LINEAR : D3D12_FILTER_TYPE_POINT,
				this->m_Desc.MagFilter ? D3D12_FILTER_TYPE_LINEAR : D3D12_FILTER_TYPE_POINT,
				this->m_Desc.MipFilter ? D3D12_FILTER_TYPE_LINEAR : D3D12_FILTER_TYPE_POINT,
				ReductionType
			);

		this->m_SamplerDesc.AddressU = Get_D3D12AddressMode(this->m_Desc.AddressModeU);
		this->m_SamplerDesc.AddressV = Get_D3D12AddressMode(this->m_Desc.AddressModeV);
		this->m_SamplerDesc.AddressW = Get_D3D12AddressMode(this->m_Desc.AddressModeW);

		this->m_SamplerDesc.MipLODBias = this->m_Desc.MiBias;
		this->m_SamplerDesc.MaxAnisotropy = Math::Max(static_cast<UINT32>(this->m_Desc.MaxAnisotropy), 1u);
		this->m_SamplerDesc.ComparisonFunc = this->m_Desc.ReductionType == RHISamplerReductionType::Comparison ? D3D12_COMPARISON_FUNC_LESS : D3D12_COMPARISON_FUNC_NEVER;

		this->m_SamplerDesc.BorderColor[0] = this->m_Desc.BorderColor.Get_R();
		this->m_SamplerDesc.BorderColor[1] = this->m_Desc.BorderColor.Get_G();
		this->m_SamplerDesc.BorderColor[2] = this->m_Desc.BorderColor.Get_B();
		this->m_SamplerDesc.BorderColor[3] = this->m_Desc.BorderColor.Get_A();

		this->m_SamplerDesc.MinLOD = 0.f;
		this->m_SamplerDesc.MaxLOD = Max_Float;
	}


}