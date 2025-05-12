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

PARTING_SUBMODE_IMPORT(Traits)
PARTING_SUBMODE_IMPORT(Common)
PARTING_SUBMODE_IMPORT(Resource)
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
#include "Core/String/Module/String.h"
#include "Core/Color/Module/Color.h"
#include "Core/Container/Module/Container.h"
#include "Core/Logger/Module/Logger.h"
#include "RHI/Module/RHI-Traits.h"
#include "RHI/Module/RHI-Common.h"
#include "RHI/Module/RHI-Resource.h"
#include "RHI/Module/RHI-Format.h"
#include "RHI/Module/RHI-Heap.h"

#endif // PARTING_MODULE_BUILD#pragma once

namespace RHI {
	PARTING_EXPORT enum class RHIBlendFactor : Uint32 {
		Zero = 1,
		One = 2,
		SrcColor = 3,
		InvSrcColor = 4,
		SrcAlpha = 5,
		InvSrcAlpha = 6,
		DstAlpha = 7,
		InvDstAlpha = 8,
		DstColor = 9,
		InvDstColor = 10,
		SrcAlphaSaturate = 11,
		ConstantColor = 14,
		InvConstantColor = 15,
		Src1Color = 16,
		InvSrc1Color = 17,
		Src1Alpha = 18,
		InvSrc1Alpha = 19,

		// Vulkan names
		OneMinusSrcColor = InvSrcColor,
		OneMinusSrcAlpha = InvSrcAlpha,
		OneMinusDstAlpha = InvDstAlpha,
		OneMinusDstColor = InvDstColor,
		OneMinusConstantColor = InvConstantColor,
		OneMinusSrc1Color = InvSrc1Color,
		OneMinusSrc1Alpha = InvSrc1Alpha,
	};

	PARTING_EXPORT enum class RHIBlendOp : Uint8 {
		Add = 1,
		Subrtact = 2,
		ReverseSubtract = 3,
		Min = 4,
		Max = 5
	};

	PARTING_EXPORT enum class RHIColorMask : Uint8 {
		// These values are equal to their counterparts in DX11, DX12, and Vulkan.
		Red = 1,
		Green = 2,
		Blue = 4,
		Alpha = 8,
		All = 0xF
	};
	EXPORT_ENUM_CLASS_OPERATORS(RHIColorMask);

	PARTING_EXPORT struct RHIBlendState final {
		struct RHIRenderTarget final {
			bool			BlendEnable{ false };
			RHIBlendFactor	SrcBlend{ RHIBlendFactor::One };
			RHIBlendFactor	DestBlend{ RHIBlendFactor::Zero };
			RHIBlendOp		BlendOp{ RHIBlendOp::Add };
			RHIBlendFactor	SrcBlendAlpha{ RHIBlendFactor::One };
			RHIBlendFactor	DestBlendAlpha{ RHIBlendFactor::Zero };
			RHIBlendOp		BlendOpAlpha{ RHIBlendOp::Add };
			RHIColorMask	ColorWriteMask{ RHIColorMask::All };

			STDNODISCARD constexpr bool Is_UsesConstantColor(void)const {
				using enum RHIBlendFactor;
				return
					ConstantColor == this->SrcBlend || OneMinusConstantColor == this->SrcBlend ||
					ConstantColor == this->DestBlend || OneMinusConstantColor == this->DestBlend ||
					ConstantColor == this->SrcBlendAlpha || OneMinusConstantColor == this->SrcBlendAlpha ||
					ConstantColor == this->DestBlendAlpha || OneMinusConstantColor == this->DestBlendAlpha;
			}

			STDNODISCARD constexpr bool operator==(const RHIRenderTarget& other)const {
				return
					this->BlendEnable == other.BlendEnable &&
					this->SrcBlend == other.SrcBlend &&
					this->DestBlend == other.DestBlend &&
					this->BlendOp == other.BlendOp &&
					this->SrcBlendAlpha == other.SrcBlendAlpha &&
					this->DestBlendAlpha == other.DestBlendAlpha &&
					this->BlendOpAlpha == other.BlendOpAlpha &&
					this->ColorWriteMask == other.ColorWriteMask;
			}
			STDNODISCARD constexpr bool operator!=(const RHIRenderTarget& other)const { return !(*this == other); }
		};

		class RHIRenderTargetBuilder final {
		public:
			STDNODISCARD constexpr RHIRenderTargetBuilder& Reset(void) { this->m_RenderTarget = RHIRenderTarget{}; return *this; }

			STDNODISCARD constexpr RHIRenderTargetBuilder& Set_BlendEnable(bool enable)noexcept { this->m_RenderTarget.BlendEnable = enable; return *this; }
			STDNODISCARD constexpr RHIRenderTargetBuilder& Set_SrcBlend(RHIBlendFactor srcBlend)noexcept { this->m_RenderTarget.SrcBlend = srcBlend; return *this; }
			STDNODISCARD constexpr RHIRenderTargetBuilder& Set_DestBlend(RHIBlendFactor destBlend)noexcept { this->m_RenderTarget.DestBlend = destBlend; return *this; }
			STDNODISCARD constexpr RHIRenderTargetBuilder& Set_BlendOp(RHIBlendOp blendOp)noexcept { this->m_RenderTarget.BlendOp = blendOp; return *this; }
			STDNODISCARD constexpr RHIRenderTargetBuilder& Set_SrcBlendAlpha(RHIBlendFactor srcBlendAlpha)noexcept { this->m_RenderTarget.SrcBlendAlpha = srcBlendAlpha; return *this; }
			STDNODISCARD constexpr RHIRenderTargetBuilder& Set_DestBlendAlpha(RHIBlendFactor destBlendAlpha)noexcept { this->m_RenderTarget.DestBlendAlpha = destBlendAlpha; return *this; }
			STDNODISCARD constexpr RHIRenderTargetBuilder& Set_BlendOpAlpha(RHIBlendOp blendOpAlpha)noexcept { this->m_RenderTarget.BlendOpAlpha = blendOpAlpha; return *this; }
			STDNODISCARD constexpr RHIRenderTargetBuilder& Set_ColorWriteMask(RHIColorMask colorWriteMask)noexcept { this->m_RenderTarget.ColorWriteMask = colorWriteMask; return *this; }
			
			STDNODISCARD constexpr RHIRenderTarget Build(void)const noexcept { return this->m_RenderTarget; }
		private:
			RHIRenderTarget m_RenderTarget{};
		};

		Array<RHIRenderTarget, g_MaxRenderTargetCount> RenderTargets{};
		bool alphaToCoverageEnable{ false };

		STDNODISCARD constexpr bool Is_UsesConstantColor(Uint32 RenderTargetCount)const {
			for(Uint32 Index=0;Index<RenderTargetCount;++Index)
				if (this->RenderTargets[Index].Is_UsesConstantColor())
					return true;

			return false;
		}

		STDNODISCARD constexpr bool operator==(const RHIBlendState& other)const {
			if (this->alphaToCoverageEnable != other.alphaToCoverageEnable)
				return false;

			for (Uint32 Index = 0; Index < g_MaxRenderTargetCount; ++Index)
				if (this->RenderTargets[Index] != other.RenderTargets[Index])
					return false;

			return true;
		}

		STDNODISCARD constexpr bool operator!=(const RHIBlendState& other)const { return !(*this == other); }
	};
}