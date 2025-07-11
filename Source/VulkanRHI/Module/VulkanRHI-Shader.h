#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"

PARTING_SUBMODULE(D3D12RHI, Shader)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Algorithm;
PARTING_IMPORT Container;
PARTING_IMPORT String;
PARTING_IMPORT Logger;

PARTING_IMPORT RHI;

PARTING_SUBMODE_IMPORT(Traits)
PARTING_SUBMODE_IMPORT(Common)

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
#include "RHI/Module/StateTracking.h"

#include "VulkanRHI/Module/VulkanRHI-Traits.h"
#include "VulkanRHI/Module/VulkanRHI-Common.h"
#endif // PARTING_MODULE_BUILD

namespace RHI::Vulkan {
	vk::ShaderStageFlagBits ConvertShaderTypeToShaderStageFlagBits(RHIShaderType shaderType) {
		if (shaderType == RHIShaderType::All)
			return vk::ShaderStageFlagBits::eAll;

		if (shaderType == RHIShaderType::AllGraphics)
			return vk::ShaderStageFlagBits::eAllGraphics;

		LOG_DEBUG("Converting RHIShaderType to vk::ShaderStageFlagBits: {0}", static_cast<Uint32>(shaderType));

		static_assert(static_cast<Uint32>(RHIShaderType::Vertex) == uint32_t(VK_SHADER_STAGE_VERTEX_BIT));
		static_assert(static_cast<Uint32>(RHIShaderType::Hull) == uint32_t(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT));
		static_assert(static_cast<Uint32>(RHIShaderType::Domain) == uint32_t(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT));
		static_assert(static_cast<Uint32>(RHIShaderType::Geometry) == uint32_t(VK_SHADER_STAGE_GEOMETRY_BIT));
		static_assert(static_cast<Uint32>(RHIShaderType::Pixel) == uint32_t(VK_SHADER_STAGE_FRAGMENT_BIT));
		static_assert(static_cast<Uint32>(RHIShaderType::Compute) == uint32_t(VK_SHADER_STAGE_COMPUTE_BIT));
		static_assert(static_cast<Uint32>(RHIShaderType::Amplification) == uint32_t(VK_SHADER_STAGE_TASK_BIT_NV));
		static_assert(static_cast<Uint32>(RHIShaderType::Mesh) == uint32_t(VK_SHADER_STAGE_MESH_BIT_NV));

		return static_cast<vk::ShaderStageFlagBits>(shaderType);
	}

	class Shader final : public RHIShader<Shader> {
		friend class RHIResource<Shader>;
		friend class RHIShader<Shader>;

		friend class CommandList;
		friend class Device;
	public:
		Shader(const Context& context, const RHIShaderDesc& desc) :
			RHIShader<Shader>{},
			m_Context{ context },
			m_Desc{ desc } {
		}

		~Shader(void);

	private:
		const Context& m_Context;

		RHIShaderDesc m_Desc;

		vk::ShaderModule m_ShaderModule;
		vk::ShaderStageFlagBits m_StageFlagBits;

	private:
		RHIObject Imp_GetNativeObject(RHIObjectType objectType)const noexcept {
			switch (objectType) {
				using enum RHIObjectType;
			case VK_ShaderModule:return RHIObject{ .Pointer { this->m_ShaderModule } };
			default:return RHIObject{};
			}
		}

		const RHIShaderDesc& Imp_Get_Desc(void)const { return this->m_Desc; }

		void Imp_Get_Bytecode(const void** ppbytecode, Uint64* psize)const {
			// we don't save these for vulkan
			LOG_ERROR("Vulkan Shader does not support Imp_Get_Bytecode");

			if (nullptr != ppbytecode)
				*ppbytecode = 0;
			if (nullptr != psize)
				*psize = 0;
		}
	};

	//Src

	Shader::~Shader(void) {
		if (nullptr != this->m_ShaderModule) {
			this->m_Context.Device.destroyShaderModule(this->m_ShaderModule, this->m_Context.AllocationCallbacks);
			this->m_ShaderModule = nullptr;
		}
	}
}