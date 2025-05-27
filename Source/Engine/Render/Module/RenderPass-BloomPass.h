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

#include "Shader/bloom_cb.h"

#endif // PARTING_MODULE_BUILD

namespace Parting {

	template<RHI::APITagConcept APITag>
	class BloomPass final {
		using Imp_Device = RHI::RHITypeTraits<APITag>::Imp_Device;
		using Imp_CommandList = RHI::RHITypeTraits<APITag>::Imp_CommandList;
		using Imp_Texture = RHI::RHITypeTraits<APITag>::Imp_Texture;
		using Imp_Heap = RHI::RHITypeTraits<APITag>::Imp_Heap;
		using Imp_BindingLayout = RHI::RHITypeTraits<APITag>::Imp_BindingLayout;
		using Imp_BindingSet = RHI::RHITypeTraits<APITag>::Imp_BindingSet;
		using Imp_GraphicsPipeline = RHI::RHITypeTraits<APITag>::Imp_GraphicsPipeline;
		using Imp_ComputePipeline = RHI::RHITypeTraits<APITag>::Imp_ComputePipeline;
		using Imp_FrameBuffer = RHI::RHITypeTraits<APITag>::Imp_FrameBuffer;
		using Imp_Shader = RHI::RHITypeTraits<APITag>::Imp_Shader;
		using Imp_Sampler = RHI::RHITypeTraits<APITag>::Imp_Sampler;
		using Imp_Buffer = RHI::RHITypeTraits<APITag>::Imp_Buffer;

	public:
		struct PerViewData final {
			RHI::RefCountPtr<Imp_GraphicsPipeline> BloomBlurPSO;

			RHI::RefCountPtr<Imp_Texture> TextureDownscale1;
			RHI::RefCountPtr<Imp_FrameBuffer> FrameBufferDownscale1;
			RHI::RefCountPtr<Imp_Texture> TextureDownscale2;
			RHI::RefCountPtr<Imp_FrameBuffer> FrameBufferDownscale2;

			RHI::RefCountPtr<Imp_Texture> TexturePass1Blur;
			RHI::RefCountPtr<Imp_FrameBuffer> FrameBufferPass1Blur;
			RHI::RefCountPtr<Imp_Texture> TexturePass2Blur;
			RHI::RefCountPtr<Imp_FrameBuffer> FrameBufferPass2Blur;

			RHI::RefCountPtr<Imp_BindingSet> BloomBlurBindingSetPass1;
			RHI::RefCountPtr<Imp_BindingSet> BloomBlurBindingSetPass2;
			RHI::RefCountPtr<Imp_BindingSet> BloomBlurBindingSetPass3;
			RHI::RefCountPtr<Imp_BindingSet> BlitFromDownscale1BindingSet;
			RHI::RefCountPtr<Imp_BindingSet> CompositeBlitBindingSet;
		};

	public:
		BloomPass(Imp_Device* device, const SharedPtr<ShaderFactory<APITag>>& shaderFactory, SharedPtr<CommonRenderPasses<APITag>> commonPasses, SharedPtr<FrameBufferFactory<APITag>> framebufferFactory, const ICompositeView& compositeView);
		~BloomPass(void) = default;

	public:
		void Render(Imp_CommandList* commandList, const SharedPtr<FrameBufferFactory<APITag>>& framebufferFactory, const ICompositeView& compositeView, Imp_Texture* sourceDestTexture, float sigmaInPixels, float blendFactor);

	private:
		RHI::RefCountPtr<Imp_Device> m_Device;

		SharedPtr<CommonRenderPasses<APITag>> m_CommonPasses;
		SharedPtr<FrameBufferFactory<APITag>> m_FrameBufferFactory;

		Vector<PerViewData> m_PerViewData;
		RHI::RefCountPtr<Imp_Buffer> m_BloomHBlurCB;
		RHI::RefCountPtr<Imp_Buffer> m_BloomVBlurCB;
		RHI::RefCountPtr<Imp_Shader> m_BloomBlurPixelShader;
		RHI::RefCountPtr<Imp_BindingLayout> m_BloomBlurBindingLayout;
		RHI::RefCountPtr<Imp_BindingLayout> m_BloomApplyBindingLayout;

		BindingCache<APITag> m_BindingCache{ this->m_Device };

	};

	template<RHI::APITagConcept APITag>
	inline BloomPass<APITag>::BloomPass(Imp_Device* device, const SharedPtr<ShaderFactory<APITag>>& shaderFactory, SharedPtr<CommonRenderPasses<APITag>> commonPasses, SharedPtr<FrameBufferFactory<APITag>> framebufferFactory, const ICompositeView& compositeView) :
		m_Device{ device },
		m_CommonPasses{ ::MoveTemp(commonPasses) },
		m_FrameBufferFactory{ ::MoveTemp(framebufferFactory) } {

		this->m_BloomBlurPixelShader = shaderFactory->CreateShader("Parting/Passes/bloom_ps.hlsl", "main", nullptr, RHI::RHIShaderType::Pixel);

		RHI::RHIBufferDescBuilder CBbufferDescBuilder{}; CBbufferDescBuilder
			.Set_ByteSize(sizeof(BloomConstants))
			.Set_MaxVersions(c_MaxRenderPassConstantBufferVersions)
			.Set_IsConstantBuffer(true)
			.Set_IsVolatile(true);

		this->m_BloomHBlurCB = device->CreateBuffer(CBbufferDescBuilder.Set_DebugName(_W("BloomConstantsH")).Build());
		this->m_BloomVBlurCB = device->CreateBuffer(CBbufferDescBuilder.Set_DebugName(_W("BloomConstantsV")).Build());

		this->m_BloomBlurBindingLayout = device->CreateBindingLayout(RHI::RHIBindingLayoutDescBuilder{}
			.Set_Visibility(RHI::RHIShaderType::Pixel)
			.AddBinding(RHI::RHIBindingLayoutItem::VolatileConstantBuffer(0))
			.AddBinding(RHI::RHIBindingLayoutItem::Sampler(0))
			.AddBinding(RHI::RHIBindingLayoutItem::Texture_SRV(0))
			.Build()
		);

		RHI::RHITextureDescBuilder textureDescBuilder{}; textureDescBuilder
			.Set_IsRenderTarget(true)
			.Set_InitialState(RHI::RHIResourceState::ShaderResource)
			.Set_KeepInitialState(true);

		this->m_PerViewData.resize(compositeView.Get_NumChildViews(ViewType::PLANAR));
		for (Uint32 viewIndex = 0; viewIndex < compositeView.Get_NumChildViews(ViewType::PLANAR); ++viewIndex) {
			const IView* view = compositeView.Get_ChildView(ViewType::PLANAR, viewIndex);

			auto sampleFramebuffer{ this->m_FrameBufferFactory->Get_FrameBuffer(*view) };
			auto& perViewData{ this->m_PerViewData[viewIndex] };

			const auto& viewExtent{ view->Get_ViewExtent() };

			// temporary textures for downscaling
			perViewData.TextureDownscale1 = this->m_Device->CreateTexture(textureDescBuilder
				.Set_Width(static_cast<Uint32>(Math::Ceil(viewExtent.Extent.Width / 2.f))).Set_Height(static_cast<Uint32>(Math::Ceil(viewExtent.Extent.Height / 2.f)))
				.Set_Format(sampleFramebuffer->Get_Info().ColorFormats[0])
				.Set_DebugName("bloom src mip1")
				.Build()

			);
			perViewData.FrameBufferDownscale1 = this->m_Device->CreateFrameBuffer(RHI::RHIFrameBufferDescBuilder<APITag>{}.AddColorAttachment(perViewData.TextureDownscale1).Build());

			perViewData.TextureDownscale2 = this->m_Device->CreateTexture(textureDescBuilder
				.Set_Width(static_cast<Uint32>(Math::Ceil(viewExtent.Extent.Width / 4.f))).Set_Height(static_cast<Uint32>(Math::Ceil(viewExtent.Extent.Height / 4.f)))
				/*	.Set_Format(sampleFramebuffer->Get_Info().ColorFormats[0])*/
				.Set_DebugName("bloom src mip2")
				.Build()
			);
			perViewData.FrameBufferDownscale2 = this->m_Device->CreateFrameBuffer(RHI::RHIFrameBufferDescBuilder<APITag>{}.AddColorAttachment(perViewData.TextureDownscale2).Build());


			// intermediate textures for accumulating blur
			perViewData.TexturePass1Blur = this->m_Device->CreateTexture(textureDescBuilder
				//.Set_Width(static_cast<Uint32>(Math::Ceil(viewExtent.Extent.Width / 4.f))).Set_Height(static_cast<Uint32>(Math::Ceil(viewExtent.Extent.Height / 4.f)))
				/*.Set_Format(sampleFramebuffer->Get_Info().ColorFormats[0])*/
				.Set_DebugName("bloom accumulation pass1")
				.Build()

			);
			perViewData.FrameBufferPass1Blur = this->m_Device->CreateFrameBuffer(RHI::RHIFrameBufferDescBuilder<APITag>{}.AddColorAttachment(perViewData.TexturePass1Blur).Build());

			perViewData.TexturePass2Blur = this->m_Device->CreateTexture(textureDescBuilder.Set_DebugName("bloom accumulation pass2").Build());
			perViewData.FrameBufferPass2Blur = this->m_Device->CreateFrameBuffer(RHI::RHIFrameBufferDescBuilder<APITag>{}.AddColorAttachment(perViewData.TexturePass2Blur).Build());

			perViewData.BloomBlurPSO = device->CreateGraphicsPipeline(RHI::RHIGraphicsPipelineDescBuilder<APITag>{}
			.Set_PrimType(RHI::RHIPrimitiveType::TriangleStrip)
				.Set_VS(this->m_CommonPasses->m_FullscreenVS)
				.Set_PS(this->m_BloomBlurPixelShader)
				.AddBindingLayout(this->m_BloomBlurBindingLayout)
				.Set_CullMode(RHI::RHIRasterCullMode::None)
				.Set_DepthTestEnable(false)
				.Set_StencilEnable(false)
				.Build(),
				perViewData.FrameBufferPass1Blur
				);


			perViewData.BloomBlurBindingSetPass1 = this->m_Device->CreateBindingSet(RHI::RHIBindingSetDescBuilder<APITag>{}
			.AddBinding(RHI::RHIBindingSetItem<APITag>::ConstantBuffer(0, this->m_BloomHBlurCB))
				.AddBinding(RHI::RHIBindingSetItem<APITag>::Sampler(0, this->m_CommonPasses->m_LinearClampSampler))
				.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(0, perViewData.TextureDownscale2))
				.Build(),
				this->m_BloomBlurBindingLayout
				);
			perViewData.BloomBlurBindingSetPass2 = this->m_Device->CreateBindingSet(RHI::RHIBindingSetDescBuilder<APITag>{}
			.AddBinding(RHI::RHIBindingSetItem<APITag>::ConstantBuffer(0, this->m_BloomVBlurCB))
				.AddBinding(RHI::RHIBindingSetItem<APITag>::Sampler(0, this->m_CommonPasses->m_LinearClampSampler))
				.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(0, perViewData.TexturePass1Blur))
				.Build(),
				this->m_BloomBlurBindingLayout
				);
		}

	}

	template<RHI::APITagConcept APITag>
	inline void BloomPass<APITag>::Render(Imp_CommandList* commandList, const SharedPtr<FrameBufferFactory<APITag>>& framebufferFactory, const ICompositeView& compositeView, Imp_Texture* sourceDestTexture, float sigmaInPixels, float blendFactor) {
		float effectiveSigma{ Math::Clamp(sigmaInPixels * 0.25f, 1.f, 100.f) };

		commandList->BeginMarker("Bloom");

		constexpr RHI::RHIDrawArguments fullscreenquadargs{ .VertexCount{ 4 } };


		for (Uint32 viewIndex = 0; viewIndex < compositeView.Get_NumChildViews(ViewType::PLANAR); ++viewIndex) {
			const IView* view{ compositeView.Get_ChildView(ViewType::PLANAR, viewIndex) };
			auto framebuffer{ framebufferFactory->Get_FrameBuffer(*view) };
			auto& perViewData{ this->m_PerViewData[viewIndex] };

			const auto& viewportState{ view->Get_ViewportState() };
			const auto& scissorRect{ viewportState.ScissorRects[0] };
			const auto& fbinfo{ framebuffer->Get_Info() };

			// downscale
			{
				commandList->BeginMarker("Downscale");

				Math::BoxF2 uvSrcRect{
					Math::VecF2{ static_cast<float>(scissorRect.Offset.X) / static_cast<float>(fbinfo.Width),static_cast<float>(scissorRect.Offset.Y) / static_cast<float>(fbinfo.Height) },
					Math::VecF2{ static_cast<float>(scissorRect.Offset.X + scissorRect.Extent.Width) / static_cast<float>(fbinfo.Width),float(scissorRect.Offset.Y + scissorRect.Extent.Height) / static_cast<float>(fbinfo.Height) }
				};

				// half-scale down
				BLITParameters<APITag> blitParams1;
				{
					blitParams1.TargetFrameBuffer = perViewData.FrameBufferDownscale1;
					blitParams1.SourceTexture = sourceDestTexture;
					blitParams1.SourceBox = uvSrcRect;
				}
				this->m_CommonPasses->BLITTexture(commandList, blitParams1, &this->m_BindingCache);

				// half-scale again down to quarter-scale

				BLITParameters<APITag> blitParams2;
				{
					blitParams2.TargetFrameBuffer = perViewData.FrameBufferDownscale2;
					blitParams2.SourceTexture = perViewData.TextureDownscale1;
				}
				this->m_CommonPasses->BLITTexture(commandList, blitParams2, &this->m_BindingCache);

				commandList->EndMarker(); // "Downscale"
			}

			// apply blur
			{
				commandList->BeginMarker("Blur");

				BloomConstants bloomHorizonal{};
				{
					bloomHorizonal.Pixstep.X = 1.f / perViewData.TexturePass1Blur->Get_Desc().Extent.Width;
					bloomHorizonal.Pixstep.Y = 0.f;
					bloomHorizonal.ArgumentScale = -1.f / (2 * effectiveSigma * effectiveSigma);
					bloomHorizonal.NormalizationScale = 1.f / (Math::Sqrt(2 * Math::PI_F) * effectiveSigma);
					bloomHorizonal.NumSamples = Math::Round(effectiveSigma * 4.f);
				}
				BloomConstants bloomVertical = bloomHorizonal;
				bloomVertical.Pixstep.X = 0.f;
				bloomVertical.Pixstep.Y = 1.f / perViewData.TexturePass1Blur->Get_Desc().Extent.Height;
				commandList->WriteBuffer(this->m_BloomHBlurCB, &bloomHorizonal, sizeof(bloomHorizonal));
				commandList->WriteBuffer(this->m_BloomVBlurCB, &bloomVertical, sizeof(bloomVertical));

				commandList->SetGraphicsState(RHI::RHIGraphicsStateBuilder<APITag>{}
				.Set_Pipeline(perViewData.BloomBlurPSO)
					.Set_FrameBuffer(perViewData.FrameBufferPass1Blur)
					.AddViewportAndScissorRect(RHI::RHIViewport::Build(static_cast<float>(perViewData.TexturePass1Blur->Get_Desc().Extent.Width), static_cast<float>(perViewData.TexturePass1Blur->Get_Desc().Extent.Height)))
					.AddBindingSet(perViewData.BloomBlurBindingSetPass1)
					.Build()
					);
				commandList->Draw(fullscreenquadargs); // blur to m_TexturePass1Blur or m_TexturePass3Blur

				commandList->SetGraphicsState(RHI::RHIGraphicsStateBuilder<APITag>{}
				.Set_Pipeline(perViewData.BloomBlurPSO)
					.Set_FrameBuffer(perViewData.FrameBufferPass2Blur)
					.AddViewportAndScissorRect(RHI::RHIViewport::Build(static_cast<float>(perViewData.TexturePass2Blur->Get_Desc().Extent.Width), static_cast<float>(perViewData.TexturePass2Blur->Get_Desc().Extent.Height)))
					.AddBindingSet(perViewData.BloomBlurBindingSetPass2)
					.Build()

					);
				commandList->Draw(fullscreenquadargs); // blur to m_TexturePass2Blur

				commandList->EndMarker(); // "Blur"
			}

			// composite
			{
				commandList->BeginMarker("Apply");

				BLITParameters<APITag> blitParams3;
				{
					blitParams3.TargetFrameBuffer = framebuffer;
					blitParams3.TargetViewport = viewportState.Viewports[0];
					blitParams3.SourceTexture = perViewData.TexturePass2Blur;
					blitParams3.BlendState = RHI::RHIBlendState::RHIRenderTargetBuilder{}
						.Set_BlendEnable(true)
						.Set_SrcBlend(RHI::RHIBlendFactor::ConstantColor)
						.Set_DestBlend(RHI::RHIBlendFactor::InvConstantColor)
						.Set_SrcBlendAlpha(RHI::RHIBlendFactor::Zero)
						.Set_DestBlendAlpha(RHI::RHIBlendFactor::One)
						.Build();
					blitParams3.BlendConstantColor = Color{ blendFactor };
				}

				this->m_CommonPasses->BLITTexture(commandList, blitParams3, &this->m_BindingCache);

				commandList->EndMarker(); // "Apply"
			}
		}

		commandList->EndMarker(); // "Bloom"
	}
}