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
#include "VulkanWrapper.h"

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Algorithm/Module/Algorithm.h"
#include "Core/Container/Module/Container.h"

#include "RHI/Module/RHI.h"

#include "VulkanRHI/Module/VulkanRHI-Traits.h"
#include "VulkanRHI/Module/VulkanRHI-Common.h"

#endif // PARTING_MODULE_BUILD

namespace RHI::Vulkan {
	// requires VK_KHR_maintenance1 which allows negative-height to indicate an inverted coord space to match DX
	vk::Viewport VKViewportWithDXCoords(const RHIViewport& v) {
		return vk::Viewport{}
			.setX(v.MinX)
			.setY(v.MaxY) // Vulkan's origin is bottom-left, so we need to invert the Y coordinate
			.setWidth(v.MaxX - v.MinX)
			.setHeight(-(v.MaxY - v.MinY)) // Negative height to indicate inverted Y
			.setMinDepth(v.MinZ)
			.setMaxDepth(v.MaxZ);
		//return vk::Viewport(v.minX, v.maxY, v.maxX - v.minX, -(v.maxY - v.minY), v.minZ, v.maxZ);
	}
}