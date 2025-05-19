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

namespace RHI::D3D12 {

	D3D_PRIMITIVE_TOPOLOGY ConvertPrimitiveType(RHIPrimitiveType pt, Uint32 controlPoints) {
		switch (pt) {
			using enum RHIPrimitiveType;
		case PointList:return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		case LineList:return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		case LineStrip:return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
		case TriangleList:return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case TriangleStrip:return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		case TriangleFan:ASSERT(false) return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
		case TriangleListWithAdjacency:return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
		case TriangleStripWithAdjacency:return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
		case PatchList:
			if (controlPoints == 0 || controlPoints > 32) {
				ASSERT(false);
				return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
			}
			return static_cast<D3D_PRIMITIVE_TOPOLOGY>(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST + (controlPoints - 1));
		default:return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
		}
	}

	D3D12_PRIMITIVE_TOPOLOGY_TYPE ConvertPrimitiveType(RHIPrimitiveType pt) {
		switch (pt) {
			using enum RHIPrimitiveType;
		case PointList:return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		case LineList:return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		case TriangleList:case TriangleStrip:case TriangleFan:case TriangleListWithAdjacency:case TriangleStripWithAdjacency:
			return  D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		case PatchList:return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
		default: ASSERT(false); return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
		}
	}


	D3D12_RASTERIZER_DESC TranslateRasterizerState(const RHIRasterState& inState) {
		D3D12_RASTERIZER_DESC outState{};
		switch (inState.FillMode) {
			using enum RHIRasterFillMode;
		case Solid:outState.FillMode = D3D12_FILL_MODE_SOLID; break;
		case Wireframe:outState.FillMode = D3D12_FILL_MODE_WIREFRAME; break;
		default:ASSERT(false); break;
		}

		switch (inState.CullMode) {
			using enum RHIRasterCullMode;
		case Back:outState.CullMode = D3D12_CULL_MODE_BACK; break;
		case Front:outState.CullMode = D3D12_CULL_MODE_FRONT; break;
		case None:outState.CullMode = D3D12_CULL_MODE_NONE; break;
		default:ASSERT(false); break;
		}

		outState.FrontCounterClockwise = inState.FrontCounterClockwise;
		outState.DepthBias = inState.DepthBias;
		outState.DepthBiasClamp = inState.DepthBiasClamp;
		outState.SlopeScaledDepthBias = inState.SlopeScaledDepthBias;
		outState.DepthClipEnable = inState.DepthClipEnable;
		outState.MultisampleEnable = inState.MultisampleEnable;
		outState.AntialiasedLineEnable = inState.AntialiasedLineEnable;
		outState.ConservativeRaster = inState.ConservativeRasterEnable ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		outState.ForcedSampleCount = inState.ForcedSampleCount;

		return outState;
	}



}