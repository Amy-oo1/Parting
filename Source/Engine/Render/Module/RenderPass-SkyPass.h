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

#include "Shader/sky_cb.h"

#endif // PARTING_MODULE_BUILD


namespace Parting {

	namespace _NameSpace_SkyPass {
		struct Parameters final {
			Math::VecF3 SkyColor{ 0.17f, 0.37f, 0.65f };
			Math::VecF3 HorizonColor{ 0.50f, 0.70f, 0.92f };
			Math::VecF3 GroundColor{ 0.62f, 0.59f, 0.55f };
			Math::VecF3 DirectionUp{ 0.f, 1.f, 0.f };
			float Brightness{ 0.1f }; // scaler for sky brightness
			float HorizonSize{ 30.f }; // +/- degrees
			float GlowSize{ 5.f }; // degrees, starting from the edge of the light disk
			float GlowIntensity{ 0.1f }; // [0-1] relative to light intensity
			float GlowSharpness{ 4.f }; // [1-10] is the glow power exponent
			float MaxLightRadiance{ 100.f }; // clamp for light radiance derived from its angular size, 0 = no clamp
		};
	}

	template<RHI::APITagConcept APITag>
	class SkyPass final {
		using Imp_Device = typename RHI::RHITypeTraits<APITag>::Imp_Device;
		using Imp_CommandList = typename RHI::RHITypeTraits<APITag>::Imp_CommandList;
		using Imp_Texture = typename RHI::RHITypeTraits<APITag>::Imp_Texture;
		using Imp_Buffer = typename RHI::RHITypeTraits<APITag>::Imp_Buffer;
		using Imp_Heap = typename RHI::RHITypeTraits<APITag>::Imp_Heap;
		using Imp_Shader = typename RHI::RHITypeTraits<APITag>::Imp_Shader;
		using Imp_BindingLayout = typename RHI::RHITypeTraits<APITag>::Imp_BindingLayout;
		using Imp_BindingSet = typename RHI::RHITypeTraits<APITag>::Imp_BindingSet;
		using Imp_GraphicsPipeline = typename RHI::RHITypeTraits<APITag>::Imp_GraphicsPipeline;
		using Imp_FrameBuffer = typename RHI::RHITypeTraits<APITag>::Imp_FrameBuffer;
		using Imp_Sampler = typename RHI::RHITypeTraits<APITag>::Imp_Sampler;

	public:
		using Parameters = _NameSpace_SkyPass::Parameters;

	public:
		SkyPass(Imp_Device* device, const SharedPtr<ShaderFactory<APITag>>& shaderFactory, const SharedPtr<CommonRenderPasses<APITag>>& commonPasses, const SharedPtr<FrameBufferFactory<APITag>>& framebufferFactory, const ICompositeView& compositeView);

		~SkyPass(void) = default;


	public:
		void Render(Imp_CommandList* commandList, const ICompositeView& compositeView, const DirectionalLight<APITag>& light, const SkyPass<APITag>::Parameters& params);

	private:
		void FillShaderParameters(const DirectionalLight<APITag>& light, const SkyPass<APITag>::Parameters& input, ::ProceduralSkyShaderParameters& output);


	private:
		RHI::RefCountPtr<Imp_Shader> m_PixelShader;
		RHI::RefCountPtr<Imp_Buffer> m_SkyCB;
		RHI::RefCountPtr<Imp_BindingLayout> m_RenderBindingLayout;
		RHI::RefCountPtr<Imp_BindingSet> m_RenderBindingSet;
		RHI::RefCountPtr<Imp_GraphicsPipeline> m_PSO;

		SharedPtr<FrameBufferFactory<APITag>> m_FrameBufferFactory;



	};



	template<RHI::APITagConcept APITag>
	inline SkyPass<APITag>::SkyPass(Imp_Device* device, const SharedPtr<ShaderFactory<APITag>>& shaderFactory, const SharedPtr<CommonRenderPasses<APITag>>& commonPasses, const SharedPtr<FrameBufferFactory<APITag>>& framebufferFactory, const ICompositeView& compositeView) :
		m_FrameBufferFactory{ framebufferFactory } {
		this->m_PixelShader = shaderFactory->CreateShader("Parting/Passes/sky_ps.hlsl", "main", nullptr, RHI::RHIShaderType::Pixel);

		this->m_SkyCB = device->CreateBuffer(RHI::RHIBufferDescBuilder{}
			.Set_ByteSize(sizeof(SkyConstants))
			.Set_MaxVersions(c_MaxRenderPassConstantBufferVersions)
			.Set_DebugName(_W("SkyPass/SkyConstants"))
			.Set_IsConstantBuffer(true)
			.Set_IsVolatile(true)
			.Build()
		);

		::Tie(this->m_RenderBindingLayout, this->m_RenderBindingSet) = device->CreateBindingLayoutAndSet(
			RHI::RHIShaderType::Pixel,
			0,
			RHI::RHIBindingSetDescBuilder<APITag>{}
		.AddBinding(RHI::RHIBindingSetItem<APITag>::ConstantBuffer(0, this->m_SkyCB.Get()))
			.Build()
			);

		const IView* sampleView{ compositeView.Get_ChildView(ViewType::PLANAR, 0) };
		Imp_FrameBuffer* sampleFramebuffer{ this->m_FrameBufferFactory->Get_FrameBuffer(*sampleView) };

		RHI::RHIGraphicsPipelineDescBuilder<APITag> pipelineDescBuilder{}; pipelineDescBuilder
			.Set_PrimType(RHI::RHIPrimitiveType::TriangleStrip)
			.Set_VS(sampleView->Is_ReverseDepth() ? commonPasses->m_FullscreenVS.Get() : commonPasses->m_FullscreenAtOneVS.Get())
			.Set_PS(this->m_PixelShader.Get())
			.AddBindingLayout(this->m_RenderBindingLayout)
			.Set_CullMode(RHI::RHIRasterCullMode::None)
			/*.Set_DepthTestEnable(true)*///NOTE :Default
			.Set_DepthWriteEnable(false)
			.Set_DepthFunc(sampleView->Is_ReverseDepth() ? RHI::RHIComparisonFunc::GreaterOrEqual : RHI::RHIComparisonFunc::LessOrEqual);

		this->m_PSO = device->CreateGraphicsPipeline(pipelineDescBuilder.Build(), sampleFramebuffer);
	}

	template<RHI::APITagConcept APITag>
	inline void SkyPass<APITag>::Render(Imp_CommandList* commandList, const ICompositeView& compositeView, const DirectionalLight<APITag>& light, const SkyPass<APITag>::Parameters& params) {
		commandList->BeginMarker("Sky");

		for (Uint32 viewIndex = 0; viewIndex < compositeView.Get_NumChildViews(ViewType::PLANAR); ++viewIndex) {
			const IView* view{ compositeView.Get_ChildView(ViewType::PLANAR, viewIndex) };

			Math::AffineF3 viewToWorld{ view->Get_InverseViewMatrix() };
			viewToWorld.m_Translation = 0.f;
			Math::MatF44 clipToTranslatedWorld{ view->Get_InverseProjectionMatrix(true) * Math::AffineToHomogeneous(viewToWorld) };

			SkyConstants skyConstants{};
			skyConstants.MatClipToTranslatedWorld = clipToTranslatedWorld;
			this->FillShaderParameters(light, params, skyConstants.Params);
			commandList->WriteBuffer(this->m_SkyCB, &skyConstants, sizeof(skyConstants));

			commandList->SetGraphicsState(RHI::RHIGraphicsStateBuilder<APITag>{}
			.Set_Pipeline(this->m_PSO)
				.Set_FrameBuffer(this->m_FrameBufferFactory->Get_FrameBuffer(*view))
				.AddBindingSet(this->m_RenderBindingSet)
				.Set_ViewportState(view->Get_ViewportState())
				.Build()
				);

			commandList->Draw(RHI::RHIDrawArguments{ .VertexCount{ 4 } });
		}

		commandList->EndMarker();
	}

	template<RHI::APITagConcept APITag>
	inline void SkyPass<APITag>::FillShaderParameters(const DirectionalLight<APITag>& light, const SkyPass<APITag>::Parameters& input, ::ProceduralSkyShaderParameters& output) {
		float lightAngularSize{ Math::Radians(Math::Clamp(light.AngularSize, 0.1f, 90.f)) };
		float lightSolidAngle{ 4 * Math::PI_F * Math::Square(Math::Sin(lightAngularSize * 0.5f)) };
		float lightRadiance{ light.Irradiance / lightSolidAngle };
		if (input.MaxLightRadiance > 0.f)
			lightRadiance = Math::Min(lightRadiance, input.MaxLightRadiance);

		output.DirectionToLight = Math::VecF3{ Math::Normalize(-light.Get_Direction()) };
		output.AngularSizeOfLight = lightAngularSize;
		output.LightColor = lightRadiance * light.LightColor;
		output.GlowSize = Math::Radians(Math::Clamp(input.GlowSize, 0.f, 90.f));
		output.SkyColor = input.SkyColor * input.Brightness;
		output.GlowIntensity = Math::Clamp(input.GlowIntensity, 0.f, 1.f);
		output.HorizonColor = input.HorizonColor * input.Brightness;
		output.HorizonSize = Math::Radians(Math::Clamp(input.HorizonSize, 0.f, 90.f));
		output.GroundColor = input.GroundColor * input.Brightness;
		output.GlowSharpness = Math::Clamp(input.GlowSharpness, 1.f, 10.f);
		output.DirectionUp = Math::Normalize(input.DirectionUp);
	}

}