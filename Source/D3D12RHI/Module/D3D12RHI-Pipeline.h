#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"


PARTING_SUBMODULE(D3D12RHI, Pipeline)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Algorithm;
PARTING_IMPORT Container;
PARTING_IMPORT String;
PARTING_IMPORT Logger;

PARTING_IMPORT RHI;

PARTING_SUBMODE_IMPORT(Traits)
PARTING_SUBMODE_IMPORT(Common)
PARTING_SUBMODE_IMPORT(Format)
PARTING_SUBMODE_IMPORT(Heap)
PARTING_SUBMODE_IMPORT(Buffer)
PARTING_SUBMODE_IMPORT(Texture)
PARTING_SUBMODE_IMPORT(Sampler)
PARTING_SUBMODE_IMPORT(InputLayout)
PARTING_SUBMODE_IMPORT(Shader)
PARTING_SUBMODE_IMPORT(BlendState)
PARTING_EXPORT PARTING_SUBMODE_IMPORT(RasterState)
PARTING_SUBMODE_IMPORT(DepthStencilState)
PARTING_SUBMODE_IMPORT(ViewportState)
PARTING_SUBMODE_IMPORT(FrameBuffer)
PARTING_SUBMODE_IMPORT(ShaderBinding)

#else
#pragma once

#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"
//Global
#include "D3D12RHI/Module/DirectX12Wrapper.h"

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Algorithm/Module/Algorithm.h"
#include "Core/Container/Module/Container.h"
#include "Core/String/Module/String.h"
#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI.h"

#include "D3D12RHI/Module/D3D12RHI-Traits.h"
#include "D3D12RHI/Module/D3D12RHI-Common.h"
#include "D3D12RHI/Module/D3D12RHI-Format.h"
#include "D3D12RHI/Module/D3D12RHI-Heap.h"
#include "D3D12RHI/Module/D3D12RHI-Buffer.h"
#include "D3D12RHI/Module/D3D12RHI-Texture.h"
#include "D3D12RHI/Module/D3D12RHI-Sampler.h"
#include "D3D12RHI/Module/D3D12RHI-InputLayout.h"
#include "D3D12RHI/Module/D3D12RHI-Shader.h"
#include "D3D12RHI/Module/D3D12RHI-BlendState.h"
#include "D3D12RHI/Module/D3D12RHI-RasterState.h"
#include "D3D12RHI/Module/D3D12RHI-DepthStencilState.h"
#include "D3D12RHI/Module/D3D12RHI-ViewportState.h"
#include "D3D12RHI/Module/D3D12RHI-FrameBuffer.h"
#include "D3D12RHI/Module/D3D12RHI-ShaderBinding.h"


#endif // PARTING_MODULE_BUILD

namespace RHI::D3D12 {

	class GraphicsPipeline final :public RHIGraphicsPipeline<GraphicsPipeline, D3D12Tag> {
		friend class RHIResource<GraphicsPipeline>;
		friend class RHIGraphicsPipeline<GraphicsPipeline, D3D12Tag>;

		friend class CommandList;
		friend class Device;
	public:
		GraphicsPipeline(void) = default;
		~GraphicsPipeline(void) = default;

	public:

	private:

	private:
		RHIGraphicsPipelineDesc<D3D12Tag> m_Desc;
		RHIFrameBufferInfo<D3D12Tag> m_FrameBufferInfo;

		RefCountPtr<D3D12RootSignature> m_RootSignature;
		RefCountPtr<ID3D12PipelineState> m_PipelineState;

		bool m_RequiresBlendFactor{ false };
	private:
		RHIObject Imp_GetNativeObject(RHIObjectType type)const noexcept;

		const RHIGraphicsPipelineDesc<D3D12Tag>& Imp_Get_Desc(void)const {return this->m_Desc;}
		const RHIFrameBufferInfo<D3D12Tag>& Imp_Get_FramebufferInfo(void)const { return this->m_FrameBufferInfo; }
	};

	//Imp

	inline RHIObject GraphicsPipeline::Imp_GetNativeObject(RHIObjectType type)const noexcept {
		switch (type) {
		case RHIObjectType::D3D12_RootSignature: return this->m_RootSignature->GetNativeObject(type);
		case RHIObjectType::D3D12_PipelineState: return RHIObject{ .Pointer{ this->m_PipelineState.Get() } };
		default:ASSERT(false); return RHIObject{};
		}
	}

	class ComputePipeline final :public RHIComputePipeline<ComputePipeline, D3D12Tag> {
		friend class RHIResource<ComputePipeline>;
		friend class RHIComputePipeline<ComputePipeline, D3D12Tag>;

		friend class CommandList;
		friend class Device;
	public:
		ComputePipeline(void) = default;
		~ComputePipeline(void) = default;
	public:

	private:

	private:
		RHIComputePipelineDesc<D3D12Tag> m_Desc;
		RefCountPtr<D3D12RootSignature> m_RootSignature;
		RefCountPtr<ID3D12PipelineState> m_PipelineState;

	private:
		RHIObject Imp_GetNativeObject(RHIObjectType type)const noexcept;
		const RHIComputePipelineDesc<D3D12Tag>& Imp_Get_Desc(void)const { return this->m_Desc; }
	};

	//Imp
	RHIObject ComputePipeline::Imp_GetNativeObject(RHIObjectType type) const noexcept{
		switch (type) {
		case RHIObjectType::D3D12_RootSignature: return this->m_RootSignature->GetNativeObject(type);
		case RHIObjectType::D3D12_PipelineState: return RHIObject{ .Pointer{ this->m_PipelineState.Get() } };
		default:ASSERT(false); return RHIObject{};
		}
	}

	class MeshletPipeline final :public RHIMeshletPipeline<MeshletPipeline, D3D12Tag> {
		friend class RHIResource<MeshletPipeline>;
		friend class RHIMeshletPipeline<MeshletPipeline, D3D12Tag>;

		friend class CommandList;
	public:
		MeshletPipeline(void) = default;
		~MeshletPipeline(void) = default;

	public:

	private:

	private:
		RHIMeshletPipelineDesc<D3D12Tag> m_Desc;
		RHIFrameBufferInfo<D3D12Tag> m_FrameBufferInfo;
		RefCountPtr<D3D12RootSignature> m_RootSignature{ nullptr };
		RefCountPtr<ID3D12PipelineState> m_PipelineState{ nullptr };

		ViewportState m_viewportState;

		bool m_RequiresBlendFactor{ false };

	private:
		RHIObject Imp_GetNativeObject(RHIObjectType type)const noexcept;
		const RHIMeshletPipelineDesc<D3D12Tag>& Imp_Get_Desc(void)const { return this->m_Desc; }
		const RHIFrameBufferInfo<D3D12Tag>& Imp_Get_FramebufferInfo(void)const { return this->m_FrameBufferInfo; }
	};

	//Imp
	RHIObject MeshletPipeline::Imp_GetNativeObject(RHIObjectType type) const noexcept {
		switch (type) {
		case RHIObjectType::D3D12_RootSignature: return this->m_RootSignature->GetNativeObject(type);
		case RHIObjectType::D3D12_PipelineState: return RHIObject{ .Pointer{ this->m_PipelineState.Get() } };
		default: ASSERT(false); return RHIObject{};
		}
	}

}