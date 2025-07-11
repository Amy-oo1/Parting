#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"


PARTING_SUBMODULE(D3D12RHI, Texture)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Algorithm;
PARTING_IMPORT Container;
PARTING_IMPORT String;
PARTING_IMPORT Logger;

PARTING_IMPORT RHI;

PARTING_SUBMODE_IMPORT(Traits)
PARTING_SUBMODE_IMPORT(Common)
PARTING_SUBMODE_IMPORT(Texture)

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
#include "VulkanRHI/Module/VulkanRHI-Texture.h"

#endif // PARTING_MODULE_BUILD

namespace RHI::Vulkan {
	class FrameBuffer final : public RHIFrameBuffer<FrameBuffer, VulkanTag> {
		friend class RHIResource<FrameBuffer>;
		friend class RHIFrameBuffer<FrameBuffer, VulkanTag>;

		friend class CommandList;
		friend class Device;
	public:
		explicit FrameBuffer(const Context& context) :
			RHIFrameBuffer<FrameBuffer, VulkanTag>{},
			m_Context{ context }{
		}

		~FrameBuffer(void);

	private:
		const Context& m_Context;

		RHIFrameBufferDesc<VulkanTag> m_Desc;
		RHIFrameBufferInfo<VulkanTag> m_Info;

		vk::RenderPass m_RenderPass;
		vk::Framebuffer m_FrameBuffer;

		Vector<RefCountPtr<Texture>> m_Resources;

		bool m_Managed{ true };

	private:
		RHIObject Imp_GetNativeObject(RHIObjectType objectType)const noexcept;

		const RHIFrameBufferDesc<VulkanTag>& Imp_Get_Desc(void)const { return this->m_Desc; };
		const RHIFrameBufferInfo<VulkanTag>& Imp_Get_Info(void)const { return this->m_Info; };
	};

	//Misc

	RHITextureDimension	GetDimensionForFramebuffer(RHITextureDimension dimension, bool isArray) {
		using enum RHITextureDimension;
		// Can't render into cubes and 3D textures directly, convert them to 2D arrays
		if (dimension == TextureCube || dimension == TextureCubeArray || dimension == Texture3D)
			dimension = Texture2DArray;

		if (!isArray){
			// Demote arrays to single textures if we just need one layer
			switch (dimension){
			case Texture1DArray:
				dimension = Texture1D;
				break;

			case Texture2DArray:
				dimension = Texture2D;
				break;

			case Texture2DMSArray:
				dimension = Texture2DMS;
				break;

			default:
				break;
			}
		}

		return dimension;
	}

	//Src
	
	FrameBuffer::~FrameBuffer(void) {
		if (nullptr != this->m_FrameBuffer && this->m_Managed) {
			this->m_Context.Device.destroyFramebuffer(this->m_FrameBuffer);
			this->m_FrameBuffer = nullptr;
		}

		if (nullptr != this->m_RenderPass && this->m_Managed) {
			this->m_Context.Device.destroyRenderPass(this->m_RenderPass);
			this->m_RenderPass = nullptr;
		}
	}

	//Imp
	
	RHIObject FrameBuffer::Imp_GetNativeObject(RHIObjectType objectType)const noexcept {
		switch (objectType) {
			using enum RHIObjectType;
		case VK_RenderPass:return RHIObject{ .Pointer{ this->m_RenderPass } };
		case VK_Framebuffer:return RHIObject{ .Pointer{ this->m_FrameBuffer } };
		default:return RHIObject{};
		}
	}
}