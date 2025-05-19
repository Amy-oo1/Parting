#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"

PARTING_SUBMODULE(D3D12RHI, Format)

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


	D3D12_BLEND ConvertBlendValue(RHIBlendFactor value) {//TODO :set a Table....
		switch (value) {
			using enum RHIBlendFactor;
		case Zero:return D3D12_BLEND_ZERO;
		case One:return D3D12_BLEND_ONE;
		case SrcColor:return D3D12_BLEND_SRC_COLOR;
		case InvSrcColor:return D3D12_BLEND_INV_SRC_COLOR;
		case SrcAlpha:return D3D12_BLEND_SRC_ALPHA;
		case InvSrcAlpha:return D3D12_BLEND_INV_SRC_ALPHA;
		case DstAlpha:return D3D12_BLEND_DEST_ALPHA;
		case InvDstAlpha:return D3D12_BLEND_INV_DEST_ALPHA;
		case DstColor:return D3D12_BLEND_DEST_COLOR;
		case InvDstColor:return D3D12_BLEND_INV_DEST_COLOR;
		case SrcAlphaSaturate:return D3D12_BLEND_SRC_ALPHA_SAT;
		case ConstantColor:return D3D12_BLEND_BLEND_FACTOR;
		case InvConstantColor:return D3D12_BLEND_INV_BLEND_FACTOR;
		case Src1Color:return D3D12_BLEND_SRC1_COLOR;
		case InvSrc1Color:return D3D12_BLEND_INV_SRC1_COLOR;
		case Src1Alpha:return D3D12_BLEND_SRC1_ALPHA;
		case InvSrc1Alpha:return D3D12_BLEND_INV_SRC1_ALPHA;
		default:ASSERT(false); return D3D12_BLEND_ZERO;
		}
	}

	D3D12_BLEND_OP ConvertBlendOp(RHIBlendOp value) {
		switch (value) {
			using enum RHIBlendOp;
		case Add:return D3D12_BLEND_OP_ADD;
		case Subrtact:return D3D12_BLEND_OP_SUBTRACT;
		case ReverseSubtract:return D3D12_BLEND_OP_REV_SUBTRACT;
		case Min:return D3D12_BLEND_OP_MIN;
		case Max:return D3D12_BLEND_OP_MAX;
		default:ASSERT(false); return D3D12_BLEND_OP_ADD;
		}
	}



	D3D12_BLEND_DESC TranslateBlendState(const RHIBlendState& inState) {
		D3D12_BLEND_DESC outState{};
		outState.AlphaToCoverageEnable = inState.AlphaToCoverageEnable;
		outState.IndependentBlendEnable = true;

		for (Uint32 Index = 0; Index < g_MaxRenderTargetCount; ++Index) {
			const auto& src{ inState.RenderTargets[Index] };
			D3D12_RENDER_TARGET_BLEND_DESC& dst{ outState.RenderTarget[Index] };

			dst.BlendEnable = src.BlendEnable;
			dst.SrcBlend = ConvertBlendValue(src.SrcBlend);
			dst.DestBlend = ConvertBlendValue(src.DestBlend);
			dst.BlendOp = ConvertBlendOp(src.BlendOp);
			dst.SrcBlendAlpha = ConvertBlendValue(src.SrcBlendAlpha);
			dst.DestBlendAlpha = ConvertBlendValue(src.DestBlendAlpha);
			dst.BlendOpAlpha = ConvertBlendOp(src.BlendOpAlpha);
			dst.RenderTargetWriteMask = static_cast<D3D12_COLOR_WRITE_ENABLE>(src.ColorWriteMask);
		}

		return outState;
	}
}