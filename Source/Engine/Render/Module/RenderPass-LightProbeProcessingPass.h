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

#include "Engine/Render/Module/GBuffer.h"

#include "Shader/light_probe_cb.h"

#include "Engine/Engine/Module/TextureCache.h"//TODO GenMipmap

#endif // PARTING_MODULE_BUILD

namespace Parting {

	template<RHI::APITagConcept APITag>
	class LightProbeProcessingPass final {
		using Imp_Device = typename RHI::RHITypeTraits<APITag>::Imp_Device;
		using Imp_CommandList = typename RHI::RHITypeTraits<APITag>::Imp_CommandList;
		using Imp_Shader = typename RHI::RHITypeTraits<APITag>::Imp_Shader;
		using Imp_FrameBuffer = typename RHI::RHITypeTraits<APITag>::Imp_FrameBuffer;
		using Imp_Buffer = typename RHI::RHITypeTraits<APITag>::Imp_Buffer;
		using Imp_Texture = typename RHI::RHITypeTraits<APITag>::Imp_Texture;
		using Imp_Heap = typename RHI::RHITypeTraits<APITag>::Imp_Heap;
		using Imp_Sampler = typename RHI::RHITypeTraits<APITag>::Imp_Sampler;
		using Imp_BindingSet = typename RHI::RHITypeTraits<APITag>::Imp_BindingSet;
		using Imp_BindingLayout = typename RHI::RHITypeTraits<APITag>::Imp_BindingLayout;
		using Imp_GraphicsPipeline = typename RHI::RHITypeTraits<APITag>::Imp_GraphicsPipeline;

	public:
		LightProbeProcessingPass(
			Imp_Device* device,
			SharedPtr<ShaderFactory<APITag>> shaderFactory,
			SharedPtr<CommonRenderPasses<APITag>> commonPasses,
			Uint32 intermediateTextureSize = 1024,
			RHI::RHIFormat intermediateTextureFormat = RHI::RHIFormat::RGBA16_FLOAT
		);
		~LightProbeProcessingPass(void) = default;


	private:
		RHI::RefCountPtr<Imp_Device> m_Device;
		RHI::RefCountPtr<Imp_Shader> m_GeometryShader;
		RHI::RefCountPtr<Imp_Shader> m_MipPixelShader;
		RHI::RefCountPtr<Imp_Shader> m_DiffusePixelShader;
		RHI::RefCountPtr<Imp_Shader> m_SpecularPixelShader;
		RHI::RefCountPtr<Imp_Shader> m_EnvironmentBrdfPixelShader;
		RHI::RefCountPtr<Imp_Buffer> m_LightProbeCB;

		RHI::RefCountPtr<Imp_BindingLayout> m_BindingLayout;

		RHI::RefCountPtr<Imp_Texture> m_IntermediateTexture;
		Uint32 m_IntermediateTextureSize;

		RHI::RefCountPtr<Imp_Texture> m_EnvironmentBrdfTexture;
		Uint32 m_EnvironmentBrdfTextureSize{ 64 };

		SharedPtr<CommonRenderPasses<APITag>> m_CommonPasses;

		UnorderedMap<RHI::RHIFrameBufferInfo<APITag>, RHI::RefCountPtr<Imp_GraphicsPipeline>, typename RHI::RHIFrameBufferInfo<APITag>::RHIFrameBufferInfoHash> m_BLITPSOCache;
		UnorderedMap<RHI::RHIFrameBufferInfo<APITag>, RHI::RefCountPtr<Imp_GraphicsPipeline>, typename RHI::RHIFrameBufferInfo<APITag>::RHIFrameBufferInfoHash> m_DiffusePSOCache;
		UnorderedMap<RHI::RHIFrameBufferInfo<APITag>, RHI::RefCountPtr<Imp_GraphicsPipeline>, typename RHI::RHIFrameBufferInfo<APITag>::RHIFrameBufferInfoHash> m_SpecularPSOCache;

		UnorderedMap<RHI::RHITextureSubresourcesKey<APITag>, RHI::RefCountPtr<Imp_FrameBuffer>, typename RHI::RHITextureSubresourcesKey<APITag>::Hash> m_FrameBufferCache;
		UnorderedMap<RHI::RHITextureSubresourcesKey<APITag>, RHI::RefCountPtr<Imp_BindingSet>, typename RHI::RHITextureSubresourcesKey<APITag>::Hash> m_BindingSetCache;


	};







	template<RHI::APITagConcept APITag>
	inline LightProbeProcessingPass<APITag>::LightProbeProcessingPass(Imp_Device* device, SharedPtr<ShaderFactory<APITag>> shaderFactory, SharedPtr<CommonRenderPasses<APITag>> commonPasses, Uint32 intermediateTextureSize, RHI::RHIFormat intermediateTextureFormat) :
		m_Device{ device },
		m_IntermediateTextureSize{ intermediateTextureSize },
		m_CommonPasses{ commonPasses } {

		ASSERT(intermediateTextureSize > 0);

		this->m_GeometryShader = shaderFactory->CreateShader("Parting/Passes/light_probe.hlsl", "cubemap_gs", nullptr, RHI::RHIShaderType::Geometry);
		this->m_MipPixelShader = shaderFactory->CreateShader("Parting/Passes/light_probe.hlsl", "mip_ps", nullptr, RHI::RHIShaderType::Pixel);
		this->m_DiffusePixelShader = shaderFactory->CreateShader("Parting/Passes/light_probe.hlsl", "diffuse_probe_ps", nullptr, RHI::RHIShaderType::Pixel);
		this->m_SpecularPixelShader = shaderFactory->CreateShader("Parting/Passes/light_probe.hlsl", "specular_probe_ps", nullptr, RHI::RHIShaderType::Pixel);
		this->m_EnvironmentBrdfPixelShader = shaderFactory->CreateShader("Parting/Passes/light_probe.hlsl", "environment_brdf_ps", nullptr, RHI::RHIShaderType::Pixel);

		this->m_BindingLayout = device->CreateBindingLayout(RHI::RHIBindingLayoutDescBuilder{}
			.Set_Visibility(RHI::RHIShaderType::Pixel)
			.AddBinding(RHI::RHIBindingLayoutItem::VolatileConstantBuffer(0))
			.AddBinding(RHI::RHIBindingLayoutItem::Sampler(0))
			.AddBinding(RHI::RHIBindingLayoutItem::Texture_SRV(0))
			.Build()
		);

		this->m_LightProbeCB = device->CreateBuffer(RHI::RHIBufferDescBuilder{}
			.Set_ByteSize(sizeof(LightProbeProcessingConstants))
			.Set_MaxVersions(64)//TODO :
			.Set_DebugName(_W("LightProbeProcessingConstants"))
			.Set_IsConstantBuffer(true)
			.Set_IsVolatile(true)
			.Build()
		);

		RHI::RHITextureDescBuilder textureDescBuilder{}; textureDescBuilder
			.Set_IsRenderTarget(true)
			.Set_ClearValue(Color{ 0.f })
			.Set_KeepInitialState(true);

		this->m_IntermediateTexture = this->m_Device->CreateTexture(textureDescBuilder
			.Set_Width(intermediateTextureSize).Set_Height(intermediateTextureSize)
			.Set_ArraySize(6)
			.Set_MipLevels(Parting::GetMipLevelsNum(intermediateTextureSize, intermediateTextureSize))
			.Set_Format(intermediateTextureFormat)
			.Set_Dimension(RHI::RHITextureDimension::TextureCube)
			.Set_DebugName("LightProbeIntermediate")
			.Set_InitialState(RHI::RHIResourceState::RenderTarget)
			.Build()
		);

		this->m_EnvironmentBrdfTexture = this->m_Device->CreateTexture(textureDescBuilder
			.Set_Width(this->m_EnvironmentBrdfTextureSize).Set_Height(this->m_EnvironmentBrdfTextureSize)
			.Set_ArraySize(1)
			.Set_MipLevels(1)
			.Set_Format(RHI::RHIFormat::RG16_FLOAT)
			.Set_Dimension(RHI::RHITextureDimension::Texture2D)
			.Set_DebugName("LightProbeBrdf")
			.Set_InitialState(RHI::RHIResourceState::ShaderResource)
			.Build()
		);
	}

}