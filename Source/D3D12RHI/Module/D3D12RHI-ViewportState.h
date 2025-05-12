#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"

PARTING_SUBMODULE(D3D12RHI, ViewportState)

PARTING_IMPORT DirectX12Wrapper;

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Algorithm;
PARTING_IMPORT Container;

PARTING_IMPORT RHI;

#else
#pragma once

#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
//Global
#include "D3D12RHI/Module/DirectX12Wrapper.h"

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Algorithm/Module/Algorithm.h"
#include "Core/Container/Module/Container.h"

#include "RHI/Module/RHI.h"

#include "D3D12RHI/Module/D3D12RHI-Traits.h"
#include "D3D12RHI/Module/D3D12RHI-Common.h"

#endif // PARTING_MODULE_BUILD

namespace RHI::D3D12 {

	struct ViewportState final {
		Array<D3D12_VIEWPORT, g_MaxViewportCount> Viewports{};
		Uint32 ViewportCount{ 0 };

		Array<D3D12_RECT, g_MaxViewportCount> Scissors{};
		Uint32 ScissorCount{ 0 };

		STDNODISCARD static ViewportState Convert(const RHIRasterState& rasterState, const RHIFrameBufferInfo<D3D12Tag>& framebufferInfo, const RHIViewportState& vpState) {
			ViewportState Re{};

			for (Uint32 Index = 0; Index < vpState.ViewportCount; ++Index)
				Re.Viewports[Re.ViewportCount++] = D3D12_VIEWPORT{
					.TopLeftX{ vpState.Viewports[Index].MinX },
					.TopLeftY{ vpState.Viewports[Index].MinY },
					.Width{ vpState.Viewports[Index].Width() },
					.Height{ vpState.Viewports[Index].Height() },
					.MinDepth{ vpState.Viewports[Index].MinZ },
					.MaxDepth{ vpState.Viewports[Index].MaxZ }
			};

			for (Uint32 Index = 0; Index < vpState.ScissorCount; ++Index) {
				if (rasterState.ScissorEnable) {
					Re.Scissors[Re.ScissorCount++] = D3D12_RECT{
						.left{ static_cast<LONG>(vpState.ScissorRects[Index].Offset.X) },
						.top{ static_cast<LONG>(vpState.ScissorRects[Index].Offset.Y) },
						.right{ static_cast<LONG>(vpState.ScissorRects[Index].Offset.X + vpState.ScissorRects[Index].Extent.Width) },
						.bottom{ static_cast<LONG>(vpState.ScissorRects[Index].Offset.Y + vpState.ScissorRects[Index].Extent.Height) }
					};
				}
				else {
					Re.Scissors[Re.ScissorCount++] = D3D12_RECT{
						.left{ static_cast<LONG>(vpState.Viewports[Index].MinX) },
						.top{ static_cast<LONG>(vpState.Viewports[Index].MinY) },
						.right{ static_cast<LONG>(vpState.Viewports[Index].MaxX) },
						.bottom{ static_cast<LONG>(vpState.Viewports[Index].MaxY) }
					};

					if (framebufferInfo.Width > 0) {
						Re.Scissors[Index].left = Math::Max(Re.Scissors[Index].left, static_cast<LONG>(0));
						Re.Scissors[Index].top = Math::Max(Re.Scissors[Index].top, static_cast<LONG>(0));
						Re.Scissors[Index].right = Math::Min(Re.Scissors[Index].right, static_cast<LONG>(framebufferInfo.Width));
						Re.Scissors[Index].bottom = Math::Min(Re.Scissors[Index].bottom, static_cast<LONG>(framebufferInfo.Height));
					}
				}
			}

			return Re;
		}
	};



}