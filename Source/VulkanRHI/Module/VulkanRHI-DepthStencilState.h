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

namespace RHI::Vulkan {
	vk::CompareOp ConvertCompareOp(RHIComparisonFunc op) {
		switch (op) {
			using enum RHIComparisonFunc;
		case Never:return vk::CompareOp::eNever;
		case Less:return vk::CompareOp::eLess;
		case Equal:return vk::CompareOp::eEqual;
		case LessOrEqual:return vk::CompareOp::eLessOrEqual;
		case Greater:return vk::CompareOp::eGreater;
		case NotEqual:return vk::CompareOp::eNotEqual;
		case GreaterOrEqual:return vk::CompareOp::eGreaterOrEqual;
		case Always:return vk::CompareOp::eAlways;
		default:ASSERT(false); return vk::CompareOp::eAlways;
		}
	}

	vk::StencilOp ConvertStencilOp(RHIStencilOp op) {
		switch (op) {
			using enum RHIStencilOp;
		case Keep:return vk::StencilOp::eKeep;
		case Zero:return vk::StencilOp::eZero;
		case Replace:return vk::StencilOp::eReplace;
		case IncrementAndClamp:return vk::StencilOp::eIncrementAndClamp;
		case DecrementAndClamp:return vk::StencilOp::eDecrementAndClamp;
		case Invert:return vk::StencilOp::eInvert;
		case IncrementAndWrap:return vk::StencilOp::eIncrementAndWrap;
		case DecrementAndWrap:return vk::StencilOp::eDecrementAndWrap;
		default:ASSERT(false); return vk::StencilOp::eKeep;
		}
	}

	vk::StencilOpState ConvertStencilState(const RHIDepthStencilState& depthStencilState, const RHIDepthStencilState::RHIStencilOpDesc& desc) {
		return vk::StencilOpState{}
			.setFailOp(ConvertStencilOp(desc.FailOp))
			.setPassOp(ConvertStencilOp(desc.PassOp))
			.setDepthFailOp(ConvertStencilOp(desc.DepthFailOp))
			.setCompareOp(ConvertCompareOp(desc.StencilFunc))
			.setCompareMask(depthStencilState.StencilReadMask)
			.setWriteMask(depthStencilState.StencilWriteMask)
			.setReference(depthStencilState.StencilRefValue);
	}
}