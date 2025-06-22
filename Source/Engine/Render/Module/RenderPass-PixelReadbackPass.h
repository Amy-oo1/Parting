#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"


PARTING_IMPORT Platform;
PARTING_IMPORT Utility;
PARTING_IMPORT Algorithm;
PARTING_IMPORT Container;
PARTING_IMPORT VectorMath;
PARTING_IMPORT Logger;


PARTING_SUBMODULE(Parting, SSAOPass)


#else
#pragma once

#include "Core/ModuleBuild.h"


#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"
//Global

#include<random>

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Algorithm/Module/Algorithm.h"
#include "Core/Container/Module/Container.h"
#include "Core/String/Module/String.h"
#include "Core/VectorMath/Module/VectorMath.h"
#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI.h"
#include "D3D12RHI/Module/D3D12RHI.h"

#include "Engine/Render/Module/RenderPass-Base.h"

#include "Engine/Render/Module/SceneTypes.h"
#include "Engine/Engine/Module/SceneGraph.h"

#include "Engine/Engine/Module/CommonRenderPasses.h"
#include "Engine/Engine/Module/FrameBufferFactory.h"
#include "Engine/Engine/Module/ShaderFactory.h"
#include "Engine/Render/Module/MaterialBindingCache.h"
#include "Engine/Render/Module/View.h"

#include "Shader/pixel_readback_cb.h"



#endif // PARTING_MODULE_BUILD


namespace Parting {

	template<RHI::APITagConcept APITag>
	class PixelReadbackPass final {
		using Imp_Device = typename RHI::RHITypeTraits<APITag>::Imp_Device;
		using Imp_CommandList = typename RHI::RHITypeTraits<APITag>::Imp_CommandList;
		using Imp_Shader = typename RHI::RHITypeTraits<APITag>::Imp_Shader;
		using Imp_InputLayout = typename RHI::RHITypeTraits<APITag>::Imp_InputLayout;
		using Imp_Texture = typename RHI::RHITypeTraits<APITag>::Imp_Texture;
		using Imp_Heap = typename RHI::RHITypeTraits<APITag>::Imp_Heap;
		using Imp_Buffer = typename RHI::RHITypeTraits<APITag>::Imp_Buffer;
		using Imp_BindingLayout = typename RHI::RHITypeTraits<APITag>::Imp_BindingLayout;
		using Imp_BindingSet = typename RHI::RHITypeTraits<APITag>::Imp_BindingSet;
		using Imp_GraphicsPipeline = typename RHI::RHITypeTraits<APITag>::Imp_GraphicsPipeline;
		using Imp_ComputePipeline = typename RHI::RHITypeTraits<APITag>::Imp_ComputePipeline;
		using Imp_FrameBuffer = typename RHI::RHITypeTraits<APITag>::Imp_FrameBuffer;
		using Imp_Sampler = typename RHI::RHITypeTraits<APITag>::Imp_Sampler;

	public:
		PixelReadbackPass(
			Imp_Device* device,
			SharedPtr<ShaderFactory<APITag>> shaderFactory,
			Imp_Texture* inputTexture,
			RHI::RHIFormat format,
			Uint32 arraySlice = 0,
			Uint32 mipLevel = 0
		);

		~PixelReadbackPass(void) = default;

	public:
		void Capture(Imp_CommandList* commandList, Math::VecU2 pixelPosition);

	private:



	private:
		RHI::RefCountPtr<Imp_Device> m_Device;
		RHI::RefCountPtr<Imp_Shader> m_Shader;
		RHI::RefCountPtr<Imp_ComputePipeline> m_Pipeline;
		RHI::RefCountPtr<Imp_BindingLayout> m_BindingLayout;
		RHI::RefCountPtr<Imp_BindingSet> m_BindingSet;
		RHI::RefCountPtr<Imp_Buffer> m_ConstantBuffer;
		RHI::RefCountPtr<Imp_Buffer> m_IntermediateBuffer;
		RHI::RefCountPtr<Imp_Buffer> m_ReadbackBuffer;

	};











	template<RHI::APITagConcept APITag>
	inline PixelReadbackPass<APITag>::PixelReadbackPass(Imp_Device* device, SharedPtr<ShaderFactory<APITag>> shaderFactory, Imp_Texture* inputTexture, RHI::RHIFormat format, Uint32 arraySlice, Uint32 mipLevel) :
		m_Device{ device } {

		String formatName;
		switch (format) {
			using enum RHI::RHIFormat;
		case RGBA32_FLOAT: formatName = "float4"; break;
		case RGBA32_UINT: formatName = "uint4"; break;
		case RGBA32_SINT: formatName = "int4"; break;
		default: LOG_ERROR("unsupported readback format");
		}

		Vector<ShaderMacro> macros{
			ShaderMacro{.Name{ String{ "TYPE" } }, .Definition{ formatName } },
			ShaderMacro{.Name{ String{ "INPUT_MSAA" } }, .Definition{ inputTexture->Get_Desc().SampleCount > 1 ? "1" : "0" } },
		};
		this->m_Shader = shaderFactory->CreateShader("Parting/Passes/pixel_readback_cs.hlsl", "main", &macros, RHI::RHIShaderType::Compute);

		RHI::RHIBufferDescBuilder bufferDescBuilder{}; bufferDescBuilder
			.Set_ByteSize(16)
			.Set_Format(format)
			.Set_CanHaveTypedViews(true)
			.Set_InitialState(RHI::RHIResourceState::CopySource)
			.Set_KeepInitialState(true);

		this->m_IntermediateBuffer = this->m_Device->CreateBuffer(bufferDescBuilder
			.Set_DebugName(_W("PixelReadbackPass/IntermediateBuffer"))
			.Set_CanHaveUAVs(true)
			.Set_CPUAccess(RHI::RHICPUAccessMode::None)
			.Build()
		);

		this->m_ReadbackBuffer = this->m_Device->CreateBuffer(bufferDescBuilder
			.Set_DebugName(_W("PixelReadbackPass/ReadbackBuffer"))
			.Set_CanHaveUAVs(false)
			.Set_CPUAccess(RHI::RHICPUAccessMode::Read)
			.Build()
		);

		this->m_ConstantBuffer = this->m_Device->CreateBuffer(bufferDescBuilder
			.Reset()
			.Set_ByteSize(sizeof(Shader::PixelReadbackConstants))
			.Set_MaxVersions(Parting::c_MaxRenderPassConstantBufferVersions)
			.Set_DebugName(_W("PixelReadbackPass/Constants"))
			.Set_IsConstantBuffer(true)
			.Set_IsVolatile(true)
			.Build()
		);

		::Tie(this->m_BindingLayout, this->m_BindingSet) = this->m_Device->CreateBindingLayoutAndSet(
			RHI::RHIShaderType::Compute,
			0,
			RHI::RHIBindingSetDescBuilder<APITag>{}
		.AddBinding(RHI::RHIBindingSetItem<APITag>::ConstantBuffer(0, this->m_ConstantBuffer.Get()))
			.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(0, inputTexture, RHI::RHIFormat::UNKNOWN, RHI::RHITextureSubresourceSet{ .BaseMipLevel{ mipLevel }, .BaseArraySlice{ arraySlice } }))
			.AddBinding(RHI::RHIBindingSetItem<APITag>::TypedBuffer_UAV(0, this->m_IntermediateBuffer.Get()))
			.Build()
			);

		this->m_Pipeline = this->m_Device->CreateComputePipeline(RHI::RHIComputePipelineDescBuilder<APITag>{}
		.Set_CS(this->m_Shader)
			.AddBindingLayout(this->m_BindingLayout)
			.Build()
			);
	}

	template<RHI::APITagConcept APITag>
	inline void PixelReadbackPass<APITag>::Capture(Imp_CommandList* commandList, Math::VecU2 pixelPosition) {
		Shader::PixelReadbackConstants constants{};
		constants.PixelPosition = Math::VecI2{ pixelPosition };
		commandList->WriteBuffer(this->m_ConstantBuffer, &constants, sizeof(constants));

		commandList->SetComputeState(RHI::RHIComputeStateBuilder<APITag>{}
		.Set_Pipeline(this->m_Pipeline)
			.AddBindingSet(this->m_BindingSet)
			.Build()
			);
		commandList->Dispatch(1, 1, 1);

		commandList->CopyBuffer(this->m_ReadbackBuffer, 0, this->m_IntermediateBuffer, 0, this->m_ReadbackBuffer->Get_Desc().ByteSize);
	}

}