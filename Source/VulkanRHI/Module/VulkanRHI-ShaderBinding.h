#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"


PARTING_SUBMODULE(D3D12RHI, ShaderBinding)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Algorithm;
PARTING_IMPORT Container;
PARTING_IMPORT String;
PARTING_IMPORT Logger;

PARTING_IMPORT RHI;

PARTING_SUBMODE_IMPORT(Traits)
PARTING_SUBMODE_IMPORT(Common)
PARTING_SUBMODE_IMPORT(Buffer)
PARTING_SUBMODE_IMPORT(Texture)
PARTING_SUBMODE_IMPORT(Sampler)

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
#include "VulkanRHI/Module/VulkanRHI-Buffer.h"
#include "VulkanRHI/Module/VulkanRHI-Texture.h"
#include "VulkanRHI/Module/VulkanRHI-Sampler.h"

#endif // PARTING_MODULE_BUILD

namespace RHI::Vulkan {
	class BindingLayout final : public RHIBindingLayout<BindingLayout> {
		friend class RHIResource<BindingLayout>;
		friend class RHIBindingLayout<BindingLayout>;

		friend class CommandList;
		friend class Device;
	public:
		BindingLayout(const Context& context, const RHIBindingLayoutDesc& desc);

		~BindingLayout(void);

	public:
		void Bake(void);// generate the descriptor set layout

	private:
		const Context& m_Context;

		RHIBindingLayoutDesc m_Desc;

		Vector<vk::DescriptorSetLayoutBinding> m_VulkanLayoutBindings;

		vk::DescriptorSetLayout m_DescriptorSetLayout;

		// descriptor pool size information per binding set
		Vector<vk::DescriptorPoolSize> m_DescriptorPoolSizeInfo;

	private:
		RHIObject Imp_GetNativeObject(RHIObjectType objectType)const noexcept {
			switch (objectType) {
				using enum RHIObjectType;
			case VK_DescriptorSetLayout:return RHIObject{ .Pointer { this->m_DescriptorSetLayout } };
			default:return RHIObject{};
			}
		}

		const RHIBindingLayoutDesc* Imp_Get_Desc(void)const { return &this->m_Desc; };
	};

	namespace BindingOffsets {
		constexpr Uint32 ShaderResource{ 0 };
		constexpr Uint32 Sampler{ 128 };
		constexpr Uint32 ConstantBuffer{ 256 };
		constexpr Uint32 UnorderedAccess{ 384 };
	}

	//Src

	BindingLayout::BindingLayout(const Context& context, const RHIBindingLayoutDesc& _desc) :
		RHIBindingLayout<BindingLayout>{},
		m_Context{ context },
		m_Desc{ _desc } {

		vk::ShaderStageFlagBits shaderStageFlags{ ConvertShaderTypeToShaderStageFlagBits(this->m_Desc.Visibility) };

		// iterate over all binding types and add to map
		for (const auto& binding : Span< const RHIBindingLayoutItem>{ this->m_Desc.Bindings.data(), this->m_Desc.BindingCount }) {
			vk::DescriptorType descriptorType;
			Uint32 descriptorCount{ 1 };
			Uint32 registerOffset;

			switch (binding.Type) {
				using enum RHIResourceType;
			case Texture_SRV:
				registerOffset = BindingOffsets::ShaderResource;
				descriptorType = vk::DescriptorType::eSampledImage;
				break;

			case Texture_UAV:
				registerOffset = BindingOffsets::UnorderedAccess;
				descriptorType = vk::DescriptorType::eStorageImage;
				break;

			case TypedBuffer_SRV:
				registerOffset = BindingOffsets::ShaderResource;
				descriptorType = vk::DescriptorType::eUniformTexelBuffer;
				break;

			case StructuredBuffer_SRV:case RawBuffer_SRV:
				registerOffset = BindingOffsets::ShaderResource;
				descriptorType = vk::DescriptorType::eStorageBuffer;
				break;

			case TypedBuffer_UAV:
				registerOffset = BindingOffsets::UnorderedAccess;
				descriptorType = vk::DescriptorType::eStorageTexelBuffer;
				break;

			case StructuredBuffer_UAV:case RawBuffer_UAV:
				registerOffset = BindingOffsets::UnorderedAccess;
				descriptorType = vk::DescriptorType::eStorageBuffer;
				break;

			case ConstantBuffer:
				registerOffset = BindingOffsets::ConstantBuffer;
				descriptorType = vk::DescriptorType::eUniformBuffer;
				break;

			case VolatileConstantBuffer:
				registerOffset = BindingOffsets::ConstantBuffer;
				descriptorType = vk::DescriptorType::eUniformBufferDynamic;
				break;

			case Sampler:
				registerOffset = BindingOffsets::Sampler;
				descriptorType = vk::DescriptorType::eSampler;
				break;

			case PushConstants:
				// don't need any descriptors for the push constants, but the vulkanLayoutBindings array 
				// must match the binding layout items for further processing within RHI --
				// so set descriptorCount to 0 instead of skipping it
				registerOffset = BindingOffsets::ConstantBuffer;
				descriptorType = vk::DescriptorType::eUniformBuffer;
				descriptorCount = 0;
				break;

			case None:case Count:default:
				ASSERT(false);
				break;
			}

			const auto bindingLocation{ registerOffset + binding.Slot };

			this->m_VulkanLayoutBindings.push_back(vk::DescriptorSetLayoutBinding{}
				.setBinding(bindingLocation)
				.setDescriptorCount(descriptorCount)
				.setDescriptorType(descriptorType)
				.setStageFlags(shaderStageFlags)
			);
		}
	}

	BindingLayout::~BindingLayout(void) {
		if (nullptr != this->m_DescriptorSetLayout) {
			this->m_Context.Device.destroyDescriptorSetLayout(this->m_DescriptorSetLayout, this->m_Context.AllocationCallbacks);
			this->m_DescriptorSetLayout = nullptr;
		}
	}

	void BindingLayout::Bake(void) {
		// create the descriptor set layout object

		auto descriptorSetLayoutInfo{ vk::DescriptorSetLayoutCreateInfo{}
			.setBindingCount(static_cast<Uint32>(this->m_VulkanLayoutBindings.size()))
			.setPBindings(this->m_VulkanLayoutBindings.data())
		};

		VULKAN_CHECK(this->m_Context.Device.createDescriptorSetLayout(&descriptorSetLayoutInfo, this->m_Context.AllocationCallbacks, &this->m_DescriptorSetLayout));

		// count the number of descriptors required per type
		UnorderedMap<vk::DescriptorType, Uint32> poolSizeMap;
		for (const auto& layoutBinding : this->m_VulkanLayoutBindings) {
			if (poolSizeMap.find(layoutBinding.descriptorType) == poolSizeMap.end())
				poolSizeMap[layoutBinding.descriptorType] = 0;

			poolSizeMap[layoutBinding.descriptorType] += layoutBinding.descriptorCount;
		}

		// compute descriptor pool size info
		for (const auto& poolSizeIter : poolSizeMap)
			if (poolSizeIter.second > 0) {
				this->m_DescriptorPoolSizeInfo.push_back(vk::DescriptorPoolSize{}
					.setType(poolSizeIter.first)
					.setDescriptorCount(poolSizeIter.second));
			}
	}

	class BindingSet final : public RHIBindingSet<BindingSet, VulkanTag> {
		friend class RHIResource<BindingSet>;
		friend class RHIBindingSet<BindingSet, VulkanTag>;

		friend class CommandList;
		friend class Device;
	public:
		explicit BindingSet(const Context& context) :
			RHIBindingSet<BindingSet, VulkanTag>{},
			m_Context{ context } {
		}

		~BindingSet(void);

	private:
		const Context& m_Context;

		RHIBindingSetDesc<VulkanTag> m_Desc;
		RefCountPtr<BindingLayout> m_Layout;

		Vector<RHIShaderBindingResources<VulkanTag>> m_Resources;

		Array<Buffer*, g_MaxVolatileConstantBufferCountPerLayout> m_VolatileConstantBuffers;
		RemoveCV<decltype(g_MaxVolatileConstantBufferCountPerLayout)>::type m_VolatileConstantBuffersCount{ 0 };

		Vector<Uint16> m_BindingsThatNeedTransitions;

		// TODO: move pool to the context instead
		vk::DescriptorPool m_DescriptorPool;
		vk::DescriptorSet m_DescriptorSet;

	private:
		RHIObject Imp_GetNativeObject(RHIObjectType)const noexcept;

		const RHIBindingSetDesc<VulkanTag>* Imp_Get_Desc(void)const { return &this->m_Desc; };

		const BindingLayout* Imp_Get_Layout(void)const { return this->m_Layout.Get(); };
	};

	RHIObject BindingSet::Imp_GetNativeObject(RHIObjectType objectType)const noexcept {
		switch (objectType) {
			using enum RHIObjectType;
		case VK_DescriptorPool:return RHIObject{ .Pointer{ this->m_DescriptorPool } };
		case VK_DescriptorSet:return RHIObject{ .Pointer{ this->m_DescriptorSet } };
		default:return RHIObject{};
		}
	}

	BindingSet::~BindingSet(void) {
		if (nullptr != this->m_DescriptorPool) {
			this->m_Context.Device.destroyDescriptorPool(this->m_DescriptorPool, this->m_Context.AllocationCallbacks);
			this->m_DescriptorPool = nullptr;
			this->m_DescriptorSet = nullptr;
		}
	}
}