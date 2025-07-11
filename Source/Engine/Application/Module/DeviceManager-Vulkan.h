#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"

PARTING_SUBMODULE(DeviceManager, D3D12)

PARTING_IMPORT GLFWWrapper;

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Container;

PARTING_IMPORT RHI;

PARTING_SUBMODE_IMPORT Base;


#else
#pragma once

#include "Core/ModuleBuild.h"


#include "Core/Utility/Include/UtilityMacros.h"
//Global
#include "Engine/Application/Module/GLFWWrapper.h"

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Container/Module/Container.h"
#include "Core/String/Module/String.h"

#include "RHI/Module/RHI.h"

#include "VulkanRHI/Module/VulkanRHI.h"

#include "Engine/Application/Module/DeviceManager-Base.h"

#endif // PARTING_MODULE_BUILD

namespace Parting {

	class VulkaneviceManager;

	template<>
	struct ManageTypeTraits<RHI::VulkanTag> {
		using DeviceManager = typename Parting::VulkaneviceManager;
	};

	namespace _NameSpace_VulkaneviceManager {
		struct VulkanExtensionSet final {
			UnorderedSet<String> Instance;
			UnorderedSet<String> Layers;
			UnorderedSet<String> Device;
		};
	}

	class VulkaneviceManager final : public DeviceManagerBase<VulkaneviceManager, RHI::VulkanTag> {
		friend class DeviceManagerBase<VulkaneviceManager, RHI::VulkanTag>;

		using Imp_Texture = typename RHI::RHITypeTraits<RHI::VulkanTag>::Imp_Texture;
		using Imp_EventQuery = typename RHI::RHITypeTraits<RHI::VulkanTag>::Imp_EventQuery;
		using Imp_Device = typename RHI::RHITypeTraits<RHI::VulkanTag>::Imp_Device;

		using VulkanExtensionSet = _NameSpace_VulkaneviceManager::VulkanExtensionSet;

		using VulkanDynamicLoader = vk::detail::DynamicLoader;
	public:
		VulkaneviceManager(void) = default;
		~VulkaneviceManager(void) = default;

	private:
		void InstallDebugCallback(void);

		void VulkanCreateSwapChain(void);

		void VulkanDestroySwapChain(void);

	private:
		VulkanExtensionSet m_EnabledExtensions{
			.Instance{ "VK_KHR_get_physical_device_properties2" },
			.Device{
				"VK_KHR_swapchain",
				"VK_KHR_buffer_device_address",
				"VK_KHR_maintenance1"
			}
		};

		VulkanExtensionSet m_OptionalExtensions{
			.Instance{
				"VK_EXT_debug_utils",
				"VK_EXT_sampler_filter_minmax"
			},
			.Device{
				"VK_EXT_debug_marker",
				"VK_EXT_descriptor_indexing",
				"VK_KHR_maintenance4",
				"VK_KHR_swapchain_mutable_format",
				"VK_KHR_synchronization2",
				"VK_NV_mesh_shader"
			}
		};

		String m_RendererString;

		vk::Instance m_VulkanInstance;
		vk::DebugReportCallbackEXT m_DebugReportCallback;

		vk::PhysicalDevice m_VulkanPhysicalDevice;
		Uint32 m_GraphicsQueueFamily{ Max_Uint32 };
		Uint32 m_ComputeQueueFamily{ Max_Uint32 };
		Uint32 m_TransferQueueFamily{ Max_Uint32 };
		Uint32 m_PresentQueueFamily{ Max_Uint32 };

		vk::Device m_VulkanDevice;
		vk::Queue m_GraphicsQueue;
		vk::Queue m_ComputeQueue;
		vk::Queue m_TransferQueue;
		vk::Queue m_PresentQueue;

		vk::SurfaceKHR m_WindowSurface;

		vk::SurfaceFormatKHR m_SwapChainFormat;
		vk::SwapchainKHR m_SwapChain;

		Vector<vk::Image> m_SwapChainImages;

		bool m_SwapChainMutableFormatSupported{ false };

		Vector<vk::Semaphore> m_AcquireSemaphores;
		Vector<vk::Semaphore> m_PresentSemaphores;
		Uint32 m_AcquireSemaphoreIndex{ 0 };
		Uint32 m_PresentSemaphoreIndex{ 0 };

		Queue<RHI::RefCountPtr<Imp_EventQuery>> m_FramesInFlight;
		Vector<RHI::RefCountPtr<Imp_EventQuery>> m_QueryPool;

		UniquePtr<VulkanDynamicLoader> m_DynamicLoader;

		RHI::RefCountPtr<Imp_Device> m_RHIDevice;

		Vector<RHI::RefCountPtr<Imp_Texture>> m_RHISwapChainImages;
		Uint32 m_SwapChainIndex{ 0 };



	private:
		bool Imp_CreateInstance(void);
		bool Imp_CreateDevice(void);
		bool Imp_CreateSwapChain(void);
		void Imp_ResizeSwapChain(void);
		Uint32 Imp_Get_BackBufferCount(void) const { return this->m_DeviceParams.SwapChainBufferCount; }
		Imp_Device* Imp_Get_Device(void) { return this->m_RHIDevice.Get(); }
		Imp_Texture* Imp_Get_BackBuffer(Uint32 index) { ASSERT(index < this->m_RHISwapChainImages.size());	return this->m_RHISwapChainImages[index].Get(); }
		Uint32 Imp_Get_CurrentBackBufferIndex(void) { return this->m_SwapChainIndex; }
		bool Imp_BeginFrame(void);
		void Imp_DestroyDeviceAndSwapChain(void);
		bool Imp_Present(void);
		void Imp_Shutdown(void);
	};

	//Misc

	Vector<const char*> StringSetToVector(const UnorderedSet<String>& set) {
		Vector<const char*> ret;
		for (const auto& s : set)
			ret.push_back(s.c_str());

		return ret;
	}

	template <typename Type>
	Vector<Type> SetToVector(const UnorderedSet<Type>& set) {
		Vector<Type> ret;
		for (const auto& s : set)
			ret.push_back(s);

		return ret;
	}

	//Misc

	// On Windows, Vulkan commands use the stdcall convention
	static VKAPI_ATTR VkBool32 __stdcall VulkanDebugCallback(vk::DebugReportFlagsEXT flags, vk::DebugReportObjectTypeEXT objType, Uint64 obj, Uint64 location, Int32 code, const char* layerPrefix, const char* msg, void* userData) {
		const VulkaneviceManager* manager{ static_cast<const VulkaneviceManager*>(userData) };

		LOG_INFO("[Vulkan: location = {0} code = {1}, layerPrefix={2}] {3}", location, code, layerPrefix, msg);

		return 0U;
	}

	//Src

	void VulkaneviceManager::InstallDebugCallback(void) {
		auto info{ vk::DebugReportCallbackCreateInfoEXT{}
			.setFlags(vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eWarning |/*vk::DebugReportFlagBitsEXT::eInformation |*/ vk::DebugReportFlagBitsEXT::ePerformanceWarning)
			.setPfnCallback(VulkanDebugCallback)
			.setPUserData(this)
		};

		VULKAN_CHECK(this->m_VulkanInstance.createDebugReportCallbackEXT(&info, nullptr, &this->m_DebugReportCallback));
	}

	inline void VulkaneviceManager::VulkanCreateSwapChain(void) {
		this->VulkanDestroySwapChain();
		this->m_SwapChainFormat = vk::SurfaceFormatKHR{
			vk::Format(RHI::Vulkan::ConvertFormat(this->m_DeviceParams.SwapChainFormat)),
			vk::ColorSpaceKHR::eSrgbNonlinear
		};

		vk::Extent2D extent{ vk::Extent2D(this->m_DeviceParams.BackBufferWidth, this->m_DeviceParams.BackBufferHeight) };

		UnorderedSet<Uint32> uniqueQueues{ this->m_GraphicsQueueFamily,this->m_PresentQueueFamily };

		Vector<Uint32> queues{ SetToVector(uniqueQueues) };

		const bool enableSwapChainSharing{ queues.size() > 1 };

		auto desc{ vk::SwapchainCreateInfoKHR{}
			.setSurface(this->m_WindowSurface)
			.setMinImageCount(this->m_DeviceParams.SwapChainBufferCount)
			.setImageFormat(this->m_SwapChainFormat.format)
			.setImageColorSpace(this->m_SwapChainFormat.colorSpace)
			.setImageExtent(extent)
			.setImageArrayLayers(1)
			.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled)
			.setImageSharingMode(enableSwapChainSharing ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive)
			.setFlags(this->m_SwapChainMutableFormatSupported ? vk::SwapchainCreateFlagBitsKHR::eMutableFormat : vk::SwapchainCreateFlagBitsKHR{})
			.setQueueFamilyIndexCount(enableSwapChainSharing ? static_cast<Uint32>(queues.size()) : 0)
			.setPQueueFamilyIndices(enableSwapChainSharing ? queues.data() : nullptr)
			.setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity)
			.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
			.setPresentMode(this->m_DeviceParams.VsyncEnabled ? vk::PresentModeKHR::eFifo : vk::PresentModeKHR::eImmediate)
			.setClipped(true)
		};

		Vector<vk::Format> imageFormats{ this->m_SwapChainFormat.format };
		switch (this->m_SwapChainFormat.format) {
		case vk::Format::eR8G8B8A8Unorm:
			imageFormats.push_back(vk::Format::eR8G8B8A8Srgb);
			break;

		case vk::Format::eR8G8B8A8Srgb:
			imageFormats.push_back(vk::Format::eR8G8B8A8Unorm);
			break;

		case vk::Format::eB8G8R8A8Unorm:
			imageFormats.push_back(vk::Format::eB8G8R8A8Srgb);
			break;

		case vk::Format::eB8G8R8A8Srgb:
			imageFormats.push_back(vk::Format::eB8G8R8A8Unorm);
			break;

		default:
			break;
		}

		auto imageFormatListCreateInfo{ vk::ImageFormatListCreateInfo{}
			.setViewFormats(imageFormats)
		};

		if (this->m_SwapChainMutableFormatSupported)
			desc.pNext = &imageFormatListCreateInfo;

		VULKAN_CHECK(this->m_VulkanDevice.createSwapchainKHR(&desc, nullptr, &this->m_SwapChain));

		// retrieve swap chain images
		auto images{ this->m_VulkanDevice.getSwapchainImagesKHR(this->m_SwapChain) };
		for (auto& image : images) {
			RHI::RHITextureDesc textureDesc{
				.Extent{.Width{ this->m_DeviceParams.BackBufferWidth }, .Height{ this->m_DeviceParams.BackBufferHeight } },
				.Format{ this->m_DeviceParams.SwapChainFormat },
				.DebugName{ String{ "Swap chain image" } },
				.IsRenderTarget{ true },
				.InitialState{ RHI::RHIResourceState::Present },
				.KeepInitialState{ true },
			};

			this->m_SwapChainImages.push_back(image);
			this->m_RHISwapChainImages.push_back(this->m_RHIDevice->CreateHandleForNativeTexture(RHI::RHIObjectType::VK_Image, RHI::RHIObject{ .Pointer{ image } }, textureDesc));
		}

		this->m_SwapChainIndex = 0;
	}

	inline void VulkaneviceManager::VulkanDestroySwapChain(void) {
		if (nullptr != this->m_VulkanDevice)
			this->m_VulkanDevice.waitIdle();

		if (nullptr != this->m_SwapChain) {
			this->m_VulkanDevice.destroySwapchainKHR(this->m_SwapChain);
			this->m_SwapChain = nullptr;
		}

		this->m_SwapChainImages.clear();
		this->m_RHISwapChainImages.clear();
	}

	//Imp

	bool VulkaneviceManager::Imp_CreateInstance(void) {
		if (this->m_DeviceParams.EnableDebugRuntime) {
			this->m_EnabledExtensions.Instance.insert("VK_EXT_debug_report");
			this->m_EnabledExtensions.Layers.insert("VK_LAYER_KHRONOS_validation");
		}

		this->m_DynamicLoader = MakeUnique<decltype(this->m_DynamicLoader)::element_type>(this->m_DeviceParams.VulkanLibraryName);

		::vk::detail::defaultDispatchLoaderDynamic.init(this->m_DynamicLoader->getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"));

		if (!glfwVulkanSupported()) {
			LOG_ERROR("Vulkan is not supported by GLFW. Please ensure that the GLFW library is built with Vulkan support.");

			return false;
		}

		Uint32 glfwExtCount;
		const char** glfwExt{ glfwGetRequiredInstanceExtensions(&glfwExtCount) };

		ASSERT(nullptr != glfwExt);

		for (Uint32 Index = 0; Index < glfwExtCount; ++Index)
			this->m_EnabledExtensions.Instance.insert(String{ glfwExt[Index] });

		// add instance extensions requested by the user
		for (const auto& name : this->m_DeviceParams.RequiredVulkanInstanceExtensions)
			this->m_EnabledExtensions.Instance.insert(name);
		for (const auto& name : this->m_DeviceParams.OptionalVulkanInstanceExtensions)
			this->m_OptionalExtensions.Instance.insert(name);

		// add layers requested by the user
		for (const auto& name : this->m_DeviceParams.RequiredVulkanLayers)
			this->m_EnabledExtensions.Layers.insert(name);
		for (const auto& name : this->m_DeviceParams.OptionalVulkanLayers)
			this->m_OptionalExtensions.Layers.insert(name);

		UnorderedSet<String> requiredExtensions{ this->m_EnabledExtensions.Instance };

		// figure out which optional extensions are supported
		for (const auto& instanceExt : vk::enumerateInstanceExtensionProperties()) {
			const auto& name{ instanceExt.extensionName };
			if (this->m_OptionalExtensions.Instance.find(name) != this->m_OptionalExtensions.Instance.end())
				this->m_EnabledExtensions.Instance.insert(name);

			requiredExtensions.erase(name);
		}

		if (!requiredExtensions.empty()) {
			LOG_ERROR("Cannot create a Vulkan instance because the following required extension(s) are not supported:");

			return false;
		}

		UnorderedSet<String> requiredLayers{ this->m_EnabledExtensions.Layers };

		for (const auto& layer : vk::enumerateInstanceLayerProperties()) {
			const auto name{ layer.layerName };
			if (this->m_OptionalExtensions.Layers.find(name) != this->m_OptionalExtensions.Layers.end())
				this->m_EnabledExtensions.Layers.insert(name);

			requiredLayers.erase(name);
		}

		if (!requiredLayers.empty()) {
			LOG_ERROR("Cannot create a Vulkan instance because the following required layer(s) are not supported:");

			return false;
		}

		auto instanceExtVec{ StringSetToVector(this->m_EnabledExtensions.Instance) };
		auto layerVec{ StringSetToVector(this->m_EnabledExtensions.Layers) };

		vk::ApplicationInfo applicationInfo{};

		// Query the Vulkan API version supported on the system to make sure we use at least 1.3 when that's present.
		VULKAN_CHECK(vk::enumerateInstanceVersion(&applicationInfo.apiVersion));

		// Check if the Vulkan API version is sufficient.
		if (applicationInfo.apiVersion < RHI::Vulkan::MinimumVulkanAPIVersion) {
			LOG_ERROR("Vulkan API version is not supported. Minimum required version is 1.3.0.");

			return false;
		}

		// Spec says: A non-zero variant indicates the API is a variant of the Vulkan API and applications will typically need to be modified to run against it.
		if (/*VK_API_VERSION_VARIANT*/(applicationInfo.apiVersion >> 29U) != 0) {
			LOG_ERROR("Spec says: A non-zero variant indicates the API is a variant of the Vulkan API and applications will typically need to be modified to run against it");

			return false;
		}

		// Create the vulkan instance
		vk::InstanceCreateInfo info{ vk::InstanceCreateInfo{}
			.setEnabledLayerCount(static_cast<Uint32>(layerVec.size()))
			.setPpEnabledLayerNames(layerVec.data())
			.setEnabledExtensionCount(static_cast<Uint32>(instanceExtVec.size()))
			.setPpEnabledExtensionNames(instanceExtVec.data())
			.setPApplicationInfo(&applicationInfo)
		};

		VULKAN_CHECK(vk::createInstance(&info, nullptr, &this->m_VulkanInstance));

		::vk::detail::defaultDispatchLoaderDynamic.init(this->m_VulkanInstance);

		return true;
	}

	bool VulkaneviceManager::Imp_CreateDevice(void) {
		if (this->m_DeviceParams.EnableDebugRuntime)
			this->InstallDebugCallback();

		// add device extensions requested by the user
		for (const auto& name : this->m_DeviceParams.RequiredVulkanDeviceExtensions)
			this->m_EnabledExtensions.Device.insert(name);
		for (const auto& name : this->m_DeviceParams.OptionalVulkanDeviceExtensions)
			this->m_OptionalExtensions.Device.insert(name);

		// Need to adjust the swap chain format before creating the device because it affects physical device selection
		if (this->m_DeviceParams.SwapChainFormat == RHI::RHIFormat::SRGBA8_UNORM)
			this->m_DeviceParams.SwapChainFormat = RHI::RHIFormat::SBGRA8_UNORM;
		else if (this->m_DeviceParams.SwapChainFormat == RHI::RHIFormat::RGBA8_UNORM)
			this->m_DeviceParams.SwapChainFormat = RHI::RHIFormat::BGRA8_UNORM;


		auto Res{ glfwCreateWindowSurface(this->m_VulkanInstance, this->m_Window, nullptr, reinterpret_cast<VkSurfaceKHR*>(&this->m_WindowSurface)) };//TODO :Check if this is correct
		VULKAN_CHECK(vk::Result{ Res });

		for (const auto& device : this->m_VulkanInstance.enumeratePhysicalDevices()) {
			this->m_VulkanPhysicalDevice = device;
			if (device.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
				break;
		}

		{
			auto props{ this->m_VulkanPhysicalDevice.getQueueFamilyProperties() };

			for (Uint32 Index = 0; Index < static_cast<Uint32>(props.size()); ++Index) {
				const auto& queueFamily{ props[Index] };

				if (this->m_GraphicsQueueFamily == Max_Uint32) {
					if (queueFamily.queueCount > 0 &&
						(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics))
						this->m_GraphicsQueueFamily = Index;
				}

				if (this->m_ComputeQueueFamily == Max_Uint32) {
					if (queueFamily.queueCount > 0 &&
						(queueFamily.queueFlags & vk::QueueFlagBits::eCompute) &&
						!(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics))
						this->m_ComputeQueueFamily = Index;
				}

				if (this->m_TransferQueueFamily == Max_Uint32) {
					if (queueFamily.queueCount > 0 &&
						(queueFamily.queueFlags & vk::QueueFlagBits::eTransfer) &&
						!(queueFamily.queueFlags & vk::QueueFlagBits::eCompute) &&
						!(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics))
						this->m_TransferQueueFamily = Index;
				}

				if (this->m_PresentQueueFamily == Max_Uint32) {
					if (queueFamily.queueCount > 0 &&
						glfwGetPhysicalDevicePresentationSupport(this->m_VulkanInstance, this->m_VulkanPhysicalDevice, Index))
						this->m_PresentQueueFamily = Index;
				}
			}

			if (this->m_GraphicsQueueFamily == Max_Uint32 ||
				(this->m_PresentQueueFamily == Max_Uint32) ||
				(this->m_ComputeQueueFamily == Max_Uint32 && this->m_DeviceParams.EnableComputeQueue) ||
				(this->m_TransferQueueFamily == Max_Uint32 && this->m_DeviceParams.EnableCopyQueue))
				return false;

			if (!this->m_VulkanPhysicalDevice.getSurfaceSupportKHR(this->m_PresentQueueFamily, this->m_WindowSurface))
				return false;
		}

		{
			// figure out which optional extensions are supported
			auto deviceExtensions{ this->m_VulkanPhysicalDevice.enumerateDeviceExtensionProperties() };
			for (const auto& ext : deviceExtensions) {
				const String name{ ext.extensionName.data() };
				if (this->m_OptionalExtensions.Device.find(name) != this->m_OptionalExtensions.Device.end())
					this->m_EnabledExtensions.Device.insert(name);
			}

			const vk::PhysicalDeviceProperties physicalDeviceProperties{ this->m_VulkanPhysicalDevice.getProperties() };
			this->m_RendererString = String{ physicalDeviceProperties.deviceName.data() };

			bool meshletsSupported{ false };
			bool synchronization2Supported{ false };
			bool maintenance4Supported{ false };

			for (const auto& ext : this->m_EnabledExtensions.Device) {
				if (ext == "VK_NV_mesh_shader")
					meshletsSupported = true;
				else if (ext == "VK_KHR_synchronization2")
					synchronization2Supported = true;
				else if (ext == "VK_KHR_maintenance4")
					maintenance4Supported = true;
				else if (ext == "VK_KHR_swapchain_mutable_format")
					this->m_SwapChainMutableFormatSupported = true;
			}

			void* pNext{ nullptr };

			vk::PhysicalDeviceFeatures2 physicalDeviceFeatures2;
			// Determine support for Buffer Device Address, the Vulkan 1.2 way
			vk::PhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures;
			// Determine support for maintenance4
			vk::PhysicalDeviceMaintenance4Features maintenance4Features;

			// Put the user-provided extension structure at the end of the chain
			pNext = this->m_DeviceParams.PhysicalDeviceFeatures2Extensions;
			(bufferDeviceAddressFeatures).pNext = pNext;
			pNext = &(bufferDeviceAddressFeatures);
			if (maintenance4Supported) {
				(maintenance4Features).pNext = pNext;
				pNext = &(maintenance4Features);
			};
			physicalDeviceFeatures2.pNext = pNext;
			this->m_VulkanPhysicalDevice.getFeatures2(&physicalDeviceFeatures2);

			UnorderedSet<Uint32> uniqueQueueFamilies{ this->m_GraphicsQueueFamily,this->m_PresentQueueFamily };

			if (this->m_DeviceParams.EnableComputeQueue)
				uniqueQueueFamilies.insert(this->m_ComputeQueueFamily);

			if (this->m_DeviceParams.EnableCopyQueue)
				uniqueQueueFamilies.insert(this->m_TransferQueueFamily);

			float priority{ 1.f };
			Vector<vk::DeviceQueueCreateInfo> queueDesc;
			queueDesc.reserve(uniqueQueueFamilies.size());
			for (int queueFamily : uniqueQueueFamilies)
				queueDesc.emplace_back(vk::DeviceQueueCreateInfo{}
					.setQueueFamilyIndex(queueFamily)
					.setQueueCount(1)
					.setPQueuePriorities(&priority)
				);

			auto meshletFeatures = vk::PhysicalDeviceMeshShaderFeaturesNV()
				.setTaskShader(true)
				.setMeshShader(true);

			auto vulkan13features = vk::PhysicalDeviceVulkan13Features()
				.setSynchronization2(synchronization2Supported)
				.setMaintenance4(maintenance4Features.maintenance4);

			pNext = nullptr;
			if (meshletsSupported) {
				(meshletFeatures).pNext = pNext;
				pNext = &(meshletFeatures);
			}
			if (physicalDeviceProperties.apiVersion >= RHI::Vulkan::MinimumVulkanAPIVersion) {
				(vulkan13features).pNext = pNext;
				pNext = &(vulkan13features);
			}
			if (physicalDeviceProperties.apiVersion < RHI::Vulkan::MinimumVulkanAPIVersion && maintenance4Supported) {
				(maintenance4Features).pNext = pNext;
				pNext = &(maintenance4Features);
			};

			auto deviceFeatures{ vk::PhysicalDeviceFeatures{}
				.setShaderImageGatherExtended(true)
				.setSamplerAnisotropy(true)
				.setTessellationShader(true)
				.setTextureCompressionBC(true)
				.setGeometryShader(true)
				.setImageCubeArray(true)
				.setShaderInt16(true)
				.setFillModeNonSolid(true)
				.setFragmentStoresAndAtomics(true)
				.setDualSrcBlend(true)
				.setVertexPipelineStoresAndAtomics(true)
			};

			// Add a Vulkan 1.1 structure with default settings to make it easier for apps to modify them
			auto vulkan11features{ vk::PhysicalDeviceVulkan11Features{}
				.setPNext(pNext)
			};

			auto vulkan12features{ vk::PhysicalDeviceVulkan12Features{}
				.setDescriptorIndexing(true)
				.setRuntimeDescriptorArray(true)
				.setDescriptorBindingPartiallyBound(true)
				.setDescriptorBindingVariableDescriptorCount(true)
				.setTimelineSemaphore(true)
				.setShaderSampledImageArrayNonUniformIndexing(true)
				.setBufferDeviceAddress(true)
				.setPNext(&vulkan11features)
			};

			auto layerVec{ StringSetToVector(this->m_EnabledExtensions.Layers) };
			auto extVec{ StringSetToVector(this->m_EnabledExtensions.Device) };

			auto deviceDesc{ vk::DeviceCreateInfo{}
				.setPQueueCreateInfos(queueDesc.data())
				.setQueueCreateInfoCount(static_cast<Uint32>(queueDesc.size()))
				.setPEnabledFeatures(&deviceFeatures)
				.setEnabledExtensionCount(static_cast<Uint32>(extVec.size()))
				.setPpEnabledExtensionNames(extVec.data())
				.setEnabledLayerCount(static_cast<Uint32>(layerVec.size()))
				.setPpEnabledLayerNames(layerVec.data())
				.setPNext(&vulkan12features)
			};

			VULKAN_CHECK(this->m_VulkanPhysicalDevice.createDevice(&deviceDesc, nullptr, &this->m_VulkanDevice));

			this->m_VulkanDevice.getQueue(this->m_GraphicsQueueFamily, 0, &this->m_GraphicsQueue);
			this->m_VulkanDevice.getQueue(this->m_PresentQueueFamily, 0, &this->m_PresentQueue);
			if (this->m_DeviceParams.EnableComputeQueue)
				this->m_VulkanDevice.getQueue(this->m_ComputeQueueFamily, 0, &this->m_ComputeQueue);
			if (this->m_DeviceParams.EnableCopyQueue)
				this->m_VulkanDevice.getQueue(this->m_TransferQueueFamily, 0, &this->m_TransferQueue);
			::vk::detail::defaultDispatchLoaderDynamic.init(this->m_VulkanDevice);
		}

		auto vecInstanceExt{ StringSetToVector(this->m_EnabledExtensions.Instance) };
		auto vecDeviceExt{ StringSetToVector(this->m_EnabledExtensions.Device) };

		RHI::Vulkan::DeviceDesc deviceDesc{
			.Instance{ this->m_VulkanInstance },
			.PhysicalDevice{ this->m_VulkanPhysicalDevice },
			.Device{ this->m_VulkanDevice },
			.GraphicsQueue{ this->m_GraphicsQueue },
			.GraphicsQueueIndex{ this->m_GraphicsQueueFamily },
		};

		if (this->m_DeviceParams.EnableComputeQueue) {
			deviceDesc.ComputeQueue = this->m_ComputeQueue;
			deviceDesc.ComputeQueueIndex = this->m_ComputeQueueFamily;
		}
		if (this->m_DeviceParams.EnableCopyQueue) {
			deviceDesc.TransferQueue = this->m_TransferQueue;
			deviceDesc.TransferQueueIndex = this->m_TransferQueueFamily;
		}
		deviceDesc.InstanceExtensions = vecInstanceExt.data();
		deviceDesc.NumInstanceExtensions = vecInstanceExt.size();
		deviceDesc.DeviceExtensions = vecDeviceExt.data();
		deviceDesc.NumDeviceExtensions = vecDeviceExt.size();

		this->m_RHIDevice = RHI::RefCountPtr<Imp_Device>::Create(new Imp_Device{ deviceDesc });

		return true;
	}

	bool VulkaneviceManager::Imp_CreateSwapChain(void) {
		this->VulkanCreateSwapChain();

		this->m_PresentSemaphores.reserve(static_cast<Uint64>(this->m_DeviceParams.MaxFramesInFlight) + 1u);
		this->m_AcquireSemaphores.reserve(static_cast<Uint64>(this->m_DeviceParams.MaxFramesInFlight) + 1u);
		for (Uint32 Index = 0; Index < this->m_DeviceParams.MaxFramesInFlight + 1; ++Index) {
			this->m_PresentSemaphores.push_back(this->m_VulkanDevice.createSemaphore(vk::SemaphoreCreateInfo{}));
			this->m_AcquireSemaphores.push_back(this->m_VulkanDevice.createSemaphore(vk::SemaphoreCreateInfo{}));
		}

		return true;
	}

	void VulkaneviceManager::Imp_ResizeSwapChain(void) {
		if (nullptr != this->m_VulkanDevice) {
			this->VulkanDestroySwapChain();
			this->VulkanCreateSwapChain();
		}
	}

	bool VulkaneviceManager::Imp_BeginFrame(void) {
		const auto& semaphore{ this->m_AcquireSemaphores[this->m_AcquireSemaphoreIndex] };

		constexpr Uint32 maxAttempts{ 3 };

		vk::Result res{};
		for (Uint32 attempt = 0; attempt < maxAttempts; ++attempt) {
			res = this->m_VulkanDevice.acquireNextImageKHR(this->m_SwapChain, Max_Uint64, semaphore, vk::Fence{}, & this->m_SwapChainIndex);

			if ((res == vk::Result::eErrorOutOfDateKHR || res == vk::Result::eSuboptimalKHR) && attempt < maxAttempts) {
				this->BackBufferResizing();
				auto surfaceCaps{ this->m_VulkanPhysicalDevice.getSurfaceCapabilitiesKHR(this->m_WindowSurface) };

				this->m_DeviceParams.BackBufferWidth = surfaceCaps.currentExtent.width;
				this->m_DeviceParams.BackBufferHeight = surfaceCaps.currentExtent.height;

				this->ResizeSwapChain();
				this->BackBufferResized();
			}
			else
				break;
		}

		this->m_AcquireSemaphoreIndex = (this->m_AcquireSemaphoreIndex + 1) % this->m_AcquireSemaphores.size();

		if (res == vk::Result::eSuccess || res == vk::Result::eSuboptimalKHR) {// Suboptimal is considered a success
			// Schedule the wait. The actual wait operation will be submitted when the app executes any command list.
			this->m_RHIDevice->QueueWaitForSemaphore(RHI::RHICommandQueue::Graphics, semaphore, 0);
			return true;
		}

		return false;
	}

	void VulkaneviceManager::Imp_DestroyDeviceAndSwapChain(void) {
		this->VulkanDestroySwapChain();

		for (auto& semaphore : this->m_PresentSemaphores) {
			if (nullptr != semaphore) {
				this->m_VulkanDevice.destroySemaphore(semaphore);
				semaphore = nullptr;
			}
		}

		for (auto& semaphore : this->m_AcquireSemaphores) {
			if (nullptr != semaphore) {
				this->m_VulkanDevice.destroySemaphore(semaphore);
				semaphore = nullptr;
			}
		}

		this->m_RHIDevice = nullptr;
		this->m_RendererString.clear();

		if (nullptr != this->m_VulkanDevice) {
			this->m_VulkanDevice.destroy();
			this->m_VulkanDevice = nullptr;
		}

		if (nullptr != this->m_WindowSurface) {
			ASSERT(nullptr != m_VulkanInstance);

			this->m_VulkanInstance.destroySurfaceKHR(this->m_WindowSurface);
			this->m_WindowSurface = nullptr;
		}

		if (this->m_DebugReportCallback)
			this->m_VulkanInstance.destroyDebugReportCallbackEXT(this->m_DebugReportCallback);

		if (this->m_VulkanInstance) {
			this->m_VulkanInstance.destroy();
			this->m_VulkanInstance = nullptr;
		}
	}

	bool VulkaneviceManager::Imp_Present(void) {
		const auto& semaphore{ this->m_PresentSemaphores[this->m_PresentSemaphoreIndex] };

		this->m_RHIDevice->QueueSignalSemaphore(RHI::RHICommandQueue::Graphics, semaphore, 0);

		// RHI buffers the semaphores and signals them when something is submitted to a queue.
		// Call 'executeCommandLists' with no command lists to actually signal the semaphore.
		this->m_RHIDevice->ExecuteCommandLists(nullptr, 0);

		vk::PresentInfoKHR info{ vk::PresentInfoKHR{}
			.setWaitSemaphoreCount(1)
			.setPWaitSemaphores(&semaphore)
			.setSwapchainCount(1)
			.setPSwapchains(&this->m_SwapChain)
			.setPImageIndices(&this->m_SwapChainIndex)
		};

		VULKAN_CHECK(this->m_PresentQueue.presentKHR(&info));

		this->m_PresentSemaphoreIndex = (this->m_PresentSemaphoreIndex + 1) % this->m_PresentSemaphores.size();

#ifndef VULKAN_PLATFORM_WIN32
		// according to vulkan-tutorial.com, "the validation layer implementation expects
		// the application to explicitly synchronize with the GPU"
		if (this->m_DeviceParams.VsyncEnabled || this->m_DeviceParams.EnableDebugRuntime)
			this->m_PresentQueue.WaitIdle();
#endif
		while (static_cast<Uint32>(this->m_FramesInFlight.size()) >= this->m_DeviceParams.MaxFramesInFlight) {
			auto query{ this->m_FramesInFlight.front() };
			this->m_FramesInFlight.pop();

			this->m_RHIDevice->WaitEventQuery(query);

			this->m_QueryPool.push_back(query);
		}

		RHI::RefCountPtr<Imp_EventQuery> query;
		if (!this->m_QueryPool.empty()) {
			query = this->m_QueryPool.back();
			this->m_QueryPool.pop_back();
		}
		else
			query = this->m_RHIDevice->CreateEventQuery();

		this->m_RHIDevice->ResetEventQuery(query);
		this->m_RHIDevice->SetEventQuery(query, RHI::RHICommandQueue::Graphics);
		this->m_FramesInFlight.push(query);
		return true;
	}

	void VulkaneviceManager::Imp_Shutdown(void) {
		for (auto& e : RHI::Vulkan::g_CommandBuffersInFlight)
			e.clear();
		for(auto& e : RHI::Vulkan::g_CommandBuffersPool)
			e.clear();

	}
}