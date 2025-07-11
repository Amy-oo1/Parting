#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"

PARTING_SUBMODULE(D3D12RHI, Traits)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;

#else
#pragma once

#include "Core/ModuleBuild.h"

#include "Core/Utility/Include/UtilityMacros.h"
//Global
#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "RHI/Module/RHI.h"


#endif // PARTING_MODULE_BUILD


//NOTE :Forward declaration this be sorted althoug maybe equal
namespace RHI {
	namespace Vulkan {
		class MessageCallback;

		class EventQuery;
		class TimerQuery;

		class Heap;

		class Buffer;

		class Texture;
		class StagingTexture;

		class Sampler;

		class InputLayout;

		class Shader;

		class FrameBuffer;

		class BindingLayout;

		class BindingSet;

		class GraphicsPipeline;
		class ComputePipeline;
		class MeshletPipeline;

		class CommandList;

		class Device;
	}

	template<> struct RHITypeTraits<VulkanTag> {
		static constexpr auto APIName{ "Vulkan" };
		static constexpr auto ShaderType{ "Spirv" };

		using Imp_EventQuery = Vulkan::EventQuery;
		using Imp_TimerQuery = Vulkan::TimerQuery;

		using Imp_Heap = Vulkan::Heap;

		using Imp_Buffer = Vulkan::Buffer;

		using Imp_Texture = Vulkan::Texture;
		using Imp_StagingTexture = Vulkan::StagingTexture;

		using Imp_Sampler = Vulkan::Sampler;

		using Imp_InputLayout = Vulkan::InputLayout;

		using Imp_Shader = Vulkan::Shader;

		using Imp_FrameBuffer = Vulkan::FrameBuffer;

		using Imp_BindingLayout = Vulkan::BindingLayout;

		using Imp_BindingSet = Vulkan::BindingSet;

		using Imp_GraphicsPipeline = Vulkan::GraphicsPipeline;
		using Imp_ComputePipeline = Vulkan::ComputePipeline;
		using Imp_MeshletPipeline = Vulkan::MeshletPipeline;

		using Imp_CommandList = Vulkan::CommandList;

		using Imp_Device = Vulkan::Device;
	};
}