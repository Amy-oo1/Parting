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
#include "VulkanWrapper.h"

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Algorithm/Module/Algorithm.h"
#include "Core/Container/Module/Container.h"
#include "Core/String/Module/String.h"
#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI.h"

#include "VulkanRHI/Module/VulkanRHI-Traits.h"
#include "VulkanRHI/Module/VulkanRHI-Common.h"
#include "VulkanRHI/Module/VulkanRHI-Format.h"
#include "VulkanRHI/Module/VulkanRHI-Heap.h"
#include "VulkanRHI/Module/VulkanRHI-Buffer.h"
#include "VulkanRHI/Module/VulkanRHI-Texture.h"
#include "VulkanRHI/Module/VulkanRHI-Sampler.h"
#include "VulkanRHI/Module/VulkanRHI-InputLayout.h"
#include "VulkanRHI/Module/VulkanRHI-Shader.h"
#include "VulkanRHI/Module/VulkanRHI-BlendState.h"
#include "VulkanRHI/Module/VulkanRHI-RasterState.h"
#include "VulkanRHI/Module/VulkanRHI-DepthStencilState.h"
#include "VulkanRHI/Module/VulkanRHI-ViewportState.h"
#include "VulkanRHI/Module/VulkanRHI-FrameBuffer.h"
#include "VulkanRHI/Module/VulkanRHI-ShaderBinding.h"


#endif // PARTING_MODULE_BUILD

namespace RHI::Vulkan {
	class GraphicsPipeline final : public RHIGraphicsPipeline <GraphicsPipeline, VulkanTag> {
		friend class RHIResource<GraphicsPipeline>;
		friend class RHIGraphicsPipeline < GraphicsPipeline, VulkanTag>;

		friend class CommandList;
		friend class Device;
	public:
		explicit GraphicsPipeline(const Context& context) : m_Context{ context } {}

		~GraphicsPipeline(void);

	private:
		const Context& m_Context;

		RHIGraphicsPipelineDesc<VulkanTag> m_Desc;

		RHIFrameBufferInfo<VulkanTag> m_FrameBufferInfo;
		RHIShaderType m_ShaderMask{ RHIShaderType::None };

		Array<RefCountPtr<BindingLayout>, g_MaxBindingLayoutCount> m_PipelineBindingLayouts;
		RemoveCV<decltype(g_MaxBindingLayoutCount)>::type m_BindingLayoutCount{ 0 };

		Array<Uint32, g_MaxBindingLayoutCount> m_DescriptorSetIdxToBindingIdx;

		vk::PipelineLayout m_PipelineLayout;
		vk::Pipeline m_Pipeline;
		vk::ShaderStageFlags m_PushConstantVisibility;

		bool m_UsesBlendConstants{ false };

	private:
		RHIObject Imp_GetNativeObject(RHIObjectType type)const noexcept;

		const RHIGraphicsPipelineDesc<VulkanTag>& Imp_Get_Desc(void)const { return this->m_Desc; }
		
		const RHIFrameBufferInfo<VulkanTag>& Imp_Get_FrameBufferInfo(void)const { return this->m_FrameBufferInfo; }
	};

	//Src

	GraphicsPipeline::~GraphicsPipeline(void) {
		if (nullptr != this->m_Pipeline) {
			this->m_Context.Device.destroyPipeline(this->m_Pipeline, this->m_Context.AllocationCallbacks);
			this->m_Pipeline = nullptr;
		}

		if (nullptr != this->m_PipelineLayout) {
			this->m_Context.Device.destroyPipelineLayout(this->m_PipelineLayout, this->m_Context.AllocationCallbacks);
			this->m_PipelineLayout = nullptr;
		}
	}

	//Imp

	RHIObject GraphicsPipeline::Imp_GetNativeObject(RHIObjectType objectType)const noexcept {
		switch (objectType) {
			using enum RHIObjectType;
		case VK_PipelineLayout:return RHIObject{ .Pointer{ this->m_PipelineLayout } };
		case VK_Pipeline:return RHIObject{ .Pointer { this->m_Pipeline } };
		default:return RHIObject{};
		}
	}

	class ComputePipeline final : public RHIComputePipeline <ComputePipeline, VulkanTag> {
		friend class RHIResource<ComputePipeline>;
		friend class RHIComputePipeline <ComputePipeline, VulkanTag>;

		friend class CommandList;
		friend class Device;
	public:
		explicit ComputePipeline(const Context& context) : m_Context{ context } {}

		~ComputePipeline(void);

	private:
		const Context& m_Context;

		RHIComputePipelineDesc<VulkanTag> m_Desc;

		Array<RefCountPtr<BindingLayout>, g_MaxBindingLayoutCount> m_PipelineBindingLayouts;
		RemoveCV<decltype(g_MaxBindingLayoutCount)>::type m_BindingLayoutCount{ 0 };

		Array<Uint32, g_MaxBindingLayoutCount> m_DescriptorSetIdxToBindingIdx;

		vk::PipelineLayout m_PipelineLayout;
		vk::Pipeline m_Pipeline;
		vk::ShaderStageFlags m_PushConstantVisibility;

	private:
		RHIObject Imp_GetNativeObject(RHIObjectType type)const noexcept;

		const RHIComputePipelineDesc<VulkanTag>& Imp_Get_Desc(void)const { return this->m_Desc; }
	};

	//Src

	ComputePipeline::~ComputePipeline(void) {
		if (nullptr != this->m_Pipeline) {
			this->m_Context.Device.destroyPipeline(this->m_Pipeline, this->m_Context.AllocationCallbacks);
			this->m_Pipeline = nullptr;
		}

		if (nullptr != this->m_PipelineLayout) {
			this->m_Context.Device.destroyPipelineLayout(this->m_PipelineLayout, this->m_Context.AllocationCallbacks);
			this->m_PipelineLayout = nullptr;
		}
	}

	//Imp

	RHIObject ComputePipeline::Imp_GetNativeObject(RHIObjectType objectType)const noexcept {
		switch (objectType) {
			using enum RHIObjectType;
		case VK_PipelineLayout:return RHIObject{ .Pointer{ this->m_PipelineLayout } };
		case VK_Pipeline:return RHIObject{ .Pointer { this->m_Pipeline } };
		default:return RHIObject{};
		}
	}

	class MeshletPipeline final : public RHIMeshletPipeline<MeshletPipeline, VulkanTag> {
		friend class RHIResource<MeshletPipeline>;
		friend class RHIMeshletPipeline<MeshletPipeline, VulkanTag>;
	public:
		explicit MeshletPipeline(const Context& context) : m_Context{ context } {}

		~MeshletPipeline(void);

	private:
		const Context& m_Context;

		RHIMeshletPipelineDesc<VulkanTag> m_Desc;

		RHIFrameBufferInfo<VulkanTag> m_FrameBufferInfo;
		RHIShaderType m_ShaderMask{ RHIShaderType::None };

		Array<RefCountPtr<BindingLayout>, g_MaxBindingLayoutCount> m_PipelineBindingLayouts;
		RemoveCV<decltype(g_MaxBindingLayoutCount)>::type m_BindingLayoutCount{ 0 };

		Array<Uint32, g_MaxBindingLayoutCount> m_DescriptorSetIdxToBindingIdx;
	

		vk::PipelineLayout m_PipelineLayout;
		vk::Pipeline m_Pipeline;
		vk::ShaderStageFlags m_PushConstantVisibility;

		bool m_UsesBlendConstants{ false };

	private:
		RHIObject Imp_GetNativeObject(RHIObjectType type)const noexcept;

		const RHIMeshletPipelineDesc<VulkanTag>& Imp_Get_Desc(void)const { return this->m_Desc; }
		
		const RHIFrameBufferInfo<VulkanTag>& Imp_Get_FramebufferInfo(void)const { return this->m_FrameBufferInfo; }
	};

	//Src

	MeshletPipeline::~MeshletPipeline(void) {
		if (nullptr != this->m_Pipeline) {
			this->m_Context.Device.destroyPipeline(this->m_Pipeline, this->m_Context.AllocationCallbacks);
			this->m_Pipeline = nullptr;
		}

		if (nullptr != this->m_PipelineLayout) {
			this->m_Context.Device.destroyPipelineLayout(this->m_PipelineLayout, this->m_Context.AllocationCallbacks);
			this->m_PipelineLayout = nullptr;
		}
	}

	//Imp

	RHIObject MeshletPipeline::Imp_GetNativeObject(RHIObjectType objectType)const noexcept {
		switch (objectType) {
			using enum RHIObjectType;
		case VK_PipelineLayout:return RHIObject{ .Pointer{ this->m_PipelineLayout } };
		case VK_Pipeline:return RHIObject{ .Pointer { this->m_Pipeline } };
		default:return RHIObject{};
		}
	}
}