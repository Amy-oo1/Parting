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
	PARTING_EXPORT enum class RHIRasterFillMode : Uint8{
		Solid,
		Wireframe,

		// Vulkan names
		Fill = Solid,
		Line = Wireframe
	};

	PARTING_EXPORT enum class RHIRasterCullMode : Uint8{
		Back,
		Front,
		None
	};

	PARTING_EXPORT struct RHIRasterState final{
		RHIRasterFillMode FillMode{ RHIRasterFillMode::Solid };
		RHIRasterCullMode CullMode{ RHIRasterCullMode::Back };
		bool FrontCounterClockwise{ false };
		bool DepthClipEnable{ false };
		bool ScissorEnable{ false };
		bool MultisampleEnable{ false };
		bool AntialiasedLineEnable{ false };
		int DepthBias{ 0 };
		float DepthBiasClamp{ 0.f };
		float SlopeScaledDepthBias{ 0.f };

		// Extended rasterizer state supported by Maxwell
		// In D3D11, use NvAPI_D3D11_CreateRasterizerState to create such rasterizer state.
		Uint8 ForcedSampleCount{ 0 };
		bool ProgrammableSamplePositionsEnable{ false };
		bool ConservativeRasterEnable{ false };
		bool QuadFillEnable{ false };
		Array<char, 16> SamplePositionsX{};
		Array<char, 16> SamplePositionsY{};
	};

	PARTING_EXPORT class RHIRasterStateBuilder final {
	public:
		STDNODISCARD constexpr RHIRasterStateBuilder& Set_FillMode(RHIRasterFillMode RasterFillMode) { this->m_RasterState.FillMode = RasterFillMode; return *this; }
		STDNODISCARD constexpr RHIRasterStateBuilder& Set_CullMode(RHIRasterCullMode RasterCullMode) { this->m_RasterState.CullMode = RasterCullMode; return *this; }
		STDNODISCARD constexpr RHIRasterStateBuilder& Set_FrontCounterClockwise(bool FrontCounterClockwise) { this->m_RasterState.FrontCounterClockwise = FrontCounterClockwise; return *this; }
		STDNODISCARD constexpr RHIRasterStateBuilder& Set_DepthClipEnable(bool DepthClipEnable) { this->m_RasterState.DepthClipEnable = DepthClipEnable; return *this; }
		STDNODISCARD constexpr RHIRasterStateBuilder& Set_ScissorEnable(bool ScissorEnable) { this->m_RasterState.ScissorEnable = ScissorEnable; return *this; }
		STDNODISCARD constexpr RHIRasterStateBuilder& Set_MultisampleEnable(bool MultisampleEnable) { this->m_RasterState.MultisampleEnable = MultisampleEnable; return *this; }
		STDNODISCARD constexpr RHIRasterStateBuilder& Set_AntialiasedLineEnable(bool AntialiasedLineEnable) { this->m_RasterState.AntialiasedLineEnable = AntialiasedLineEnable; return *this; }
		STDNODISCARD constexpr RHIRasterStateBuilder& Set_DepthBias(int DepthBias) { this->m_RasterState.DepthBias = DepthBias; return *this; }
		STDNODISCARD constexpr RHIRasterStateBuilder& Set_DepthBiasClamp(float DepthBiasClamp) { this->m_RasterState.DepthBiasClamp = DepthBiasClamp; return *this; }
		STDNODISCARD constexpr RHIRasterStateBuilder& Set_SlopeScaledDepthBias(float SlopeScaledDepthBias) { this->m_RasterState.SlopeScaledDepthBias = SlopeScaledDepthBias; return *this; }
		STDNODISCARD constexpr RHIRasterStateBuilder& Set_ForcedSampleCount(Uint8 ForcedSampleCount) { this->m_RasterState.ForcedSampleCount = ForcedSampleCount; return *this; }
		STDNODISCARD constexpr RHIRasterStateBuilder& Set_ProgrammableSamplePositionsEnable(bool ProgrammableSamplePositionsEnable) { this->m_RasterState.ProgrammableSamplePositionsEnable = ProgrammableSamplePositionsEnable; return *this; }
		STDNODISCARD constexpr RHIRasterStateBuilder& Set_ConservativeRasterEnable(bool ConservativeRasterEnable) { this->m_RasterState.ConservativeRasterEnable = ConservativeRasterEnable; return *this; }
		STDNODISCARD constexpr RHIRasterStateBuilder& Set_QuadFillEnable(bool QuadFillEnable) { this->m_RasterState.QuadFillEnable = QuadFillEnable; return *this; }
		STDNODISCARD constexpr RHIRasterStateBuilder& Set_SamplePositionsX(const Array<char, 16>& SamplePositionsX) { this->m_RasterState.SamplePositionsX = SamplePositionsX; return *this; }
		STDNODISCARD constexpr RHIRasterStateBuilder& Set_SamplePositionsY(const Array<char, 16>& SamplePositionsY) { this->m_RasterState.SamplePositionsY = SamplePositionsY; return *this; }

		STDNODISCARD constexpr RHIRasterStateBuilder& Set_SamplePositions(const char* x, const char* y, Uint32 Count) {
			ASSERT(x != nullptr);
			ASSERT(y != nullptr);
			ASSERT(Count <= 16);

			for (Uint32 Index = 0; Index < Count; ++Index){
				this->m_RasterState.SamplePositionsX[Index] = x[Index];
				this->m_RasterState.SamplePositionsY[Index] = y[Index];
			}

			return *this;
		}

		STDNODISCARD constexpr RHIRasterState Build() const { return this->m_RasterState; }
	private:
		RHIRasterState m_RasterState{};
	};

}