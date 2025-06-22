#ifdef PARTING_MODULE_BUILD
#include "Core/ModuleBuild.h"

PARTING_GLOBAL_MODULE
#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"

PARTING_MODULE(CommonRenderPasses)

PARTING_IMPORT Platform;
PARTING_IMPORT Utility;


#else
#pragma once

#include "Core/ModuleBuild.h"


#include "Core/Utility/Include/UtilityMacros.h"
#include "Core/Logger/Include/LogMacros.h"

//Global

#include "Core/Platform/Module/Platform.h"
#include "Core/Utility/Module/Utility.h"
#include "Core/Concurrent/Module/Concurrent.h"
#include "Core/Container/Module/Container.h"
#include "Core/String/Module/String.h"
#include "Core/Algorithm/Module/Algorithm.h"
#include "Core/VectorMath/Module/VectorMath.h"
#include "Core/VFS/Module/VFS.h"
#include "Core/Logger/Module/Logger.h"

#include "RHI/Module/RHI.h"
#include "D3D12RHI/Module/D3D12RHI.h"

#include "Engine/Engine/Module/ShaderFactory.h"
#include "Engine/Engine/Module/BindingCache.h"


#include "Shader/blit_cb.h"

#endif // PARTING_MODULE_BUILD

namespace Parting {


	template<RHI::APITagConcept APITag>
	class DepthPass;


	constexpr Uint32 c_MaxRenderPassConstantBufferVersions{ 16 };

	enum class BLITSampler :Uint8 {
		Point,
		Linear,
		Sharpen
	};

	template<RHI::APITagConcept APITag>
	struct BLITParameters final {
		using Imp_Texture = typename RHI::RHITypeTraits<APITag>::Imp_Texture;
		using Imp_FramBuffer = typename RHI::RHITypeTraits<APITag>::Imp_FrameBuffer;

		Imp_FramBuffer* TargetFrameBuffer{ nullptr };
		RHI::RHIViewport TargetViewport;
		Math::BoxF2 TargetBox{ 0.f, 1.f };

		Imp_Texture* SourceTexture{ nullptr };
		Uint32 SourceArraySlice{ 0 };
		Uint32 SourceMip{ 0 };
		Math::BoxF2 SourceBox{ 0.f, 1.f };
		RHI::RHIFormat SourceFormat{ RHI::RHIFormat::UNKNOWN };

		BLITSampler Sampler{ BLITSampler::Linear };
		RHI::RHIBlendState::RHIRenderTarget BlendState;
		Color BlendConstantColor;
	};



	template<RHI::APITagConcept APITag>
	class CommonRenderPasses final {
		friend class DepthPass<APITag>;

		using Imp_Device = typename RHI::RHITypeTraits<APITag>::Imp_Device;
		using Imp_CommandList = typename RHI::RHITypeTraits<APITag>::Imp_CommandList;
		using Imp_BindingLayout = typename RHI::RHITypeTraits<APITag>::Imp_BindingLayout;
		using Imp_BindingSet = typename RHI::RHITypeTraits<APITag>::Imp_BindingSet;
		using Imp_Shader = typename RHI::RHITypeTraits<APITag>::Imp_Shader;
		using Imp_Sampler = typename RHI::RHITypeTraits<APITag>::Imp_Sampler;
		using Imp_Texture = typename RHI::RHITypeTraits<APITag>::Imp_Texture;
		using Imp_FrameBuffer = typename RHI::RHITypeTraits<APITag>::Imp_FrameBuffer;
		using Imp_GraphicsPipeline = typename RHI::RHITypeTraits<APITag>::Imp_GraphicsPipeline;

	public:
		struct PSOCacheKey final {
			RHI::RHIFrameBufferInfo<APITag> FrameBufferInfo;
			Imp_Shader* Shader;
			RHI::RHIBlendState::RHIRenderTarget BlendState;

			bool operator==(const PSOCacheKey&) const noexcept = default;
			bool operator!=(const PSOCacheKey&) const noexcept = default;

			struct PSOCacheKeyHash {
				Uint64 operator ()(const PSOCacheKey& s) const {
					Uint64 hash{ 0 };
					hash = ::HashCombine(hash, typename RHI::RHIFrameBufferInfo<APITag>::RHIFrameBufferInfoHash{}(s.FrameBufferInfo));
					hash = ::HashCombine(hash, HashVoidPtr{}(s.Shader));
					hash = ::HashCombine(hash, RHI::RHIBlendState::RHIRenderTarget::RHIRenderTargetHash{}(s.BlendState));
					return hash;
				}
			};
		};

	public:
		CommonRenderPasses(Imp_Device* device, SharedPtr<ShaderFactory<APITag>> shaderFactory) :m_Device{ device } {
			{
				Vector<ShaderMacro> VSMacros{
					ShaderMacro{.Name{ String{ "QUAD_Z" } }, .Definition{ String{ "1" } } },
				};
				this->m_FullscreenVS = shaderFactory->CreateShader(String{ "Parting/fullscreen_vs" }, String{ "main" }, &VSMacros, RHI::RHIShaderType::Vertex);

				VSMacros.back().Definition = String{ "1" };
				this->m_FullscreenAtOneVS = shaderFactory->CreateShader(String{ "Parting/fullscreen_vs" }, String{ "main" }, &VSMacros, RHI::RHIShaderType::Vertex);
			}

			this->m_RectVS = shaderFactory->CreateShader(String{ "Parting/rect_vs" }, String{ "main" }, nullptr, RHI::RHIShaderType::Vertex);

			{
				Vector<ShaderMacro> blitMacros{
					ShaderMacro{.Name{ String{ "TEXTURE_ARRAY" } }, .Definition{ String{ "0" } } },
				};
				this->m_BLITPS = shaderFactory->CreateShader(String{ "Parting/blit_ps" }, String{ "main" }, &blitMacros, RHI::RHIShaderType::Pixel);
				this->m_SharpenPS = shaderFactory->CreateShader(String{ "Parting/sharpen_ps" }, String{ "main" }, &blitMacros, RHI::RHIShaderType::Pixel);

				blitMacros.back().Definition = String{ "1" };
				this->m_BLITArrayPS = shaderFactory->CreateShader(String{ "Parting/blit_ps" }, String{ "main" }, &blitMacros, RHI::RHIShaderType::Pixel);
				this->m_SharpenArrayPS = shaderFactory->CreateShader(String{ "Parting/sharpen_ps" }, String{ "main" }, &blitMacros, RHI::RHIShaderType::Pixel);
			}

			{
				RHI::RHISamplerDescBuilder samplerDescBuilder{};

				samplerDescBuilder.Set_AddressModeUVW(RHI::RHISamplerAddressMode::Clamp);
				this->m_PointClampSampler = this->m_Device->CreateSampler(samplerDescBuilder.Set_AllFilter(false).Build());
				this->m_LinearClampSampler = this->m_Device->CreateSampler(samplerDescBuilder.Set_AllFilter(true).Build());

				samplerDescBuilder.Set_AddressModeUVW(RHI::RHISamplerAddressMode::Wrap).Set_AllFilter(true);
				this->m_LinearWrapSampler = this->m_Device->CreateSampler(samplerDescBuilder.Build());
				this->m_AnisotropicWrapSampler = this->m_Device->CreateSampler(samplerDescBuilder.Set_MaxAnisotropy(16.0f).Build());
			}

			{
				RHI::RHITextureDescBuilder textureDescBuilder{};

				textureDescBuilder.Set_Format(RHI::RHIFormat::RGBA8_UNORM);
				this->m_BlackTexture = this->m_Device->CreateTexture(textureDescBuilder.Set_DebugName(String{ "BlackTexture" }).Build());
				this->m_GrayTexture = this->m_Device->CreateTexture(textureDescBuilder.Set_DebugName(String{ "GrayTexture" }).Build());
				this->m_WhiteTexture = this->m_Device->CreateTexture(textureDescBuilder.Set_DebugName(String{ "WhiteTexture" }).Build());

				this->m_BlackTexture3D = this->m_Device->CreateTexture(textureDescBuilder.Set_Dimension(RHI::RHITextureDimension::Texture3D).Set_DebugName(String{ "BlackTexture3D" }).Build());

				textureDescBuilder.Set_ArraySize(6);
				this->m_BlackTexture2DArray = this->m_Device->CreateTexture(textureDescBuilder.Set_Dimension(RHI::RHITextureDimension::Texture2DArray).Set_DebugName(String{ "BlackTexture2DArray" }).Build());
				this->m_WhiteTexture2DArray = this->m_Device->CreateTexture(textureDescBuilder.Set_Dimension(RHI::RHITextureDimension::Texture2DArray).Set_DebugName(String{ "WhiteTexture2DArray" }).Build());
				this->m_BlackCubeMapArray = this->m_Device->CreateTexture(textureDescBuilder.Set_Dimension(RHI::RHITextureDimension::TextureCubeArray).Set_DebugName(String{ "BlackCubeMapArray" }).Build());
			}

			constexpr Uint32 blackImage{ 0xff000000 };
			constexpr Uint32 grayImage{ 0xff808080 };
			constexpr Uint32 whiteImage{ 0xffffffff };

			RHI::RefCountPtr<Imp_CommandList> commandList{ this->m_Device->CreateCommandList() };

			commandList->Open();

			commandList->BeginTrackingTextureState(this->m_BlackTexture, RHI::g_AllSubResourceSet, RHI::RHIResourceState::Common);
			commandList->BeginTrackingTextureState(this->m_GrayTexture, RHI::g_AllSubResourceSet, RHI::RHIResourceState::Common);
			commandList->BeginTrackingTextureState(this->m_WhiteTexture, RHI::g_AllSubResourceSet, RHI::RHIResourceState::Common);
			commandList->BeginTrackingTextureState(this->m_BlackCubeMapArray, RHI::g_AllSubResourceSet, RHI::RHIResourceState::Common);
			commandList->BeginTrackingTextureState(this->m_BlackTexture2DArray, RHI::g_AllSubResourceSet, RHI::RHIResourceState::Common);
			commandList->BeginTrackingTextureState(this->m_WhiteTexture2DArray, RHI::g_AllSubResourceSet, RHI::RHIResourceState::Common);
			commandList->BeginTrackingTextureState(this->m_BlackTexture3D, RHI::g_AllSubResourceSet, RHI::RHIResourceState::Common);

			commandList->WriteTexture(this->m_BlackTexture, 0, 0, &blackImage, 0);//TODO :Check
			commandList->WriteTexture(this->m_GrayTexture, 0, 0, &grayImage, 0);//TODO :Check
			commandList->WriteTexture(this->m_WhiteTexture, 0, 0, &whiteImage, 0);//TODO :Check

			for (Uint32 arraySlice = 0; arraySlice < 6; ++arraySlice) {
				commandList->WriteTexture(this->m_BlackTexture2DArray, arraySlice, 0, &blackImage, 0);
				commandList->WriteTexture(this->m_WhiteTexture2DArray, arraySlice, 0, &whiteImage, 0);
				commandList->WriteTexture(this->m_BlackCubeMapArray, arraySlice, 0, &blackImage, 0);
				commandList->WriteTexture(this->m_BlackTexture3D, 0, 0, &blackImage, 0);
			}

			commandList->SetPermanentTextureState(this->m_BlackTexture, RHI::RHIResourceState::ShaderResource);
			commandList->SetPermanentTextureState(this->m_GrayTexture, RHI::RHIResourceState::ShaderResource);
			commandList->SetPermanentTextureState(this->m_WhiteTexture, RHI::RHIResourceState::ShaderResource);
			commandList->SetPermanentTextureState(this->m_BlackCubeMapArray, RHI::RHIResourceState::ShaderResource);
			commandList->SetPermanentTextureState(this->m_BlackTexture2DArray, RHI::RHIResourceState::ShaderResource);
			commandList->SetPermanentTextureState(this->m_WhiteTexture2DArray, RHI::RHIResourceState::ShaderResource);
			commandList->SetPermanentTextureState(this->m_BlackTexture3D, RHI::RHIResourceState::ShaderResource);
			commandList->CommitBarriers();

			commandList->Close();
			this->m_Device->ExecuteCommandList(commandList);

			this->m_BLITBindingLayout = this->m_Device->CreateBindingLayout(RHI::RHIBindingLayoutDescBuilder{}
				.Set_Visibility(RHI::RHIShaderType::All)
				.AddBinding(RHI::RHIBindingLayoutItem::PushConstants(0, sizeof(Shader::BLITConstants)))
				.AddBinding(RHI::RHIBindingLayoutItem::Texture_SRV(0))
				.AddBinding(RHI::RHIBindingLayoutItem::Sampler(0))
				.Build()
			);
		}
		~CommonRenderPasses(void) = default;


	public:
		void BLITTexture(Imp_CommandList* commandlist, const BLITParameters<APITag>& params, BindingCache<APITag>* bindingCache = nullptr) {
			ASSERT(nullptr != commandlist);
			ASSERT(nullptr != params.TargetFrameBuffer);
			ASSERT(nullptr != params.SourceTexture);

			const auto& framebufferDesc{ params.TargetFrameBuffer->Get_Desc() };
			ASSERT(1 == framebufferDesc.ColorAttachmentCount);
			ASSERT(framebufferDesc.ColorAttachments[0].Is_Valid());

			const auto& fbinfo{ params.TargetFrameBuffer->Get_Info() };
			const auto& sourceDesc{ params.SourceTexture->Get_Desc() };

			ASSERT(CommonRenderPasses<APITag>::Is_SupportedBLITDimension(sourceDesc.Dimension));
			bool isTextureArray{ CommonRenderPasses<APITag>::Is_TextureArray(sourceDesc.Dimension) };


			RHI::RHIViewport targetViewport{ params.TargetViewport };
			// If no viewport is specified, create one based on the framebuffer dimensions.
			// Note that the FB dimensions may not be the same as target texture dimensions, in case a non-zero mip level is used.
			if (targetViewport.Width() == 0 && targetViewport.Height() == 0)
				targetViewport = RHI::RHIViewport::Build(static_cast<float>(fbinfo.Width), static_cast<float>(fbinfo.Height));

			Imp_Shader* shader{ nullptr };
			switch (params.Sampler) {
			case BLITSampler::Point:
			case BLITSampler::Linear: shader = isTextureArray ? this->m_BLITArrayPS : this->m_BLITPS; break;
			case BLITSampler::Sharpen: shader = isTextureArray ? this->m_SharpenArrayPS : this->m_SharpenPS; break;
			default: ASSERT(false);
			}

			auto& pso{ this->m_BLITPSOCache[CommonRenderPasses<APITag>::PSOCacheKey{.FrameBufferInfo{ fbinfo }, .Shader{ shader }, .BlendState{ params.BlendState } }] };
			if (nullptr == pso)
				pso = this->m_Device->CreateGraphicsPipeline(
					RHI::RHIGraphicsPipelineDescBuilder<APITag>{}
			.Set_PrimType(RHI::RHIPrimitiveType::TriangleStrip)
				.Set_VS(this->m_RectVS)
				.Set_PS(shader)
				.Set_CullMode(RHI::RHIRasterCullMode::None)
				.Set_DepthTestEnable(false)
				/*.Set_StencilEnable(false)*/
				.Set_BlendState(0, params.BlendState)
				.AddBindingLayout(this->m_BLITBindingLayout)
				.Build(),
				params.TargetFrameBuffer
				);

			RHI::RHIBindingSetDescBuilder<APITag> bindingSetDescBuilder;
			{
				auto sourceDimension{ sourceDesc.Dimension };
				if (sourceDimension == RHI::RHITextureDimension::TextureCube || sourceDimension == RHI::RHITextureDimension::TextureCubeArray)
					sourceDimension = RHI::RHITextureDimension::Texture2DArray;

				const auto sourceSubresources{ RHI::RHITextureSubresourceSet(params.SourceMip, 1, params.SourceArraySlice, 1) };

				//TODO : TODO ?
				bindingSetDescBuilder
					.AddBinding(RHI::RHIBindingSetItem<APITag>::PushConstants(0, sizeof(Shader::BLITConstants)))
					.AddBinding(RHI::RHIBindingSetItem<APITag>::Texture_SRV(0, params.SourceTexture, params.SourceFormat, sourceSubresources, sourceDimension))
					.AddBinding(RHI::RHIBindingSetItem<APITag>::Sampler(0, params.Sampler == BLITSampler::Point ? this->m_PointClampSampler : this->m_LinearClampSampler));
			}

			// If a binding cache is provided, get the binding set from the cache.
			// Otherwise, create one and then release it.
			RHI::RefCountPtr<Imp_BindingSet>sourceBindingSet;
			if (nullptr != bindingCache)
				sourceBindingSet = bindingCache->GetOrCreateBindingSet(bindingSetDescBuilder.Build(), this->m_BLITBindingLayout);
			else
				sourceBindingSet = this->m_Device->CreateBindingSet(bindingSetDescBuilder.Build(), this->m_BLITBindingLayout);

			commandlist->SetGraphicsState(RHI::RHIGraphicsStateBuilder<APITag>{}
			.Set_Pipeline(pso)
				.Set_FrameBuffer(params.TargetFrameBuffer)
				.AddViewportAndScissorRect(targetViewport)
				.Set_BlendConstantColor(params.BlendConstantColor)
				.AddBindingSet(sourceBindingSet)
				.Build()
				);

			Shader::BLITConstants blitConstants{
				.SourceOrigin { params.SourceBox.m_Mins },
				.SourceSize { params.SourceBox.Diagonal() },

				.TargetOrigin { params.TargetBox.m_Mins },
				.TargetSize { params.TargetBox.Diagonal() },
			};

			commandlist->SetPushConstants(&blitConstants, sizeof(decltype(blitConstants)));
			commandlist->Draw(RHI::RHIDrawArguments{ .VertexCount{ 4 } });
		}

		void BLITTexture(Imp_CommandList* commandList, Imp_FrameBuffer* targetFramebuffer, Imp_Texture* sourceTexture, BindingCache<APITag>* bindingCache = nullptr) {
			ASSERT(nullptr != commandList);
			ASSERT(nullptr != targetFramebuffer);
			ASSERT(nullptr != sourceTexture);

			this->BLITTexture(commandList, BLITParameters<APITag>{.TargetFrameBuffer{ targetFramebuffer }, .SourceTexture{ sourceTexture } }, bindingCache);
		}

	public:
		static bool Is_SupportedBLITDimension(RHI::RHITextureDimension dimension) {
			using enum RHI::RHITextureDimension;
			return
				dimension == Texture2D ||
				dimension == Texture2DArray ||
				dimension == TextureCube ||
				dimension == TextureCubeArray;
		}

		static bool Is_TextureArray(RHI::RHITextureDimension dimension) {
			using enum RHI::RHITextureDimension;
			return
				dimension == Texture2DArray ||
				dimension == TextureCube ||
				dimension == TextureCubeArray;
		}


	public://TODO :Pass Friend....
		RHI::RefCountPtr<Imp_Device> m_Device;

		RHI::RefCountPtr<Imp_Shader> m_FullscreenVS;
		RHI::RefCountPtr<Imp_Shader> m_FullscreenAtOneVS;
		RHI::RefCountPtr<Imp_Shader> m_RectVS;
		RHI::RefCountPtr<Imp_Shader> m_BLITPS;
		RHI::RefCountPtr<Imp_Shader> m_BLITArrayPS;
		RHI::RefCountPtr<Imp_Shader> m_SharpenPS;
		RHI::RefCountPtr<Imp_Shader> m_SharpenArrayPS;

		RHI::RefCountPtr<Imp_Sampler> m_PointClampSampler;
		RHI::RefCountPtr<Imp_Sampler> m_LinearClampSampler;
		RHI::RefCountPtr<Imp_Sampler> m_LinearWrapSampler;
		RHI::RefCountPtr<Imp_Sampler> m_AnisotropicWrapSampler;

		RHI::RefCountPtr<Imp_Texture> m_BlackTexture;
		RHI::RefCountPtr<Imp_Texture> m_GrayTexture;
		RHI::RefCountPtr<Imp_Texture> m_WhiteTexture;
		RHI::RefCountPtr<Imp_Texture> m_BlackTexture2DArray;
		RHI::RefCountPtr<Imp_Texture> m_WhiteTexture2DArray;
		RHI::RefCountPtr<Imp_Texture> m_BlackCubeMapArray;
		RHI::RefCountPtr<Imp_Texture> m_BlackTexture3D;

		RHI::RefCountPtr<Imp_BindingLayout> m_BLITBindingLayout;

		UnorderedMap<PSOCacheKey, RHI::RefCountPtr<Imp_GraphicsPipeline>, typename PSOCacheKey::PSOCacheKeyHash> m_BLITPSOCache;


	};
}