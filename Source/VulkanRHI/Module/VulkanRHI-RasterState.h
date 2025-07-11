#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"

PARTING_SUBMODULE(D3D12RHI, RasterState)

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

#endif // PARTING_MODULE_BUILD

namespace RHI::Vulkan {
	vk::PrimitiveTopology ConvertPrimitiveTopology(RHIPrimitiveType topology) {
		switch (topology) {
			using enum RHIPrimitiveType;
		case PointList:return vk::PrimitiveTopology::ePointList;
		case LineList:return vk::PrimitiveTopology::eLineList;
		case LineStrip:return vk::PrimitiveTopology::eLineStrip;
		case TriangleList:return vk::PrimitiveTopology::eTriangleList;
		case TriangleStrip:return vk::PrimitiveTopology::eTriangleStrip;
		case TriangleFan:return vk::PrimitiveTopology::eTriangleFan;
		case TriangleListWithAdjacency:return vk::PrimitiveTopology::eTriangleListWithAdjacency;
		case TriangleStripWithAdjacency:return vk::PrimitiveTopology::eTriangleStripWithAdjacency;
		case PatchList:return vk::PrimitiveTopology::ePatchList;
		default:ASSERT(false); return vk::PrimitiveTopology::eTriangleList;
		}
	}

	vk::PolygonMode ConvertFillMode(RHIRasterFillMode mode) {
		switch (mode) {
			using enum RHIRasterFillMode;
		case Fill:return vk::PolygonMode::eFill;
		case Line:return vk::PolygonMode::eLine;
		default:ASSERT(false); return vk::PolygonMode::eFill;
		}
	}

	vk::CullModeFlagBits ConvertCullMode(RHIRasterCullMode mode) {
		switch (mode) {
			using enum RHIRasterCullMode;
		case Back:return vk::CullModeFlagBits::eBack;
		case Front:return vk::CullModeFlagBits::eFront;
		case None:return vk::CullModeFlagBits::eNone;
		default:ASSERT(false); return vk::CullModeFlagBits::eNone;
		}
	}
}