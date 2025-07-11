#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"

PARTING_SUBMODULE(D3D12RHI, InputLayout)

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
	class InputLayout final : public RHIInputLayout<InputLayout> {
		friend class RHIResource<InputLayout>;
		friend class RHIInputLayout<InputLayout>;

		friend class CommandList;
		friend class Device;
	public:
		InputLayout(void) = default;
		~InputLayout(void) = default;

	private:
		Vector<RHIVertexAttributeDesc> m_InputDesc;

		Vector<vk::VertexInputBindingDescription> m_BindingDesc;
		Vector<vk::VertexInputAttributeDescription> m_AttributeDesc;

	private:
		RHIObject Imp_GetNativeObject(RHIObjectType)const noexcept { LOG_ERROR("Imp But Empty");  return RHIObject{}; };

		Uint32 Imp_Get_AttributeCount(void)const { return static_cast<Uint32>(this->m_AttributeDesc.size()); }

		const RHIVertexAttributeDesc* Imp_Get_AttributeDesc(Uint32 Index)const {
			ASSERT(Index < this->m_InputDesc.size()/*, "Index out of range"*/);

			return &this->m_InputDesc[Index];
		};
	};
}