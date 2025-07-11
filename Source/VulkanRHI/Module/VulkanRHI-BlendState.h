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
	vk::BlendFactor ConvertBlendValue(RHIBlendFactor value) {
		switch (value) {
			using enum RHIBlendFactor;
		case Zero:return vk::BlendFactor::eZero;
		case One:return vk::BlendFactor::eOne;
		case SrcColor:return vk::BlendFactor::eSrcColor;
		case OneMinusSrcColor:return vk::BlendFactor::eOneMinusSrcColor;
		case SrcAlpha:return vk::BlendFactor::eSrcAlpha;
		case OneMinusSrcAlpha:return vk::BlendFactor::eOneMinusSrcAlpha;
		case DstAlpha:return vk::BlendFactor::eDstAlpha;
		case OneMinusDstAlpha:return vk::BlendFactor::eOneMinusDstAlpha;
		case DstColor:return vk::BlendFactor::eDstColor;
		case OneMinusDstColor:return vk::BlendFactor::eOneMinusDstColor;
		case SrcAlphaSaturate:return vk::BlendFactor::eSrcAlphaSaturate;
		case ConstantColor:return vk::BlendFactor::eConstantColor;
		case OneMinusConstantColor:return vk::BlendFactor::eOneMinusConstantColor;
		case Src1Color:return vk::BlendFactor::eSrc1Color;
		case OneMinusSrc1Color:return vk::BlendFactor::eOneMinusSrc1Color;
		case Src1Alpha:return vk::BlendFactor::eSrc1Alpha;
		case OneMinusSrc1Alpha:return vk::BlendFactor::eOneMinusSrc1Alpha;
		default:ASSERT(false); return vk::BlendFactor::eZero;
		}
	}

	vk::BlendOp ConvertBlendOp(RHIBlendOp op) {
		switch (op) {
			using enum RHIBlendOp;
		case Add:return vk::BlendOp::eAdd;
		case Subrtact:return vk::BlendOp::eSubtract;
		case ReverseSubtract:return vk::BlendOp::eReverseSubtract;
		case Min:return vk::BlendOp::eMin;
		case Max:return vk::BlendOp::eMax;
		default:ASSERT(false); return vk::BlendOp::eAdd;
		}
	}

	vk::ColorComponentFlags ConvertColorMask(RHIColorMask mask) {
		return vk::ColorComponentFlags{ static_cast<vk::ColorComponentFlagBits>(mask) };
	}

	vk::PipelineColorBlendAttachmentState ConvertBlendState(const RHIBlendState::RHIRenderTarget& state) {
		return vk::PipelineColorBlendAttachmentState{}
			.setBlendEnable(state.BlendEnable)
			.setSrcColorBlendFactor(ConvertBlendValue(state.SrcBlend))
			.setDstColorBlendFactor(ConvertBlendValue(state.DestBlend))
			.setColorBlendOp(ConvertBlendOp(state.BlendOp))
			.setSrcAlphaBlendFactor(ConvertBlendValue(state.SrcBlendAlpha))
			.setDstAlphaBlendFactor(ConvertBlendValue(state.DestBlendAlpha))
			.setAlphaBlendOp(ConvertBlendOp(state.BlendOpAlpha))
			.setColorWriteMask(ConvertColorMask(state.ColorWriteMask));
	}
}