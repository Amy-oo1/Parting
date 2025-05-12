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


#include "Shader/blit_cb.h"

#endif // PARTING_MODULE_BUILD

namespace Parting {
	template<RHI::APITagConcept APITag>
	class CommonRenderPasses final {
		using Imp_Device = typename RHI::RHITypeTraits<APITag>::Imp_Device;
		using Imp_CommandList = typename RHI::RHITypeTraits<APITag>::Imp_CommandList;
		using Imp_BindingLayout = typename RHI::RHITypeTraits<APITag>::Imp_BindingLayout;
		using Imp_Shader = typename RHI::RHITypeTraits<APITag>::Imp_Shader;
		using Imp_Sampler = typename RHI::RHITypeTraits<APITag>::Imp_Sampler;
		using Imp_Texture = typename RHI::RHITypeTraits<APITag>::Imp_Texture;
	public:
		CommonRenderPasses(RHI::RefCountPtr<Imp_Device> device, SharedPtr<ShaderFactory<APITag>> shaderFactory) : m_Device{ device } {
			{
				Vector<ShaderMacro> VSMacros;
				VSMacros.emplace_back(ShaderMacro{ .Name{ "QUAD_Z" }, .Definition{ "0" } });
				this->m_FullscreenVS = shaderFactory->CreateShader("Parting/fullscreen_vs", "main", &VSMacros, RHI::RHIShaderType::Vertex);

				VSMacros.back().Definition = "1";
				this->m_FullscreenAtOneVS = shaderFactory->CreateShader("Parting/fullscreen_vs", "main", &VSMacros, RHI::RHIShaderType::Vertex);
			}

			this->m_RectVS = shaderFactory->CreateShader("Parting/rect_vs", "main", nullptr, RHI::RHIShaderType::Vertex);

			{
				Vector<ShaderMacro> blitMacros;
				blitMacros.emplace_back(ShaderMacro{ .Name{ "TEXTURE_ARRAY" }, .Definition{ "0" } });

				this->m_BlitPS = shaderFactory->CreateShader("Parting/blit_ps", "main", &blitMacros, RHI::RHIShaderType::Pixel);
				this->m_SharpenPS = shaderFactory->CreateShader("Parting/sharpen_ps", "main", &blitMacros, RHI::RHIShaderType::Pixel);

				blitMacros.back().Definition = "1";
				this->m_BlitArrayPS = shaderFactory->CreateShader("Parting/blit_ps", "main", &blitMacros, RHI::RHIShaderType::Pixel);
				this->m_SharpenArrayPS = shaderFactory->CreateShader("Parting/sharpen_ps", "main", &blitMacros, RHI::RHIShaderType::Pixel);

			}

			{
				RHI::RHISamplerDescBuilder samplerDescBuilder;

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
				this->m_BlackTexture = this->m_Device->CreateTexture(textureDescBuilder.Build());
				this->m_GrayTexture = this->m_Device->CreateTexture(textureDescBuilder.Build());
				this->m_WhiteTexture = this->m_Device->CreateTexture(textureDescBuilder.Build());

				this->m_BlackTexture3D = this->m_Device->CreateTexture(textureDescBuilder.Set_Dimension(RHI::RHITextureDimension::Texture3D).Build());

				textureDescBuilder.Set_ArraySize(6);
				this->m_BlackTexture2DArray = this->m_Device->CreateTexture(textureDescBuilder.Set_Dimension(RHI::RHITextureDimension::Texture2DArray).Build());
				this->m_WhiteTexture2DArray = this->m_Device->CreateTexture(textureDescBuilder.Set_Dimension(RHI::RHITextureDimension::Texture2DArray).Build());
				this->m_BlackCubeMapArray = this->m_Device->CreateTexture(textureDescBuilder.Set_Dimension(RHI::RHITextureDimension::TextureCubeArray).Build());

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

			commandList->WriteTexture(this->m_BlackTexture, 0, 0, &blackImage, 0);
			commandList->WriteTexture(this->m_GrayTexture, 0, 0, &grayImage, 0);
			commandList->WriteTexture(this->m_WhiteTexture, 0, 0, &whiteImage, 0);

			for (Uint32 arraySlice = 0; arraySlice < 6; ++arraySlice){
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
			m_Device->ExecuteCommandList(commandList);

			this->m_BlitBindingLayout = this->m_Device->CreateBindingLayout(RHI::RHIBindingLayoutDescBuilder{}
				.Set_Visibility(RHI::RHIShaderType::All)
				.AddBinding(RHI::RHIBindingLayoutItem::BuildPushConstants(0, sizeof(BlitConstants)))
				.AddBinding(RHI::RHIBindingLayoutItem::BuildTexture_SRV(0))
				.AddBinding(RHI::RHIBindingLayoutItem::BuildSampler(0))
				.Build()
			);



		}
		~CommonRenderPasses(void) = default;
	private:
		RHI::RefCountPtr<Imp_Device> m_Device;

		RHI::RefCountPtr<Imp_Shader> m_FullscreenVS;
		RHI::RefCountPtr<Imp_Shader> m_FullscreenAtOneVS;
		RHI::RefCountPtr<Imp_Shader> m_RectVS;
		RHI::RefCountPtr<Imp_Shader> m_BlitPS;
		RHI::RefCountPtr<Imp_Shader> m_BlitArrayPS;
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

		RHI::RefCountPtr<Imp_BindingLayout> m_BlitBindingLayout;

	};
}