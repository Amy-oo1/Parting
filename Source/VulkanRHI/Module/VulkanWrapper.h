#pragma once

#ifndef PARTING_VULKAN_INCLUDE
#define PARTING_VULKAN_INCLUDE
#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include "vulkan/vulkan.h"
#include <vulkan/vulkan.hpp> 
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
#endif // !VulkanInclude



namespace RHI::Vulkan {

	constexpr auto MinimumVulkanAPIVersion{ VK_API_VERSION_1_3 }; // Vulkan 1.3

}
namespace Export {

	//Misc
	using vk::Result;

#define VULKAN_CHECK(res) ASSERT((res) == vk::Result::eSuccess)




	//Base
	using vk::DeviceAddress;


	//Enum
	using vk::MemoryPropertyFlags;
	using vk::BufferUsageFlags;
	using vk::ImageUsageFlags;
	using vk::ShaderStageFlags;
	using vk::DescriptorBindingFlags;
	using vk::PipelineStageFlags;
	using vk::PipelineStageFlags2;
	using vk::AccessFlags;
	using vk::AccessFlags2;
	using vk::MemoryPropertyFlags;
	using vk::ImageCreateFlags;
	using vk::MemoryPropertyFlags;

	using vk::BufferUsageFlagBits;
	using vk::ImageAspectFlagBits;
	using vk::ShaderStageFlagBits;
	using vk::DescriptorBindingFlagBits;
	using vk::CommandPoolCreateFlagBits;
	using vk::CommandBufferUsageFlagBits;
	using vk::StencilFaceFlagBits;
	using vk::MemoryPropertyFlagBits;
	using vk::SampleCountFlagBits;
	using vk::ImageCreateFlagBits;


	using vk::ObjectType;

	using vk::ImageViewType;
	using vk::DescriptorType;

	using vk::CommandBufferLevel;
	using vk::ImageLayout;

	using ::VkFormat;
	using vk::Format;

	using vk::PipelineBindPoint;
	using vk::PipelineBindPoint;

	using vk::DebugReportObjectTypeEXT;




	//Structs
	using ::VkAllocationCallbacks;
	using vk::AllocationCallbacks;

	using vk::PhysicalDeviceProperties;

	using vk::SamplerCreateInfo;
	using vk::BufferCreateInfo;
	using vk::ImageCreateInfo;
	using vk::ImageViewCreateInfo;
	using vk::ImageViewUsageCreateInfo;
	using vk::ExternalMemoryImageCreateInfo;
	using vk::DescriptorSetLayoutCreateInfo;
	using vk::DescriptorSetLayoutBindingFlagsCreateInfo;
	using vk::CommandPoolCreateInfo;
	using vk::PipelineCacheCreateInfo;
	using vk::DescriptorSetLayoutCreateInfo;

	using vk::CommandBufferAllocateInfo;
	using vk::CommandBufferBeginInfo;
	using vk::RenderPassBeginInfo;

	using vk::MappedMemoryRange;
	using vk::ImageSubresourceRange;

	using vk::ClearColorValue;
	using vk::ClearDepthStencilValue;

	using vk::VertexInputBindingDescription;
	using vk::VertexInputAttributeDescription;

	using vk::MemoryRequirements;

	using vk::DeviceSize;
	using vk::DescriptorPoolSize;

	using vk::BufferCopy;

	using vk::ImageResolve;
	using vk::ImageSubresourceLayers;

	using vk::Viewport;
	using vk::Rect2D;
	using vk::Offset2D;
	using vk::Extent2D;

	using vk::ImageMemoryBarrier;
	using vk::ImageMemoryBarrier2;
	using vk::BufferMemoryBarrier;
	using vk::BufferMemoryBarrier2;

	using vk::MemoryRequirements;

	using vk::PhysicalDeviceMemoryProperties;
	using vk::PhysicalDeviceProperties2;
	using vk::PhysicalDeviceConservativeRasterizationPropertiesEXT;

	using vk::VertexInputBindingDescription;

	//Class
	using ::VkInstance;
	using vk::Instance;

	using ::VkPhysicalDevice;
	using vk::PhysicalDevice;

	using ::VkDevice;
	using vk::Device;

	using ::VkQueue;
	using vk::PipelineCache;

	using vk::Buffer;
	using vk::BufferView;

	using vk::Image;

	using vk::ImageView;
	using ::VkImageView;

	using vk::Sampler;
	using ::VkSemaphore;


	using vk::ShaderModule;

	using vk::Framebuffer;

	using vk::RenderPass;

	using vk::DescriptorSetLayout;
	using vk::DescriptorSetLayoutBinding;
	using vk::DescriptorPoolSize;

	using vk::DescriptorPool;
	using vk::DescriptorSet;

	using vk::PipelineLayout;
	using vk::Pipeline;


	using vk::Semaphore;

}


#ifdef _WIN32
#define VULKAN_PLATFORM_WIN32
#endif // _Win32
