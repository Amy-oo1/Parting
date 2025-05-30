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
	struct D3D12Tag {};

	namespace D3D12 {
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

	template<>
	struct RHITypeTraits<D3D12Tag> {
		static constexpr auto APIName{ "D3D12" };
		static constexpr auto ShaderType{ "HLSL" };

		using Imp_EventQuery = D3D12::EventQuery;
		using Imp_TimerQuery = D3D12::TimerQuery;

		using Imp_Heap = D3D12::Heap;
		
		using Imp_Buffer = D3D12::Buffer;

		using Imp_Texture = D3D12::Texture;
		using Imp_StagingTexture = D3D12::StagingTexture;

		using Imp_Sampler = D3D12::Sampler;

		using Imp_InputLayout = D3D12::InputLayout;

		using Imp_Shader = D3D12::Shader;

		using Imp_FrameBuffer = D3D12::FrameBuffer;

		using Imp_BindingLayout = D3D12::BindingLayout;

		using Imp_BindingSet = D3D12::BindingSet;

		using Imp_GraphicsPipeline = D3D12::GraphicsPipeline;
		using Imp_ComputePipeline = D3D12::ComputePipeline;
		using Imp_MeshletPipeline = D3D12::MeshletPipeline;

		using Imp_CommandList = D3D12::CommandList;

		using Imp_Device = D3D12::Device;
	};
}


namespace RHI::D3D12 {

}