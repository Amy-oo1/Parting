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
	PARTING_EXPORT enum class RHIStencilOp : Uint8{
		Keep = 1,
		Zero = 2,
		Replace = 3,
		IncrementAndClamp = 4,
		DecrementAndClamp = 5,
		Invert = 6,
		IncrementAndWrap = 7,
		DecrementAndWrap = 8
	};

	PARTING_EXPORT enum class RHIComparisonFunc : Uint8{
		Never = 1,
		Less = 2,
		Equal = 3,
		LessOrEqual = 4,
		Greater = 5,
		NotEqual = 6,
		GreaterOrEqual = 7,
		Always = 8
	};

	PARTING_EXPORT struct RHIDepthStencilState final {
		struct RHIStencilOpDesc final {
			RHIStencilOp FailOp{ RHIStencilOp::Keep };
			RHIStencilOp DepthFailOp{ RHIStencilOp::Keep };
			RHIStencilOp PassOp{ RHIStencilOp::Keep };
			RHIComparisonFunc StencilFunc{ RHIComparisonFunc::Always };
		};

		bool				DepthTestEnable{ true };//NOTE :Default value not same with gapi 
		bool				DepthWriteEnable{ true };//NOTE :Default value not same with gapi 
		RHIComparisonFunc	DepthFunc{ RHIComparisonFunc::Less };
		bool				StencilEnable{ false };
		Uint8				StencilReadMask{ 0xff };
		Uint8				StencilWriteMask{ 0xff };
		Uint8				StencilRefValue{ 0 };
		bool				DynamicStencilRef{ false };
		RHIStencilOpDesc	FrontFaceStencil{};
		RHIStencilOpDesc	BackFaceStencil{};
	};

	PARTING_EXPORT class RHIDepthStencilStateBuilder final {
	public:
		STDNODISCARD constexpr RHIDepthStencilStateBuilder& Reset(void) { this->m_DepthStencilState = RHIDepthStencilState{}; return *this; }

		STDNODISCARD constexpr RHIDepthStencilStateBuilder& Set_DepthTestEnable(bool DepthTestEnable) { this->m_DepthStencilState.DepthTestEnable = DepthTestEnable; return *this; }
		STDNODISCARD constexpr RHIDepthStencilStateBuilder& Set_DepthWriteEnable(bool DepthWriteEnable) { this->m_DepthStencilState.DepthWriteEnable = DepthWriteEnable; return *this; }
		STDNODISCARD constexpr RHIDepthStencilStateBuilder& Set_DepthFunc(RHIComparisonFunc DepthFunc) { this->m_DepthStencilState.DepthFunc = DepthFunc; return *this; }
		STDNODISCARD constexpr RHIDepthStencilStateBuilder& Set_StencilEnable(bool StencilEnable) { this->m_DepthStencilState.StencilEnable = StencilEnable; return *this; }
		STDNODISCARD constexpr RHIDepthStencilStateBuilder& Set_StencilReadMask(Uint8 StencilReadMask) { this->m_DepthStencilState.StencilReadMask = StencilReadMask; return *this; }
		STDNODISCARD constexpr RHIDepthStencilStateBuilder& Set_StencilWriteMask(Uint8 StencilWriteMask) { this->m_DepthStencilState.StencilWriteMask = StencilWriteMask; return *this; }
		STDNODISCARD constexpr RHIDepthStencilStateBuilder& Set_StencilRefValue(Uint8 StencilRefValue) { this->m_DepthStencilState.StencilRefValue = StencilRefValue; return *this; }
		STDNODISCARD constexpr RHIDepthStencilStateBuilder& Set_DynamicStencilRef(bool DynamicStencilRef) { this->m_DepthStencilState.DynamicStencilRef = DynamicStencilRef; return *this; }
		STDNODISCARD constexpr RHIDepthStencilStateBuilder& Set_FrontFaceStencil(RHIDepthStencilState::RHIStencilOpDesc FrontFaceStencil) { this->m_DepthStencilState.FrontFaceStencil = FrontFaceStencil; return *this; }
		STDNODISCARD constexpr RHIDepthStencilStateBuilder& Set_BackFaceStencil(RHIDepthStencilState::RHIStencilOpDesc BackFaceStencil) { this->m_DepthStencilState.BackFaceStencil = BackFaceStencil; return *this; }

		STDNODISCARD constexpr RHIDepthStencilStateBuilder& Set_FrontFaceStencil(RHIStencilOp FailOp, RHIStencilOp DepthFailOp, RHIStencilOp PassOp, RHIComparisonFunc StencilFunc) {
			this->m_DepthStencilState.FrontFaceStencil.FailOp = FailOp;
			this->m_DepthStencilState.FrontFaceStencil.DepthFailOp = DepthFailOp;
			this->m_DepthStencilState.FrontFaceStencil.PassOp = PassOp;
			this->m_DepthStencilState.FrontFaceStencil.StencilFunc = StencilFunc;
			return *this;
		}
		STDNODISCARD constexpr RHIDepthStencilStateBuilder& Set_BackFaceStencil(RHIStencilOp FailOp, RHIStencilOp DepthFailOp, RHIStencilOp PassOp, RHIComparisonFunc StencilFunc) {
			this->m_DepthStencilState.BackFaceStencil.FailOp = FailOp;
			this->m_DepthStencilState.BackFaceStencil.DepthFailOp = DepthFailOp;
			this->m_DepthStencilState.BackFaceStencil.PassOp = PassOp;
			this->m_DepthStencilState.BackFaceStencil.StencilFunc = StencilFunc;
			return *this;
		}

		STDNODISCARD constexpr const RHIDepthStencilState& Build(void) { return this->m_DepthStencilState; }
	private:
		RHIDepthStencilState m_DepthStencilState{};
	};

}
