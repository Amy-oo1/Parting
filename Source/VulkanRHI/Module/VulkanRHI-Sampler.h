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
#include "VulkanWrapper.h"

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Algorithm/Module/Algorithm.h"
#include "Core/Container/Module/Container.h"
#include "Core/String/Module/String.h"
#include "Core/Color/Module/Color.h"
#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI.h"
#include "RHI/Module/StateTracking.h"

#include "VulkanRHI/Module/VulkanRHI-Traits.h"
#include "VulkanRHI/Module/VulkanRHI-Common.h"
#endif // PARTING_MODULE_BUILD

namespace RHI::Vulkan {
	class Sampler final : public RHISampler<Sampler> {
		friend class RHIResource<Sampler>;
		friend class RHISampler<Sampler>;

		friend class CommandList;
		friend class Device;
	public:
		Sampler(const Context& context, const RHISamplerDesc desc) :
			RHISampler<Sampler>{},
			m_Context{ context },
			m_Desc{ desc } {
		}

		~Sampler(void);

	private:
		const Context& m_Context;

		RHISamplerDesc m_Desc;

		vk::SamplerCreateInfo m_SamplerInfo;
		vk::Sampler m_Sampler;

	private:
		RHIObject Imp_GetNativeObject(RHIObjectType objectType)const noexcept;

		const RHISamplerDesc& Imp_Get_Desc(void)const { return this->m_Desc; };
	};

	//Misc

	constexpr vk::SamplerAddressMode ConvertSamplerAddressMode(RHISamplerAddressMode mode) {
		switch (mode) {
			using enum RHISamplerAddressMode;
		case ClampToEdge:return vk::SamplerAddressMode::eClampToEdge;
		case Repeat:return vk::SamplerAddressMode::eRepeat;
		case ClampToBorder:return vk::SamplerAddressMode::eClampToBorder;
		case MirroredRepeat:return vk::SamplerAddressMode::eMirroredRepeat;
		case MirrorClampToEdge:return vk::SamplerAddressMode::eMirrorClampToEdge;
		default:ASSERT(false) return {};
		}
	}

	vk::BorderColor PickSamplerBorderColor(Color borderColor) {
		if (borderColor.R == 0.f && borderColor.G == 0.f && borderColor.B == 0.f) {
			if (borderColor.A == 0.f)
				return vk::BorderColor::eFloatTransparentBlack;

			if (borderColor.A == 1.f)
				return vk::BorderColor::eFloatOpaqueBlack;
		}

		if (borderColor.R == 1.f && borderColor.G == 1.f && borderColor.B == 1.f)
			if (borderColor.A == 1.f)
				return vk::BorderColor::eFloatOpaqueWhite;

		ASSERT(false);//TODO :EXR
		return vk::BorderColor::eFloatOpaqueBlack;
	}

	//Src

	Sampler::~Sampler(void) {
		this->m_Context.Device.destroySampler(this->m_Sampler);
	}

	//Imp

	RHIObject Sampler::Imp_GetNativeObject(RHIObjectType objectType)const noexcept {
		switch (objectType) {
			using enum RHIObjectType;
		case VK_Sampler:return RHIObject{ .Pointer{ this->m_Sampler } };
		default:return RHIObject{};
		}
	}
}