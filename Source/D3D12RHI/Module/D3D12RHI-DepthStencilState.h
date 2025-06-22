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

	D3D12_STENCIL_OP ConvertStencilOp(RHIStencilOp value) {
		switch (value) {
			using enum RHIStencilOp;
		case Keep:return D3D12_STENCIL_OP_KEEP;
		case Zero:return D3D12_STENCIL_OP_ZERO;
		case Replace:return D3D12_STENCIL_OP_REPLACE;
		case IncrementAndClamp:return D3D12_STENCIL_OP_INCR_SAT;
		case DecrementAndClamp:return D3D12_STENCIL_OP_DECR_SAT;
		case Invert:return D3D12_STENCIL_OP_INVERT;
		case IncrementAndWrap:return D3D12_STENCIL_OP_INCR;
		case DecrementAndWrap:return D3D12_STENCIL_OP_DECR;
		default:ASSERT(false); return D3D12_STENCIL_OP_KEEP;
		}
	}

	D3D12_COMPARISON_FUNC ConvertComparisonFunc(RHIComparisonFunc value) {
		switch (value) {
			using enum RHIComparisonFunc;
		case Never:return D3D12_COMPARISON_FUNC_NEVER;
		case Less:return D3D12_COMPARISON_FUNC_LESS;
		case Equal:return D3D12_COMPARISON_FUNC_EQUAL;
		case LessOrEqual:return D3D12_COMPARISON_FUNC_LESS_EQUAL;
		case Greater:return D3D12_COMPARISON_FUNC_GREATER;
		case NotEqual:return D3D12_COMPARISON_FUNC_NOT_EQUAL;
		case GreaterOrEqual:return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		case Always:return D3D12_COMPARISON_FUNC_ALWAYS;
		default:ASSERT(false); return D3D12_COMPARISON_FUNC_NEVER;
		}
	}


	D3D12_DEPTH_STENCIL_DESC TranslateDepthStencilState(const RHIDepthStencilState& inState) {
		return D3D12_DEPTH_STENCIL_DESC{
			.DepthEnable { inState.DepthTestEnable },
			.DepthWriteMask { inState.DepthWriteEnable ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO },
			.DepthFunc { ConvertComparisonFunc(inState.DepthFunc) },
			.StencilEnable { inState.StencilEnable },
			.StencilReadMask { inState.StencilReadMask },
			.StencilWriteMask { inState.StencilWriteMask },
			.FrontFace{
				.StencilFailOp { ConvertStencilOp(inState.FrontFaceStencil.FailOp) },
				.StencilDepthFailOp { ConvertStencilOp(inState.FrontFaceStencil.DepthFailOp) },
				.StencilPassOp { ConvertStencilOp(inState.FrontFaceStencil.PassOp) },
				.StencilFunc { ConvertComparisonFunc(inState.FrontFaceStencil.StencilFunc) }
			},
			.BackFace{
				.StencilFailOp { ConvertStencilOp(inState.BackFaceStencil.FailOp) },
				.StencilDepthFailOp { ConvertStencilOp(inState.BackFaceStencil.DepthFailOp) },
				.StencilPassOp { ConvertStencilOp(inState.BackFaceStencil.PassOp) },
				.StencilFunc { ConvertComparisonFunc(inState.BackFaceStencil.StencilFunc) }
			}
		};
		/*outState.DepthEnable = inState.DepthTestEnable;
		outState.DepthWriteMask = inState.DepthWriteEnable ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
		outState.DepthFunc = ConvertComparisonFunc(inState.DepthFunc);
		outState.StencilEnable = inState.StencilEnable;
		outState.StencilReadMask = inState.StencilReadMask;
		outState.StencilWriteMask = inState.StencilWriteMask;
		outState.FrontFace.StencilFailOp = ConvertStencilOp(inState.FrontFaceStencil.FailOp);
		outState.FrontFace.StencilDepthFailOp = ConvertStencilOp(inState.FrontFaceStencil.DepthFailOp);
		outState.FrontFace.StencilPassOp = ConvertStencilOp(inState.FrontFaceStencil.PassOp);
		outState.FrontFace.StencilFunc = ConvertComparisonFunc(inState.FrontFaceStencil.StencilFunc);
		outState.BackFace.StencilFailOp = ConvertStencilOp(inState.BackFaceStencil.FailOp);
		outState.BackFace.StencilDepthFailOp = ConvertStencilOp(inState.BackFaceStencil.DepthFailOp);
		outState.BackFace.StencilPassOp = ConvertStencilOp(inState.BackFaceStencil.PassOp);
		outState.BackFace.StencilFunc = ConvertComparisonFunc(inState.BackFaceStencil.StencilFunc);

		return outState;*/
	}
}