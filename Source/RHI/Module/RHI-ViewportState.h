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
#include "Core/Algorithm/Module/Algorithm.h"
#include "Core/Logger/Module/Logger.h"
#include "RHI/Module/RHI-Traits.h"
#include "RHI/Module/RHI-Common.h"
#include "RHI/Module/RHI-Resource.h"
#include "RHI/Module/RHI-Format.h"
#include "RHI/Module/RHI-Heap.h"

#endif // PARTING_MODULE_BUILD#pragma once

namespace RHI {
	PARTING_EXPORT struct RHIViewport final {
		float MinX{ 0.f }, MaxX{ 0.f };
		float MinY{ 0.f }, MaxY{ 0.f };
		float MinZ{ 0.f }, MaxZ{ 1.f };

		STDNODISCARD static constexpr auto Build(float width, float height) {
			return RHIViewport{
				.MinX{ 0.f }, .MaxX{ width },
				.MinY{ 0.f }, .MaxY{ height },
				.MinZ{ 0.f }, .MaxZ{ 1.f }
			};
		}

		STDNODISCARD constexpr float Width(void)const { return this->MaxX - this->MinX; }
		STDNODISCARD constexpr float Height(void)const { return this->MaxY - this->MinY; }



		STDNODISCARD constexpr bool operator==(const RHIViewport&)const noexcept = default;
		STDNODISCARD constexpr bool operator!=(const RHIViewport&)const noexcept = default;
	};

	PARTING_EXPORT RHIRect2D BuildScissorRect(const RHIViewport& viewport) {
		return RHIRect2D{
			.Offset{ static_cast<Uint32>(Math::Floor(viewport.MinX)), static_cast<Uint32>(Math::Floor(viewport.MinY)) },
			.Extent{ static_cast<Uint32>(Math::Ceil(viewport.Width())), static_cast<Uint32>(Math::Ceil(viewport.Height())) }
		};
	}

	PARTING_EXPORT struct RHIViewportState final {
		//These are in pixels
		// note: you can only set each of these either in the PSO or per draw call in DrawArguments
		// it is not legal to have the same state set in both the PSO and DrawArguments
		// leaving these vectors empty means no state is set
		Array<RHIViewport, g_MaxViewportCount> Viewports;
		Array<RHIRect2D, g_MaxViewportCount> ScissorRects;

		Uint16 ViewportCount{ 0 };
		Uint16 ScissorCount{ 0 };

		STDNODISCARD constexpr bool operator==(const RHIViewportState& other)const noexcept {
			return
				ArrayEqual(this->Viewports, this->ViewportCount, other.Viewports, other.ViewportCount) &&
				ArrayEqual(this->ScissorRects, this->ScissorCount, other.ScissorRects, other.ScissorCount);
		}
		STDNODISCARD constexpr bool operator!=(const RHIViewportState& other)const noexcept { return !(*this == other); }
	};

	PARTING_EXPORT class RHIViewportStateBuilder final {
	public:
		RHIViewportStateBuilder& AddViewport(const RHIViewport& Viewport) {
			ASSERT(this->m_ViewportState.ViewportCount < g_MaxViewportCount);

			this->m_ViewportState.Viewports[this->m_ViewportState.ViewportCount++] = Viewport;

			return *this;
		}
		RHIViewportStateBuilder& AddScissorRect(const RHIRect2D& ScissorRect) {
			ASSERT(this->m_ViewportState.ScissorCount < g_MaxViewportCount);
			this->m_ViewportState.ScissorRects[this->m_ViewportState.ScissorCount++] = ScissorRect;

			return *this;
		}

		RHIViewportStateBuilder& AddViewportAndScissorRect(const RHIViewport& Viewport) {
			RHIRect2D ScissorRect{
				.Offset{ static_cast<Uint32>(Math::Floor(Viewport.MinX)), static_cast<Uint32>(Math::Floor(Viewport.MinY)) },
				.Extent{ static_cast<Uint32>(Math::Ceil(Viewport.Width())), static_cast<Uint32>(Math::Ceil(Viewport.Height())) }
			};

			this->AddViewport(Viewport);
			this->AddScissorRect(ScissorRect);

			return *this;
		}

		STDNODISCARD const RHIViewportState& Build(void)const {
			ASSERT(this->m_ViewportState.ViewportCount > 0);
			ASSERT(this->m_ViewportState.ScissorCount > 0);

			return this->m_ViewportState;
		}

	private:
		RHIViewportState m_ViewportState;
	};

}